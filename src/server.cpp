#include "../include/server.h"
#include <iostream>
#include <sstream>
#include <netinet/ether.h>
#include <thread>
#include <vector>
#include <algorithm>
#include <iomanip>

Server_Connection::Server_Connection(bool is_manager)
{
    //this->count = 0;
    char hostname_c[BUFFER_SIZE];
    gethostname(hostname_c,BUFFER_SIZE);
    this->hostname = hostname_c;
    this->server = gethostbyname(hostname_c); // nome do server/manager
    this->serv_addr.sin_family = AF_INET;     
    this->serv_addr.sin_port = htons(PORT);    
    this->serv_addr.sin_addr = *((struct in_addr *)this->server->h_addr);
    this->serv_addr.sin_addr.s_addr = INADDR_ANY;
    this->seqn = 0;
    this->participants.clear();
    this->is_manager = is_manager;
    this->active = true;
    this->status_update = NO_CHANGE;
};

void Server_Connection::start()
{
    //Thread class expects a pointer, and this "casting" allows us to use threads with class methods
    thread discovery_thread([this] { this->discovery(); } );
    thread monitoring_thread( [this] { this->monitoring(); } );

    this->interface();
    exit(0);
    discovery_thread.join();
    monitoring_thread.join();
};

void Server_Connection::discovery()
{
    // while(this->active)
    // {
        cout << "discovery" << endl; 
        if(this->is_manager)
            start_client(SLEEP_SERVICE_DISCOVERY,FIND_PARTICIPANTS);
        else
            start_server(SLEEP_SERVICE_DISCOVERY,FIND_MANAGER);
    // };
};

void Server_Connection::monitoring()
{
    // while(this->active)
    // {
        cout << "monitoring" << endl; 
        if(this->is_manager)
            start_client(SLEEP_STATUS_REQUEST,SEND_STATUS_UPDATE);

        else
            start_server(SLEEP_STATUS_REQUEST,SENDING_STATUS_UPDATE);
            // if(this->status_update != NO_CHANGE)
            // {
                // this->participants_mutex.lock();
                // this->participants
                // this->participants_mutex.unlock();
            // }
    // };
};

void Server_Connection::interface()
{
    while(this->active)
    {
        terminal_cmds();
    };
};

void Server_Connection::terminal_cmds()
{
    bool invalid_input = false;
    string terminal_input;
    
    //When "CTR.+ D" or "CTRL + S" get pressed this while will break
    while (getline(cin,terminal_input))
    {
        vector<string> input_vec;

        //getline(cin,terminal_input);
        transform(terminal_input.begin(), terminal_input.end(), terminal_input.begin(), ::toupper);

        int space_position = terminal_input.find(" ");
        string cmd = terminal_input.substr(0, space_position);
        string cmd_argument = terminal_input.substr(space_position+1);

        if (this->is_manager)
        {
            if (cmd == WAKEUP && cmd_argument != cmd) //if no hostname is informed, no argument will have been found in the terminal command and substrings will be equal
            {
                string hostname = cmd_argument;

                //wakeup(hostname,&this->participants); //Not a real method, just writing out program flow
                //show_participants(); //Show updated participants list
                invalid_input = true;
            }
            else
                cout << "The only valid command for manager station is 'WAKEUP <hostname>'" << endl;
        }
        else
        {
            if (cmd == EXIT) 
                this->active = false;
            else
                cout << "The only valid command for participant station is 'EXIT'" << endl;   
        };
    };
    this->active = false;
};

void Server_Connection::show_participants()
{
    cout << setw(15) << "\nPARTICIPANTS" << endl;
    cout << left << setw(25)
         << "Hostname"
         << left << setw(25)
         << "IP"
         << left << setw(25)
         << "MAC"
         << left << setw(10)
         << "Status"
         << left << setw(10)
         << "Manager"
         << endl;

    map<string, Work_Station>::iterator it;

    this->participants_mutex.lock();
    for (it = participants.begin(); it != participants.end(); it++)
    {
        Work_Station current = it->second;
        string status_string;

        if (current.status = AWAKEN)
            status_string = "Awaken";
        else if(current.status = ASLEEP)
            status_string = "Asleep";
        else
            status_string = "Unknown";

        cout << left << setw(25)
             << current.hostname
             << left << setw(25)
             << current.ip_address
             << left << setw(25)
             << current.mac_address
             << left << setw(10)
             << status_string
             << left << setw(10)
             << (current.is_manager ? "Yes" : "No")
             << endl;
    }
    this->participants_mutex.unlock();
};

//Manager
void Server_Connection::start_client(uint16_t packet_type,string _payload)
{
    // while(!hasTimeoutPassed(timeoutInMicroseconds) //commenting for now because it seems to need adjustments 
    // this->work_station.is_manager = false;
    // add_participant(this->work_station);
    while(this->active)
    {
        //this->start = clock();

        if (this->server == NULL) 
        {
            fprintf(stderr,"ERROR, no such host\n");
            exit(0);
        };	

        int bcast_sock;
        if ((bcast_sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
            cout << "ERROR opening socket";

        int broadcast_enable = 1; // cliente manda em broadcast pra ver quem é o lider
        if(setsockopt(bcast_sock,SOL_SOCKET,SO_BROADCAST,&broadcast_enable,sizeof(broadcast_enable)) < 0)
            cout << "Error setting broadcast"; 

        bzero(&(this->serv_addr.sin_zero), 8);
        int sent = 0, received = 0;

        Packet packet_sent = create_packet(packet_type, _payload);
        string packet_sent_str = packet_to_string(packet_sent);

        sent = sendto(bcast_sock, packet_sent_str.c_str(), packet_sent_str.length(), 0, (const struct sockaddr *) &serv_addr, sizeof(struct sockaddr_in));
        if (sent < 0) 
            cout << "ERROR sendto" << endl;

        unsigned int length = sizeof(struct sockaddr_in);
        int timeoutInMicroseconds = 1;

        char buff[BUFFER_SIZE];
        if(&serv_addr == NULL)
            cout << "serv_addr NULL" << endl;

        if (received = recvfrom(bcast_sock, buff, BUFFER_SIZE, 0, (struct sockaddr *) &from, &length) > 0) 
        {
            // recebeu quebra o loop e vai pro manager
            Packet packet_received;
            cout << "Received the answer: " << buff << endl;
            //packet_received = string_to_packet(buff);
            process_message(buff);
            //break;
            //manda pro manager()??????

        }
        else if (received < 0)
            cout << "ERROR recvfrom" << endl; 
        close(bcast_sock);
    
    };
};

//Participant
void Server_Connection::start_server(uint16_t packet_type,string _payload)
{
    while (this->active) 
    {

    int peer2peer, sent,received;
    socklen_t clilen;
    struct sockaddr_in cli_addr;
    char buff[BUFFER_SIZE];
        
    if ((peer2peer = socket(AF_INET, SOCK_DGRAM, 0)) == -1) 
        cout << "ERROR opening socket" << endl;

    bzero(&(serv_addr.sin_zero), 8);    
    
    if (bind(peer2peer, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr)) < 0) 
        cout << "ERROR on binding" << endl;
    
    // cout << "chegou aqui. peer2peer: " << peer2peer << endl;
    // clilen = sizeof(struct sockaddr_in);
    
    // this->work_station.is_manager = true;
    // add_participant(this->work_station);
        bzero(buff, BUFFER_SIZE);
        /* receive from socket */
        received = recvfrom(peer2peer, buff, BUFFER_SIZE, 0, (struct sockaddr *) &cli_addr, &clilen);
        if (received < 0) 
            cout << "ERROR on recvfrom"  << endl;
        
        cout << "String packet received: " <<  buff << endl;
        
        Packet received_packet = string_to_packet(buff); 

        //Packet packet_sent = create_packet(SLEEP_SERVICE_DISCOVERY, MANAGER_FOUND, true);
        Packet packet_sent = create_packet(packet_type, _payload);
        string packet_sent_str = packet_to_string(packet_sent); //String is easier to parse but must be converted to char when using in sendto and recvfrom
        
        /* send to socket */
        sent = sendto(peer2peer, packet_sent_str.c_str(), packet_sent_str.length(), 0,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));
        if (sent < 0) 
            cout << "ERROR on sendto" << endl;
        close(peer2peer);
    }
    
    // close(peer2peer);
};

char *Server_Connection::get_mac()
{
    struct ifreq ifr;
    char *mac_address_hex;
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    // Get Mac Adress
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, "docker0", IFNAMSIZ - 1);
    ioctl(fd, SIOCGIFHWADDR, &ifr);
    close(fd);
    // mac_address_hex = (char *)ifr.ifr_hwaddr.sa_data;
    // printf("MAC address: %02x:%02x:%02x:%02x:%02x:%02x\n", (unsigned char)ifr.ifr_hwaddr.sa_data[0], (unsigned char)ifr.ifr_hwaddr.sa_data[1],
    //       (unsigned char)ifr.ifr_hwaddr.sa_data[2], (unsigned char)ifr.ifr_hwaddr.sa_data[3], (unsigned char)ifr.ifr_hwaddr.sa_data[4],
    //       (unsigned char)ifr.ifr_hwaddr.sa_data[5]);

    //Formats mac address and converts from hexadecimal to string
    char* mac_address = ether_ntoa((struct ether_addr*)ifr.ifr_hwaddr.sa_data);
    
    return mac_address;
}

char *Server_Connection::get_ip()
{
    struct ifreq ifr;
    char *ip_address;
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    // Get IP address
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, "docker0", IFNAMSIZ - 1);
    ioctl(fd, SIOCGIFADDR, &ifr);
    close(fd);
    ip_address = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
    //printf("IP address: %s\n", ip_address);

    return (ip_address);
};

// bool Server_Connection::hasTimeoutPassed( int timeoutInMicroseconds){
//     clock_t current = clock();
//     clock_t elapsed = (current - this->start) / (CLOCKS_PER_SEC / 1000000);  // Convert ticks to microseconds
//     return (elapsed >= timeoutInMicroseconds);
// };

//Process received message and build response message
Work_Station Server_Connection::process_message(char packet_string[BUFFER_SIZE])
{
    Packet received_packet = string_to_packet(packet_string); 
    bool found_station_is_manager;
    Work_Station participant;

    if (received_packet.type = SLEEP_SERVICE_DISCOVERY)
    {
        if (received_packet._payload == MANAGER_FOUND)
            found_station_is_manager = true;
        else if (received_packet._payload == PARTICIPANT_FOUND)
            found_station_is_manager = false;
        
        Work_Station participant;
        participant = create_participant(received_packet.hostname, received_packet.mac_address, received_packet.ip_address, UNKNOWN, found_station_is_manager);
        add_participant(participant);
        show_participants();
    }
    else if (received_packet.type = SLEEP_STATUS_REQUEST)
    {
        if (received_packet._payload == SENDING_STATUS_UPDATE)
            // if (received_packet.status != UNKNOWN)
            // {
            //     participant.status = received_packet.status;
            //     participant.hostname = received_packet.hostname;
            // }
            this->status_update = received_packet.status;
    }
    else if (received_packet.type = SLEEP_SERVICE_EXIT)
    {
        //TODO
    };

    // Packet packet_sent = create_packet(SLEEP_SERVICE_DISCOVERY, MANAGER_FOUND);

    return participant;
};

Work_Station Server_Connection::create_participant(string hostname, string mac_address, string ip_address, int status, bool is_manager)
{
    Work_Station participant;
    participant.hostname = hostname;
    participant.mac_address = mac_address;
    participant.ip_address = ip_address;
    participant.status = ASLEEP;
    participant.is_manager = true;

    return participant;
};

//Add new participant (locking operation)
void Server_Connection::add_participant(Work_Station participant)
{
    this->participants_mutex.lock();
    
    this->participants[participant.hostname] = participant;
    
    this->participants_mutex.unlock();
};
// ----------------- Move this to a separate class (?) -----------------

Packet Server_Connection::create_packet(uint16_t type, string _payload)
{
    Packet packet;

    packet.type = type;
    this->seqn++;
    packet.seqn = this->seqn;
    packet.length = _payload.length();
    packet.timestamp = (uint16_t) 0; //for now
    packet._payload = _payload;
    packet.mac_address = get_mac();
    packet.ip_address = get_ip();
    packet.hostname = this->work_station.hostname;
    packet.is_manager = this->is_manager;
    packet.status = UNKNOWN;

    return packet;
};

//Turn a Packet structure into a string using format "type|seqn|length|timestamp|_payload|ip_address|hostname|is_manager"
string Server_Connection::packet_to_string(Packet packet)
{
    string packet_str;

    packet_str += to_string(packet.type);
    packet_str += "|" + to_string(packet.seqn);
    packet_str += "|" + to_string(packet.length);
    packet_str += "|" + to_string(packet.timestamp);
    packet_str += "|" + packet._payload;
    packet_str += "|" + packet.ip_address;
    packet_str += "|" + packet.mac_address;
    packet_str += "|" + packet.hostname;
    packet_str += "|" + to_string(packet.is_manager);

    return packet_str;
};

//Turn a string in format "type|seqn|length|timestamp|_payload|ip_address|hostname|is_manager" into a Packet structure
Packet Server_Connection::string_to_packet(string packet_str)
{
    Packet packet;

    char *token = strtok((char *)packet_str.c_str(), "|");
    packet.type = (uint16_t) atoi(token);
    token = strtok(NULL, "|");
    packet.seqn = atoi(token);
    token = strtok(NULL, "|");
    packet.length = (uint16_t) atoi(token);
    token = strtok(NULL, "|");
    packet.timestamp = (uint16_t) atoi(token);
    token = strtok(NULL, "|");
    packet._payload = token;
    token = strtok(NULL, "|");
    packet.ip_address = token;
    token = strtok(NULL, "|");
    packet.mac_address = token;
    token = strtok(NULL, "|");
    packet.hostname = token;
    token = strtok(NULL, "|");
    packet.is_manager = token;
    
    return packet;
};
// -----------------------------------------------------------------

#include "../include/server.h"
#include <iostream>

Server_Connection::Server_Connection()
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

    for (size_t i = 0; i < this->work_station.ip_address.size(); i++) {
        this->work_station.ip_address[i] = server->h_addr[i];
    }

    this->work_station.status = WorkStation::AWAKEN;
};

void Server_Connection::start_client()
{
    // while(!hasTimeoutPassed(timeoutInMicroseconds) //commenting for now because it seems to need adjustments 
    while(1)
    {
        this->start = clock();

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

        Packet packet_sent = create_packet(SLEEP_SERVICE_DISCOVERY, FIND_MANAGER, false);
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
            packet_received = string_to_packet(buff);
            //break;
            //manda pro manager()??????

        }
        else if (received < 0)
            cout << "ERROR recvfrom" << endl; 
        close(bcast_sock);
    
    };
};

void Server_Connection::start_server()
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
    
    clilen = sizeof(struct sockaddr_in);
    
    while (1) 
    {
        bzero(buff, BUFFER_SIZE);
        /* receive from socket */
        received = recvfrom(peer2peer, buff, BUFFER_SIZE, 0, (struct sockaddr *) &cli_addr, &clilen);
        if (received < 0) 
            cout << "ERROR on recvfrom"  << endl;
        
        cout << "String packet received: " <<  buff << endl;
        
        Packet received_packet = string_to_packet(buff); 

        Packet packet_sent = create_packet(SLEEP_SERVICE_DISCOVERY, MANAGER_FOUND, true);
        string packet_sent_str = packet_to_string(packet_sent); //String is easier to parse but must be converted to char when using in sendto and recvfrom
        
        /* send to socket */
        sent = sendto(peer2peer, packet_sent_str.c_str(), packet_sent_str.length(), 0,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));
        if (sent < 0) 
            cout << "ERROR on sendto" << endl;
    }
    
    close(peer2peer);
};

char *Server_Connection::get_mac()
{
    struct ifreq ifr;
    char *mac_address_hex;
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    // Get Mac Adress
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, "enp1s0", IFNAMSIZ - 1);
    ioctl(fd, SIOCGIFHWADDR, &ifr);
    close(fd);
    mac_address_hex = (char *)ifr.ifr_hwaddr.sa_data;
    for (size_t i = 0; i < this->work_station.mac_address.size(); i++) {
        this->work_station.mac_address[i] = mac_address_hex[i];
    }

    cout << "MAC address: " << this->work_station.mac_address << endl;

    return (mac_address_hex);
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
    cout << "IP address: " << render_ip_address(this->work_station.ip_address) << endl;

    return (ip_address);
}
bool Server_Connection::hasTimeoutPassed( int timeoutInMicroseconds){
    clock_t current = clock();
    clock_t elapsed = (current - this->start) / (CLOCKS_PER_SEC / 1000000);  // Convert ticks to microseconds
    return (elapsed >= timeoutInMicroseconds);
};

// ----------------- Move this to a separate class (?) -----------------
Packet Server_Connection::create_packet(uint16_t type, string _payload, bool is_manager)
{
    Packet packet;

    packet.type = type;
    this->seqn++;
    packet.seqn = this->seqn;
    packet.length = _payload.length();
    packet.timestamp = (uint16_t) 0; //for now
    packet._payload = _payload;
    packet.mac_adress = get_mac(); //for now
    packet.ip_address = get_ip();
    packet.hostname = this->work_station.hostname;
    packet.is_manager = is_manager;

    return packet;
};

string Server_Connection::packet_to_string(Packet packet)
{
    string packet_str;

    packet_str += to_string(packet.type);
    packet_str += "|" + to_string(packet.seqn);
    packet_str += "|" + to_string(packet.length);
    packet_str += "|" + to_string(packet.timestamp);
    packet_str += "|" + packet._payload;
    packet_str += "|" + packet.ip_address;
    packet_str += "|" + packet.mac_adress;
    packet_str += "|" + packet.hostname;
    packet_str += "|" + to_string(packet.is_manager);

    return packet_str;
};

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
    packet.mac_adress = token;
    token = strtok(NULL, "|");
    packet.hostname = token;
    token = strtok(NULL, "|");
    packet.is_manager = token;
    
    return packet;
};
// -----------------------------------------------------------------

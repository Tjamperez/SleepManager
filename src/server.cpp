#include "../include/server.h"
#include <iostream>

Server_Connection::Server_Connection(char *hostname)
{
    this->count = 0;
    this->hostname = hostname;
    this->server = gethostbyname(this->hostname); // nome do server/manager
    this->serv_addr.sin_family = AF_INET;     
    this->serv_addr.sin_port = htons(PORT);    
    this->serv_addr.sin_addr = *((struct in_addr *)this->server->h_addr);
    this->serv_addr.sin_addr.s_addr = INADDR_ANY;
    this->seqn = 0;
};

int Server_Connection::start_client()
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

        Packet packet_sent = create_packet(SLEEP_SERVICE_DISCOVERY, FIND_MANAGER);
        string packet_sent_str = packet_to_string(packet_sent);

        sent = sendto(bcast_sock, packet_sent_str.c_str(), packet_sent_str.length(), 0, (const struct sockaddr *) &serv_addr, sizeof(struct sockaddr_in));
        if (sent < 0) 
            cout << "ERROR sendto" << endl;

        unsigned int length = sizeof(struct sockaddr_in);
        int timeoutInMicroseconds = 1;

        char buff[10000];
        if(&serv_addr == NULL)
            cout << "serv_addr NULL" << endl;

        if (received = recvfrom(bcast_sock, buff, 10000, 0, (struct sockaddr *) &from, &length) > 0) 
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
    return 0;
};

int Server_Connection::start_server()
{
    int peer2peer, sent,received;
    socklen_t clilen;
    struct sockaddr_in cli_addr;
    char buff[10000];
        
    if ((peer2peer = socket(AF_INET, SOCK_DGRAM, 0)) == -1) 
        cout << "ERROR opening socket" << endl;

    bzero(&(serv_addr.sin_zero), 8);    
    
    if (bind(peer2peer, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr)) < 0) 
        cout << "ERROR on binding" << endl;
    
    clilen = sizeof(struct sockaddr_in);
    
    while (1) 
    {
        bzero(buff, 10000);
        /* receive from socket */
        received = recvfrom(peer2peer, buff, 10000, 0, (struct sockaddr *) &cli_addr, &clilen);
        if (received < 0) 
            cout << "ERROR on recvfrom"  << endl;
        
        cout << "String packet received: " <<  buff << endl;
        
        Packet packet_received = string_to_packet(buff); 

        Packet packet_sent = create_packet(SLEEP_SERVICE_DISCOVERY, MANAGER_FOUND);
        string packet_sent_str = packet_to_string(packet_sent); //String is easier to parse but must be converted to char when using in sendto and recvfrom
        
        /* send to socket */
        sent = sendto(peer2peer, packet_sent_str.c_str(), packet_sent_str.length(), 0,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));
        if (sent < 0) 
            cout << "ERROR on sendto" << endl;
    }
    
    close(peer2peer);
    return 0;
};

bool Server_Connection::hasTimeoutPassed( int timeoutInMicroseconds){
    clock_t current = clock();
    clock_t elapsed = (current - this->start) / (CLOCKS_PER_SEC / 1000000);  // Convert ticks to microseconds
    return (elapsed >= timeoutInMicroseconds);
};

// ----------------- Move this to a separate class -----------------
Packet Server_Connection::create_packet(uint16_t type, string _payload)
{
    Packet packet;

    packet.type = type;
    this->seqn++;
    packet.seqn = this->seqn;
    packet.length = _payload.length();
    packet.timestamp = (uint16_t) 0; //for now
    packet._payload = _payload;

    return packet;
};

string Server_Connection::packet_to_string(Packet packet)
{
    string packet_str;

    packet_str += to_string(packet.type);
    packet_str += "|" + to_string(packet.seqn);
    packet_str += "|" + to_string(packet.length);
    packet_str += "|" + packet.timestamp;
    packet_str += "|" + to_string(packet.length);
    packet_str += "|" + packet._payload;

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
    
    return packet;
};
// -----------------------------------------------------------------
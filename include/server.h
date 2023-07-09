#ifndef SERVER_H_
#define SERVER_H_

#include "../include/discovery.h"
#include "../include/helper.h"
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

#define PORT 4000
#define BUFFER_SIZE 10000
#define MAX_TRIES 3
#define SLEEP_SERVICE_DISCOVERY 1 
#define SLEEP_SERVICE_BROADCAST 2
#define SLEEP_SERVICE_EXIT 3
#define SLEEP_STATUS_REQUEST 4
#define FIND_MANAGER "FIND_MANAGER"
#define FIND_PARTICIPANTS "FIND_PARTICIPANTS"
#define MANAGER_FOUND "MANAGER_FOUND"
#define MANAGER "MANAGER"

class Server_Connection
{
    public:
        Server_Connection();
        void start_client();
        void start_server();

    protected:
        clock_t start;//começo do programa
        struct sockaddr_in serv_addr, from;        
        struct hostent *server;
        uint16_t seqn; //Sequence number used in packets
        string hostname;
        Work_Station work_station;

        bool hasTimeoutPassed( int timeoutInMicroseconds);
        Packet create_packet(uint16_t type, string _payload, bool is_manager);
        string packet_to_string(Packet packet);
        Packet string_to_packet(string packet_str);
};

#endif
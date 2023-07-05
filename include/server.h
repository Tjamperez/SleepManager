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
#define MAX_TRIES 3

class Server_Connection
{
    public:
        clock_t start;//começo do programa
        int count;
        struct sockaddr_in serv_addr, from;        
        struct hostent *server;
        string *ip_address, *mac_address;
        //Packet packet; //There will be a different packet for each message shared between processes
        char * hostname;
    
        Server_Connection(char *hostname);
        int start_client();
        int start_server();

    protected:
        int hasTimeoutPassed( int timeoutInMicroseconds);
};

#endif
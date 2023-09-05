#ifndef DISCOVERYSUBSURVICE_H
#define DISCOVERYSUBSURVICE_H

#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <netinet/if_ether.h>
#include "ManagementSubservice.h"
#include "WebServices.h"

class DiscoverySubservice
{
    public:
        DiscoverySubservice();
        virtual ~DiscoverySubservice();

        int run();
        int InitializeServer();
        int InitializeClient();

        static void shutDown();



    protected:
        static int runDiscovery;
        int listenForBroadcasts(int &sockfd, struct sockaddr_in client_addr,  socklen_t &client_len, char* buffer, size_t buffSize);

    private:
};

#endif // DISCOVERYSUBSURVICE_H

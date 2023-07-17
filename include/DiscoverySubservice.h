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
#include <netinet/if_ether.h>
#include "ManagementSubservice.h"
#include "WebServices.h"

class DiscoverySubservice
{
    public:
        DiscoverySubservice();
        virtual ~DiscoverySubservice();

        int InitializeServer();
        int InitializeClient(struct sockaddr_in &server_addr);




    protected:

    private:
};

#endif // DISCOVERYSUBSURVICE_H

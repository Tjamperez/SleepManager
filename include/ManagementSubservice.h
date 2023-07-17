#ifndef MANAGEMENTSUBSERVICE_H
#define MANAGEMENTSUBSERVICE_H

#include "ClientPC.h"
#include <iostream>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/socket.h>
#include <linux/if_ether.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <net/ethernet.h>
#include <netpacket/packet.h>

class ManagementSubservice
{
    public:
        ManagementSubservice();
        virtual ~ManagementSubservice();

        static bool AddPCToNetwork(std::string IP, std::string MAC);
        static const std::vector<ClientPC*> getNetwork();
        static void setPCStatus(std::string IP, std::string MAC, pcStatus status);
        static int awakePC(unsigned long int index);

    protected:
        static std::vector<ClientPC*> network;
        static std::mutex networkMutex;

    private:
};

#endif // MANAGEMENTSUBSERVICE_H
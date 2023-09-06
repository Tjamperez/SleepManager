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
#include <arpa/inet.h>
#include <ifaddrs.h>

class ManagementSubservice
{
    public:
        ManagementSubservice();
        virtual ~ManagementSubservice();

        static bool AddPCToNetwork(std::string IP, std::string MAC);
        static std::vector<NetworkPC> getNetwork();
        static void setPCStatus(std::string IP, std::string MAC, pcStatus status);
        static int awakePC(unsigned long int index);
        static void syncList(std::vector<NetworkPC> syncList);
        static void startElection();
        static bool isClient;
        static bool inElection;

    protected:
        static std::vector<NetworkPC> network;
        static std::mutex networkMutex;

    private:
};

#endif // MANAGEMENTSUBSERVICE_H

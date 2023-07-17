#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <thread>


#include "Interface.h"
#include "DiscoverySubservice.h"
#include "MonitoringSubservice.h"

int main(int argc, char* argv[])
{
    if (argc == 2)
    {
        if (strcmp("manager", argv[1]) == 0)
        {
            DiscoverySubservice serverDiscovery;
            MonitoringSubservice serverMonitor;
            std::thread discoveryThread(&DiscoverySubservice::InitializeServer, &serverDiscovery);
            std::thread monitoringThread(&MonitoringSubservice::runMonitoringServer, &serverMonitor);
            Interface interface;
            interface.startInterface();
            return 0;
        }
        else
        {
            std::cerr << "Argument: \"manager\" for manager or nothing for client.\n";
        }
    }
    else if (argc == 1)
    {
        DiscoverySubservice clientDiscovery;
        struct sockaddr_in server_addr;
        if (clientDiscovery.InitializeClient(server_addr))
            return -1;

        MonitoringSubservice clientMonitor;
        std::cout << "Starting monitoring.\n";
        clientMonitor.runMonitoringClient(server_addr);
    }
    else
    {
        std::cerr << "Argument: \"manager\" for manager or nothing for client.\n";
    }
}

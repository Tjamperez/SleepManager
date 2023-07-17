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


#include "interface.h"
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
            discoveryThread.join();
            monitoringThread.join();
            return 0;
        }
        else
        {
            std::cerr << "Argument: \"manager\" for manager or nothing for client.\n";
            return 0;
        }
    }
    else if (argc == 1)
    {
        int serverState = -1;
        DiscoverySubservice clientDiscovery;
        MonitoringSubservice clientMonitor;
        struct sockaddr_in server_addr;

        while (true)
        {
            std::cout << "Running client.\n";
            if (serverState == -1)
            {
                serverState = clientDiscovery.InitializeClient(server_addr);
                if (serverState == 1)
                    return -1;
            }
            else
            {
                std::cout << "Starting monitoring.\n";
                clientMonitor.runMonitoringClient(server_addr);
                serverState = -1;
            }
        }
    }
    else
    {
        std::cerr << "Argument: \"manager\" for manager or nothing for client.\n";
    }
}

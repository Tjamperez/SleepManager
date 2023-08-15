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

bool clRun = true;

int clientThread()
{
    DiscoverySubservice clientDiscovery;
    MonitoringSubservice clientMonitor;
    int serverState = -1;
    while (clRun)
    {
        if (serverState == -1)
        {
            std::cout << "Running client discovery.\n";
            serverState = clientDiscovery.InitializeClient();
            if (serverState == 1)
                return -1;
        }
        else
        {
            std::cout << "Starting monitoring.\n";
            clientMonitor.runMonitoringClient();
            serverState = -1;
        }
     }
}

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
        std::thread loopThread(clientThread);
        Interface interface;
        interface.startInterface(false);
        clRun = false;
        MonitoringSubservice::shutDown();
        DiscoverySubservice::shutDown();
        loopThread.join();
        
    }
    else
    {
        std::cerr << "Argument: \"manager\" for manager or nothing for client.\n";
    }
}

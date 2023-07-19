#ifndef INTERFACE_H
#define INTERFACE_H

#include "ManagementSubservice.h"
#include "MonitoringSubservice.h"
#include "DiscoverySubservice.h"
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

enum interfaceCommand {CMD_UNKNOWN, CMD_LIST, CMD_AWAKE, CMD_QUIT};

typedef struct _cmd
{
    int cmdType = CMD_UNKNOWN;
    std::vector<std::string> argv;
} icmd;

class Interface
{
    public:
        Interface();
        virtual ~Interface();

        void startInterface(bool server = true);

    protected:
        void interfaceLoop();
        void clientInterfaceLoop();
        static int runInterface;
        icmd parseCommand(const std::string &commandStr);

    private:
        void listNetworkPCs();
};

#endif // INTERFACE_H

#include "interface.h"

int Interface::runInterface = 1;

Interface::Interface()
{
    //ctor
}

Interface::~Interface()
{
    //dtor
}

void Interface::startInterface(bool server)
{
    if (server)
        interfaceLoop();
    else
        clientInterfaceLoop();
}

void Interface::listNetworkPCs()
{
    //std::cout << "Is the mutex locked?\n";
    const std::vector<ClientPC*> network = ManagementSubservice::getNetwork();
    //std::cout << "Not anymore!\n";
    for(long unsigned int i = 0; i < network.size(); i++)
    {
        std::cout << "Index: " << i << "\tIP: " << network[i]->getIP() << "\tMAC: " << network[i]->getMAC() << "\tStatus: ";
        switch (network[i]->getStatus())
        {
        case STATUS_SLEEPING:
            std::cout << "asleep";
            break;
        case STATUS_AWAKE:
            std::cout << "awake";
            break;
        }
        std::cout << "\n";
    }
}

void Interface::interfaceLoop()
{
    runInterface = 1;
    while (runInterface)
    {
        std::string commandStr = "";
        std::getline(std::cin, commandStr);

        icmd command = parseCommand(commandStr);

        switch (command.cmdType)
        {
        case CMD_UNKNOWN:
            std::cout << "Command unknown.\n";
            break;
        case CMD_LIST:
            listNetworkPCs();
            break;
        case CMD_AWAKE:
            ManagementSubservice::awakePC((unsigned long int)std::stoi(command.argv[0]));
            break;
        case CMD_QUIT:
            DiscoverySubservice::shutDown();
            MonitoringSubservice::shutDown();
            runInterface = 0;
            break;
        }
    }
}

void Interface::clientInterfaceLoop()
{
    runInterface = 1;
    while (runInterface)
    {
        std::string commandStr = "";
        std::getline(std::cin, commandStr);

        if (commandStr == "quit")
        {
            runInterface = 0;
            break;
        }
        else
        {
            std::cout << "Valid command: \"quit\"\n";
        }
    }
}

icmd Interface::parseCommand(const std::string &commandStr)
{
    icmd command;
    command.cmdType = CMD_UNKNOWN;

    if (commandStr == "")
        return command;

    std::stringstream cmdStream(commandStr);
    std::string currentTk;
    std::vector<std::string> cmdTks;

    while(std::getline(cmdStream, currentTk, ' '))
    {
        cmdTks.push_back(currentTk);
    }

    if (cmdTks[0] == "list")
    {
        command.cmdType = CMD_LIST;
    }
    else if (cmdTks[0] == "awake")
    {
        if (cmdTks.size() == 2)
        {
            bool isnum = true;
            for (char c : cmdTks[1])
            {
                if (!std::isdigit(c))
                {
                    isnum = false;
                    break;
                }
            }
            if (isnum)
            {
                command.cmdType = CMD_AWAKE;
                command.argv.push_back(cmdTks[1]);
            }
            else
                std::cout << "Awake needs the client index specified as a number.\n";
        }
        else
            std::cout << "Awake needs the client index specified as an argument.\n";
    }
    else if (cmdTks[0] == "quit")
    {
        if (cmdTks.size() == 1)
        {
            command.cmdType = CMD_QUIT;
        }
        else
        {
            std::cout << "Quit doesn't take any arguments.\n";
        }
    }
    return command;
}

#include "ElectionMonitor.h"


int ElectionMonitor::elmSockfd = 0;
struct sockaddr_in ElectionMonitor::election_addr = {};

ElectionMonitor::ElectionMonitor()
{

}

void ElectionMonitor::electionMonitoring()
{
    elmSockfd = 0;
    memset(&election_addr, 0, sizeof(election_addr));
    election_addr.sin_family = AF_INET;
    election_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    election_addr.sin_port = htons(ELECTION_START_PORT);

    WebServices::initializeSocket(elmSockfd, election_addr);

    int enableBroadcast = 1;
    if (setsockopt(elmSockfd, SOL_SOCKET, SO_BROADCAST, &enableBroadcast, sizeof(enableBroadcast)) < 0) 
    {
        perror("setsockopt failed");
        close(elmSockfd);
        return;
    }


    socklen_t client_len = sizeof(election_addr);
    char buffer[BUFFER_SIZE];

    while(!ManagementSubservice::shouldShutDown){

        if(!ManagementSubservice::inElection){
            basePacket electionPacket = WebServices::waitForResponse(elmSockfd, election_addr, 400);
            if(electionPacket.type == PTYPE_ELECTION_REQUEST){
                char ip[32];
                std::string myIP = WebServices::getIPAddress();
                strcpy(ip, electionPacket._payload);
                std::cout << "IP:" << ip << " Mine: " << myIP << "\n";
                std::cout << "Election monitor starting election\n";
                if (strcmp(myIP.c_str(), ip) == 0)
                    continue;
                ManagementSubservice::startElection();
            }
            else if (electionPacket.type != PTYPE_NULL)
            {
                std::cout << "Wrong packet type: " << electionPacket.type << "\n";
            }
        }
            
    }
    close(elmSockfd);

}
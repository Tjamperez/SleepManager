#include "ElectionMonitor.h"

ElectionMonitor::ElectionMonitor()
{

}

void ElectionMonitor::electionMonitoring()
{
    int sockfd = 0;
    struct sockaddr_in election_addr;
    memset(&election_addr, 0, sizeof(election_addr));
    election_addr.sin_family = AF_INET;
    election_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    election_addr.sin_port = htons(ELECTION_PORT);

    socklen_t client_len = sizeof(election_addr);
    char buffer[BUFFER_SIZE];

    while(!ManagementSubservice::shouldShutDown){

        if(!ManagementSubservice::inElection){
            basePacket electionPacket = WebServices::waitForResponse(sockfd, election_addr, 4000, nullptr);
            if(electionPacket.type == PTYPE_ELECTION_REQUEST){
                ManagementSubservice::startElection();
            }
        }
            
    }

}
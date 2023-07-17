#include "MonitoringSubservice.h"

#define MONITOR_PORT 15005
#define CLIENT_MONITOR_PORT 15008

MonitoringSubservice::MonitoringSubservice()
{
    //ctor
}

MonitoringSubservice::~MonitoringSubservice()
{
    //dtor
}



int MonitoringSubservice::runMonitoringServer()
{
    int sockfd;
    struct sockaddr_in server_addr;

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(MONITOR_PORT);

    WebServices::initializeSocket(sockfd, server_addr);

    basePacket request;
    request.type = PTYPE_SSR;
    while (true)
    {
        std::vector<ClientPC*> network = ManagementSubservice::getNetwork();
        for (unsigned long int i = 0; i < network.size(); i++)
        {
            struct sockaddr_in client_addr;
            memset(&client_addr, 0, sizeof(client_addr));
            client_addr.sin_family = AF_INET;
            inet_pton(AF_INET, network[i]->getIP().c_str(), &(client_addr.sin_addr));
            client_addr.sin_port = htons(CLIENT_MONITOR_PORT);

            socklen_t client_len = sizeof(client_addr);

            char* reqPacket = WebServices::serializePacket(request);
            ssize_t sent_bytes = sendto(sockfd, reqPacket, PACKET_SIZE, 0, (struct sockaddr *)&client_addr, client_len);

            basePacket response = WebServices::waitForResponse(sockfd, client_addr, 2);

            if (response.type != PTYPE_SSR_RESP)
            {
                sent_bytes = sendto(sockfd, reqPacket, PACKET_SIZE, 0, (struct sockaddr *)&client_addr, client_len);
                response = WebServices::waitForResponse(sockfd, client_addr, 2);

                if (response.type != PTYPE_SSR_RESP)
                {
                    ManagementSubservice::setPCStatus(network[i]->getIP(), network[i]->getMAC(), STATUS_SLEEPING);
                }
                else
                {
                    ManagementSubservice::setPCStatus(network[i]->getIP(), network[i]->getMAC(), STATUS_AWAKE);
                }
            }
            else
            {
                ManagementSubservice::setPCStatus(network[i]->getIP(), network[i]->getMAC(), STATUS_AWAKE);
            }
        }
        sleep(1);
    }
    std::cerr << "Returning form monitoring SS\n";
    return 0;

}

int MonitoringSubservice::runMonitoringClient(const struct sockaddr_in server_addr)
{
    int sockfd;
    struct sockaddr_in myAddr;
    memset(&myAddr, 0, sizeof(myAddr));
    myAddr.sin_family = AF_INET;
    myAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myAddr.sin_port = htons(CLIENT_MONITOR_PORT);

    if (WebServices::initializeSocket(sockfd, myAddr))
    {
        std::cerr << "Client monitor error.\n";
        exit(2);
    }
    char buffer[BUFFER_SIZE];
    socklen_t server_len = sizeof(server_addr);

    basePacket sendPack;
    sendPack.type = PTYPE_SSR_RESP;
    while (true)
        {

            ssize_t num_bytes = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0, (struct sockaddr*)&server_addr, &server_len);

            char* response = WebServices::serializePacket(sendPack);

            ssize_t sent_bytes = sendto(sockfd, response, PACKET_SIZE, 0, (struct sockaddr *)&server_addr, server_len);

            if (sent_bytes < 0)
                std::cerr << "sendto failed in client monitoring ss\n";
            /*else
                std::cerr <<"Sent response packet.\n";*/
        }
}

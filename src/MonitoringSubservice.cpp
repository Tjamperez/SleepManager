#include "MonitoringSubservice.h"


int MonitoringSubservice::runMonitor = 1;

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
    runMonitor = 1;
    int sockfd;
    struct sockaddr_in server_addr;

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(MONITOR_PORT);

    WebServices::initializeSocket(sockfd, server_addr);

    basePacket request;
    request.type = PTYPE_MONITOR_PROBE;
    while (runMonitor == 1)
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

            if (response.type != PTYPE_MONITORING_PROBE_RESP)
            {
                sent_bytes = sendto(sockfd, reqPacket, PACKET_SIZE, 0, (struct sockaddr *)&client_addr, client_len);
                response = WebServices::waitForResponse(sockfd, client_addr, 2);

                if (response.type != PTYPE_MONITORING_PROBE_RESP)
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
            //std::cerr << "PTYPE: " << packetTypesNames[response.type] << "\n";
        }
        usleep(2000);
    }
    basePacket shutdownPacket{};
    shutdownPacket.type = PTYPE_SERVER_SHUTDOWN;
    char* serPacket = WebServices::serializePacket(shutdownPacket);
    std::vector<ClientPC*> network = ManagementSubservice::getNetwork();
    for (unsigned long int i = 0; i < network.size(); i++)
    {
        struct sockaddr_in client_addr;
        memset(&client_addr, 0, sizeof(client_addr));
        client_addr.sin_family = AF_INET;
        inet_pton(AF_INET, network[i]->getIP().c_str(), &(client_addr.sin_addr));
        client_addr.sin_port = htons(CLIENT_MONITOR_PORT);
        socklen_t client_len = sizeof(client_addr);
        ssize_t sent_bytes = sendto(sockfd, serPacket, PACKET_SIZE, 0, (struct sockaddr *)&client_addr, client_len);
    }

    close(sockfd);

    std::cerr << "Returning form monitoring server\n";
    return 0;
}

int MonitoringSubservice::runMonitoringSubservice(bool server){
    if (server == true){
        runMonitoringServer();
    }
    else{
        runMonitoringClient();
    }
}

int MonitoringSubservice::runMonitoringClient()
{
    runMonitor = 1;
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
    socklen_t server_len = sizeof(WebServices::server_addr);

    basePacket sendPack;
    sendPack.type = PTYPE_MONITORING_PROBE_RESP;
    char* response = WebServices::serializePacket(sendPack);
    while (runMonitor)
        {
            //Receber packet do server
            ssize_t num_bytes = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0, (struct sockaddr*)&WebServices::server_addr, &server_len);

            basePacket received = WebServices::deserializePacket(buffer);
            if (received.type == PTYPE_SERVER_SHUTDOWN)
            {
                std::cerr << "Server disconnecting.\n";
                runMonitor = 0;
                continue;
            }
            else if (received.type == PTYPE_MONITOR_PROBE)
            {
                ssize_t sent_bytes = sendto(sockfd, response, PACKET_SIZE, 0, (struct sockaddr *)&WebServices::server_addr, server_len);

                if (sent_bytes < 0)
                    std::cerr << "sendto failed in client monitoring ss\n";
                /*else
                    std::cerr <<"Sent response packet.\n";*/
            }
        }
    close(sockfd);
    std::cout << "Stopped monitoring.\n";

    basePacket sendPack;
    sendPack.type = PTYPE_MONITORING_PROBE_RESP;
    char* response = WebServices::serializePacket(sendPack);
    while (runMonitor)
        {
            //Receber packet do server
            ssize_t num_bytes = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0, (struct sockaddr*)&WebServices::server_addr, &server_len);

            basePacket received = WebServices::deserializePacket(buffer);
            if (received.type == PTYPE_SERVER_SHUTDOWN)
            {
                std::cerr << "Server disconnecting.\n";
                runMonitor = 0;
                continue;
            }
            else if (received.type == PTYPE_MONITOR_PROBE)
            {
                ssize_t sent_bytes = sendto(sockfd, response, PACKET_SIZE, 0, (struct sockaddr *)&WebServices::server_addr, server_len);

                if (sent_bytes < 0)
                    std::cerr << "sendto failed in client monitoring ss\n";
                /*else
                    std::cerr <<"Sent response packet.\n";*/
            }
        }
    close(sockfd);
    std::cout << "Finding Leader.\n";

    return 0;
}

void MonitoringSubservice::shutDown()
{
    runMonitor = 0;
}
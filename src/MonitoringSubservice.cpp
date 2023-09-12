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
    std::cout << "Starting monitoring server\n";
    int sockfd;
    struct sockaddr_in server_addr;

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(MONITOR_PORT);

    WebServices::initializeSocket(sockfd, server_addr);

    basePacket request;
    request.type = PTYPE_MONITOR_PROBE;
    while (!ManagementSubservice::shouldShutDown && !ManagementSubservice::inElection && !ManagementSubservice::isClient)
    {
        //std::cout<<"IN WHILE2\n";
        std::vector<NetworkPC> network = ManagementSubservice::getNetwork();
        for (unsigned long int i = 0; i < network.size(); i++)
        {
            if (WebServices::getIPAddress() == network[i].getIP())
            {
                ManagementSubservice::setPCStatus(network[i].getIP(), network[i].getMAC(), STATUS_AWAKE);
                continue;
            }
            
            struct sockaddr_in client_addr;
            memset(&client_addr, 0, sizeof(client_addr));
            client_addr.sin_family = AF_INET;
            inet_pton(AF_INET, network[i].getIP().c_str(), &(client_addr.sin_addr));
            client_addr.sin_port = htons(CLIENT_MONITOR_PORT);

            socklen_t client_len = sizeof(client_addr);

            char* reqPacket = WebServices::serializePacket(request);
            ssize_t sent_bytes = sendto(sockfd, reqPacket, PACKET_SIZE, 0, (struct sockaddr *)&client_addr, client_len);

            basePacket response = WebServices::waitForResponse(sockfd, client_addr, 500, nullptr);

            if (response.type != PTYPE_MONITORING_PROBE_RESP)
            {
                sent_bytes = sendto(sockfd, reqPacket, PACKET_SIZE, 0, (struct sockaddr *)&client_addr, client_len);
                response = WebServices::waitForResponse(sockfd, client_addr, 500, nullptr);

                if (response.type != PTYPE_MONITORING_PROBE_RESP)
                {
                    ManagementSubservice::setPCStatus(network[i].getIP(), network[i].getMAC(), STATUS_SLEEPING);
                    //std::cout << "Is asleep\n";
                }
                else
                {
                    ManagementSubservice::setPCStatus(network[i].getIP(), network[i].getMAC(), STATUS_AWAKE);
                    //std::cout << "Received probe response\n";
                }
            }
            else
            {
                ManagementSubservice::setPCStatus(network[i].getIP(), network[i].getMAC(), STATUS_AWAKE);
                //std::cout << "Received probe response\n";
            }
            //std::cerr << "PTYPE: " << packetTypesNames[response.type] << "\n";
        }
        usleep(60);
    }
    basePacket shutdownPacket{};
    shutdownPacket.type = PTYPE_SERVER_SHUTDOWN;
    char* serPacket = WebServices::serializePacket(shutdownPacket);
    std::vector<NetworkPC> network = ManagementSubservice::getNetwork();
    for (unsigned long int i = 0; i < network.size(); i++)
    {
        struct sockaddr_in client_addr;
        memset(&client_addr, 0, sizeof(client_addr));
        client_addr.sin_family = AF_INET;
        inet_pton(AF_INET, network[i].getIP().c_str(), &(client_addr.sin_addr));
        client_addr.sin_port = htons(CLIENT_MONITOR_PORT);
        socklen_t client_len = sizeof(client_addr);
        ssize_t sent_bytes = sendto(sockfd, serPacket, PACKET_SIZE, 0, (struct sockaddr *)&client_addr, client_len);
    }

    basePacket server_resp;
    server_resp.type = PTYPE_SERVER_PROBE_RESP;
    char* response = WebServices::serializePacket(server_resp);
    char buffer[BUFFER_SIZE];
    socklen_t server_len = sizeof(WebServices::server_addr);
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);


    /*while (!ManagementSubservice::shouldShutDown && !ManagementSubservice::inElection)
        {
            //Receber packet do server
            ssize_t num_bytes = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0, (struct sockaddr*)&client_addr, &client_len);

            basePacket received = WebServices::deserializePacket(buffer);
            if (received.type == PTYPE_SERVER_PROBE)
            {
                ssize_t sent_bytes = sendto(sockfd, response, PACKET_SIZE, 0, (struct sockaddr *)&client_addr, client_len);

                if (sent_bytes < 0)
                    std::cerr << "sendto failed in client monitoring ss\n";
                //else
                //    std::cerr <<"Sent response packet.\n";
            }
        }*/


    close(sockfd);

    std::cerr << "Returning from monitoring server\n";
    return 0;
}

int MonitoringSubservice::runMonitoringSubservice(){
    while (!ManagementSubservice::shouldShutDown)
    {
        if (!ManagementSubservice::inElection)
        {
            if (ManagementSubservice::isClient){
                runMonitoringClient();
            }
            else{
                runMonitoringServer();
            }
        }
    }
}

int MonitoringSubservice::runMonitoringClient()
{
    std::cout << "Starting monitoring client\n";
    int sockfd;
    struct sockaddr_in myAddr;
    memset(&myAddr, 0, sizeof(myAddr));
    myAddr.sin_family = AF_INET;
    myAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myAddr.sin_port = htons(CLIENT_MONITOR_PORT);

    if (WebServices::initializeSocket(sockfd, myAddr))
    {
        perror("Couldn't initialize client monitoring socket");
        std::cerr << "Client monitor error.\n";
        exit(2);
    }
    char buffer[BUFFER_SIZE];

    basePacket sendPack;
    sendPack.type = PTYPE_MONITORING_PROBE_RESP;
    char *response = WebServices::serializePacket(sendPack);
    struct sockaddr_in newServer;
    memset(&newServer, 0, sizeof(newServer));
    newServer.sin_family = AF_INET;
    newServer.sin_addr.s_addr = htonl(INADDR_ANY);
    newServer.sin_port = htons(CLIENT_MONITOR_PORT);
    std::string IP;
    std::string IPWeb;

    socklen_t server_len = sizeof(newServer);

    // Receber packet do server
    /*ssize_t num_bytes = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0, (struct sockaddr*)&newServer, &server_len);*/

    ///////////////////////////////////////

    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

    /*fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(sockfd, &readSet);*/

    /*int enableBroadcast = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &enableBroadcast, sizeof(enableBroadcast)) < 0)
    {
        perror("setsockopt failed");
        close(sockfd);
        return 1;
    }*/

    /*timeval timeout;
    timeout.tv_sec = 4;
    timeout.tv_usec = 0;*/

    basePacket received;// = WebServices::deserializePacket(buffer);

    while (ManagementSubservice::isClient && !ManagementSubservice::shouldShutDown && !ManagementSubservice::inElection)
    {
        //std::cout << "WAITING\n";
        received = WebServices::waitForResponse(sockfd, newServer, 100000, &newServer);
        //std::cout<<"IN WHILE\n";
        if (received.type == PTYPE_NULL)
        {
            std::cout << "Starting election\n";
            ManagementSubservice::startElection();
            continue;
        }
        if (received.type == PTYPE_MONITOR_PROBE)
        {
            char srvIP[32];
            char nsrvIP[32];
            inet_ntop(AF_INET, &WebServices::server_addr.sin_addr, srvIP, sizeof(buffer));
            inet_ntop(AF_INET, &newServer.sin_addr, nsrvIP, sizeof(buffer));

            if (strcmp(srvIP, nsrvIP) == 0)
            {
                //std::cout << "Same IP\n";
                WebServices::server_addr = newServer;
            }
            else
            {
                ManagementSubservice::startElection();
                continue;
            }

            
            ssize_t sent_bytes = sendto(sockfd, response, PACKET_SIZE, 0, (struct sockaddr *)&WebServices::server_addr, server_len);
            if (sent_bytes < 0)
            {
                perror("Error sending probe response");
            }
        }
    }
    close(sockfd);
    std::cout << "Stopped client monitoring.\n";
    return 0;
}

/*sendPack.type = PTYPE_SERVER_PROBE;
char* reqPack = WebServices::serializePacket(sendPack);
int count = 0;
while (!ManagementSubservice::shouldShutDown && !ManagementSubservice::inElection)
    {
        //Mandar packet pro server
        ssize_t sent_bytes = sendto(sockfd, reqPack, PACKET_SIZE, 0, (struct sockaddr *)&WebServices::server_addr, server_len);

        basePacket response = WebServices::waitForResponse(sockfd, WebServices::server_addr, 2);
        if (response.type == PTYPE_SERVER_SHUTDOWN)
        {
            std::cerr << "Server disconnecting.\n";
            break;
        }
        else if(response.type != PTYPE_SERVER_PROBE_RESP && count == 3)
        {
            std::cerr << "Houston we have a problem.\n";
            //Start election NOW!!!!!!
            ManagementSubservice::startElection();
        }
        usleep(2000);
        count++;
    }
std::cout << "Finding Leader.\n";*/

void MonitoringSubservice::shutDown()
{
    runMonitor = 0;
}
#include "DiscoverySubservice.h"
#include "ClientPC.h"

bool serverFound = false;

int DiscoverySubservice::runDiscovery = 1;

DiscoverySubservice::DiscoverySubservice()
{
    //ctor
}

DiscoverySubservice::~DiscoverySubservice()
{
    //dtor
}

int DiscoverySubservice::run()
{
    while (runDiscovery)
    {
        if (!ManagementSubservice::inElection)
        {
            if (ManagementSubservice::isClient)
            {
                if (!serverFound)
                    InitializeClient();
            }
            else
            {
                InitializeServer();
            }
        }
    }
    return 0;
}

int DiscoverySubservice::listenForBroadcasts(int &sockfd, struct sockaddr_in client_addr,  socklen_t &client_len, char* buffer, size_t buffSize)
{
    runDiscovery = 1;
    basePacket sendPack;
    sendPack.type = PTYPE_DISCOVERY_ACK;

    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(sockfd, &readSet);

    int enableBroadcast = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &enableBroadcast, sizeof(enableBroadcast)) < 0) 
    {
        perror("setsockopt failed");
        close(sockfd);
        return 1;
    }

    timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 5000;

    while (!ManagementSubservice::isClient && runDiscovery)
    {
        memset(buffer, 0, buffSize);
        /*ssize_t num_bytes = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0, (struct sockaddr*)&client_addr, &client_len);
        if (num_bytes < 0)
        {
            std::cerr << "recvfrom failed in listen for broadcasts\n";
            return 1;
        }*/

        fd_set tempSet = readSet;
        int result = select(sockfd + 1, &tempSet, nullptr, nullptr, &timeout);
        if (result == -1) 
        {
            std::cerr << "select failed\n";
            return 1;
        }
        else if (result == 0) 
        {
            // Timeout
            continue;
        }
        else 
        {
            // Dados prontos
            if (FD_ISSET(sockfd, &tempSet)) 
            {
                ssize_t num_bytes = recvfrom(sockfd, buffer, buffSize - 1, 0, (struct sockaddr*)&client_addr, &client_len);
                if (num_bytes < 0) 
                {
                    std::cerr << "recvfrom failed in listen for broadcasts\n";
                    return 1;
                }
            basePacket receivedPacket = WebServices::deserializePacket(buffer);

            if (receivedPacket.type != PTYPE_DISCOVERY)
                continue;

            // Pega o IP do cliente
            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);

            //Pega o MAC do cliente (Linux apenas)
            std::string client_mac(receivedPacket._payload);

            std::cout << "Received broadcast from: " << client_ip << std::endl;
            std::cout << "MAC address: " << client_mac << std::endl;

            //Responder

            char* response = WebServices::serializePacket(sendPack);

            std::cout <<"Sent: " << packetTypesNames[WebServices::deserializePacket(response).type] << "\n";

            ssize_t sent_bytes = sendto(sockfd, response, PACKET_SIZE, 0, (struct sockaddr *)&client_addr, client_len);
            if (sent_bytes < 0)
            {
                  std::cerr << "sendto failed\n";
                 return 1;
            }

            if (ManagementSubservice::AddPCToNetwork(client_ip, client_mac))
                std::cout << "Response sent to the client." << std::endl;
            
            }
        }

    }
    return 0;
}

int DiscoverySubservice::InitializeServer()
{
    int sockfd = 0;
    struct sockaddr_in server_addr, client_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT);

    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    if (!WebServices::initializeSocket(sockfd, server_addr))
    {
        if (!listenForBroadcasts(sockfd, client_addr, client_len, buffer, sizeof(buffer)))
        {
            close(sockfd);
        }
    }
    std::cerr << "Returning form discovery server\n";
    return 1;
}

int DiscoverySubservice::InitializeClient()
{
    runDiscovery = 1;
    int sockfd;
    //socklen_t server_len = sizeof(server_addr);

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed");
        return 1;
    }
    int broadcastEnable=1;
    int ret=setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));

    // Fazer o socket n�o bloquear a thread
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags < 0)
    {
        perror("fcntl failed");
        return 1;
    }
    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0)
    {
        perror("fcntl failed");
        return 1;
    }

    memset(&WebServices::server_addr, 0, sizeof(WebServices::server_addr));
    WebServices::server_addr.sin_family = AF_INET;
    WebServices::server_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    WebServices::server_addr.sin_port = htons(SERVER_PORT);

    serverFound = false;

    basePacket rtPacket;
    rtPacket.type = PTYPE_NULL;

    basePacket p;
    p.type = PTYPE_DISCOVERY;
    strcpy(p._payload, WebServices::getMACAddress().c_str());
    int count = 0;

    // Espera o server responder
    while (!serverFound && ManagementSubservice::isClient && runDiscovery)
    {
        // Mandar pacote de broadcast
        if (WebServices::sendBroadcast(sockfd, WebServices::server_addr, p))
        {
            // Esperar resposta do server
            rtPacket = WebServices::waitForResponse(sockfd, WebServices::server_addr, 2);
            if (rtPacket.type == PTYPE_DISCOVERY_ACK)
            {
                serverFound = true;
            }
        }
        usleep(2000);
        count++;
        if(serverFound == false && count == 3)
            {
               //mandar management iniciar eleição
			   ManagementSubservice::startElection();//mandar management iniciar eleição
            } 
    }

    // Fechar o socket
    close(sockfd);

    return 0;
}



void DiscoverySubservice::shutDown()
{
    runDiscovery = 0;
}


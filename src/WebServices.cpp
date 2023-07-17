#include "WebServices.h"

std::vector<std::string> packetTypesNames = {"PTYPE_NULL", "PTYPE_DISCOVERY", "PTYPE_DISCOVERY_ACK", "PTYPE_SSR", "PTYPE_SSR_RESP"};

WebServices::WebServices()
{
    //ctor
}

WebServices::~WebServices()
{
    //dtor
}

 //packet

char* WebServices::serializePacket(basePacket p)
{
    char* serializedData = new char[132];

    uint16_t type = htons(p.type);
    uint16_t timestamp = htons(p.timestamp);
    memcpy(serializedData, &type, sizeof(uint16_t));
    memcpy(serializedData + sizeof(uint16_t), &timestamp, sizeof(uint16_t));
    memcpy(serializedData + 2 * sizeof(uint16_t), p._payload, 128);

    return serializedData;
}

basePacket WebServices::deserializePacket(char* serializedData) {
    basePacket p{};

    memcpy(&p.type, serializedData, sizeof(uint16_t));
    memcpy(&p.timestamp, serializedData + sizeof(uint16_t), sizeof(uint16_t));
    memcpy(p._payload, serializedData + 2 * sizeof(uint16_t), 128);

    p.type = ntohs(p.type);
    p.timestamp = ntohs(p.timestamp);

    return p;
}

bool WebServices::sendBroadcast(int sockfd, const struct sockaddr_in server_addr, basePacket p)
{
    const char* broadcastMessage = WebServices::serializePacket(p);
    ssize_t num_bytes = sendto(sockfd, broadcastMessage, strlen(broadcastMessage), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (num_bytes < 0)
    {
        std::cerr << "sendto failed\n";
        return false;
    }

    //std::cerr << "Broadcast sent!" << std::endl;
    return true;
}

basePacket WebServices::waitForResponse(int sockfd, struct sockaddr_in server_addr,long timeOutMs)
{
    basePacket response;
    response.type = PTYPE_NULL;
    struct timeval timeout;
    timeout.tv_sec = timeOutMs / 1000;
    timeout.tv_usec = (timeOutMs % 1000) * 1000;

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);

    int result = select(sockfd + 1, &readfds, nullptr, nullptr, &timeout);
    if (result == 0)
    {
        // Timeout
        //std::cout << "Timeout: No response received." << std::endl;
        return response;
    }
    else if (result > 0)
    {
        // Se o socket recebeu algo
        if (FD_ISSET(sockfd, &readfds))
        {
            // Receber resposta do server
            struct sockaddr_in response_addr;
            socklen_t response_len = sizeof(response_addr);
            char buffer[BUFFER_SIZE];
            memset(buffer, 0, sizeof(buffer));

            ssize_t received_bytes = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&response_addr, &response_len);
            if (received_bytes < 0)
            {
                std::cerr << "recvfrom failed\n";
                return response;
            }

            // Print da resposta do server
            response = deserializePacket(buffer);
            /*std::cout << "Received response from server\n";
            std::cout << "Type: " << packetTypesNames[response.type] << "\n";*/
            return response;
        }
    }
    else
    {
        // Erro na seleção
        std::cerr << "select failed\n";
        return response;
    }
    return response;
}

bool WebServices::initializeSocket(int &sockfd, const struct sockaddr_in &server_addr)
{
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        std::cerr << "socket creation failed\n";
        return 1;
    }

    /*memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT);*/

    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        std::cerr << "bind failed\n";
        return 1;
    }
    return 0;
}

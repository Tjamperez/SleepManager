#include "WebServices.h"

std::vector<std::string> packetTypesNames = {"PTYPE_NULL", "PTYPE_DISCOVERY", "PTYPE_DISCOVERY_ACK", "PTYPE_SSR", "PTYPE_SSR_RESP", "PTYPE_SERVER_SHUTDOWN"};

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

/*std::string getMACAddress(std::string interface)
{
    struct ifreq ifr;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
     {
        perror("socket creation failed");
        return "";
    }
    std::strcpy(ifr.ifr_name, interface.c_str());
    if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) < 0)
    {
        perror("ioctl failed");
        return "";
    }
    close(sockfd);

    const unsigned char* mac = reinterpret_cast<unsigned char*>(ifr.ifr_hwaddr.sa_data);
    char macAddress[18];

    std::sprintf(macAddress, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    return macAddress;
}*/

using IpAddress = std::array<uint8_t, 4>;

std::string getMACAddress()
{
    using namespace std;
    array<uint8_t, 6> mac;
    struct ifaddrs *ifaddr_list;
    getifaddrs(&ifaddr_list);
    struct ifaddrs *ifaddr_curr = ifaddr_list;
    bool found_mac = false;
    while (ifaddr_curr != nullptr && !found_mac) {
        int fd = socket(AF_INET, SOCK_DGRAM, 0);
        struct ifreq ifreq;
        if (fd < 0) {
            perror("socket for finding MAC");
        }
        strcpy(ifreq.ifr_name, ifaddr_curr->ifa_name);
        if (ioctl(fd, SIOCGIFADDR, &ifreq) >= 0) {
            struct sockaddr_in *inet_addr =
                (struct sockaddr_in *) &ifreq.ifr_addr;
            IpAddress if_ip_addr;
            copy_n(
                (uint8_t *) &inet_addr->sin_addr.s_addr,
                4,
                if_ip_addr.begin()
            );
            if (true /*if_ip_addr == this->ip*/) {
                if (ioctl(fd, SIOCGIFHWADDR, &ifreq) < 0) {
                    perror("get MAC address");
                }
                copy_n(
                    (uint8_t *) &ifreq.ifr_hwaddr.sa_data,
                    6,
                    mac.begin()
                );
                found_mac = true;
            }
        }
        ifaddr_curr = ifaddr_curr->ifa_next;
    }
    freeifaddrs(ifaddr_list);

    ostringstream oss;
    oss << hex << uppercase;
    for (const auto& byte : mac)
    {
        oss << setw(2) << setfill('0') << static_cast<int>(byte) << ":";
    }

    string mac_address = oss.str();
    // Remove the trailing ':' character
    mac_address.pop_back();

    if (!found_mac) {
        //cerr << "couldn't get mac!\n";
        return "ERR";
    }
    return mac_address;
}


bool WebServices::sendBroadcast(int sockfd, const struct sockaddr_in &server_addr, basePacket p)
{
    strcpy(p._payload, getMACAddress().c_str());
    //std::cout << "MAC: " << p._payload << "\n";
    char* broadcastMessage = WebServices::serializePacket(p);
    ssize_t num_bytes = sendto(sockfd, broadcastMessage, PACKET_SIZE, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
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
        // Erro na sele��o
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

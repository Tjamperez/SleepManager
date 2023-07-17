#include "ManagementSubservice.h"

std::vector<ClientPC*> ManagementSubservice::network = {};
std::mutex ManagementSubservice::networkMutex;

ManagementSubservice::ManagementSubservice()
{
    //ctor
}

ManagementSubservice::~ManagementSubservice()
{
    //dtor
}

const std::vector<ClientPC*> ManagementSubservice::getNetwork()
{
    networkMutex.lock();
    const std::vector<ClientPC*> ret = network;
    networkMutex.unlock();
    return ret;
}

bool ManagementSubservice::AddPCToNetwork(std::string IP, std::string MAC)
{
    networkMutex.lock();
    for (auto & element : network)
    {
        if (IP == element->getIP() && MAC == element->getMAC())
        {
            networkMutex.unlock();
            return false;
        }
    }
    ClientPC* newPC = new ClientPC(IP, MAC);
    network.push_back(newPC);
    networkMutex.unlock();
    return true;
}

void ManagementSubservice::setPCStatus(std::string IP, std::string MAC, pcStatus status)
{
    networkMutex.lock();
    ClientPC* found = nullptr;
    for (auto & element : network)
    {
        if (element->getIP() == IP && element->getMAC() == MAC)
        {
            found = element;
            break;
        }
    }
    if (found != nullptr)
    {
        found->setStatus(status);
    }
    networkMutex.unlock();
}

std::string getServerMac()
{
    std::string macAddress;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        std::cerr << "Failed to create socket\n";
        return macAddress;
    }

    struct ifreq ifr{};
    strcpy(ifr.ifr_name, "eth0");  // Replace "eth0" with the relevant network interface name

    if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) != -1) {
        unsigned char* mac = reinterpret_cast<unsigned char*>(ifr.ifr_hwaddr.sa_data);
        char macStr[18];
        std::snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        macAddress = macStr;
    } else {
        std::cerr << "Failed to retrieve MAC address\n";
    }

    close(sockfd);

    return macAddress;
}

int ManagementSubservice::awakePC(unsigned long int index)
{
    std::string IP = network[index]->getIP();
    std::string MAC = network[index]->getMAC();

    int sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockfd < 0)
    {
        std::cerr << "Failed to create magic packet socket.\n";
        return 1;
    }

    //Obter o indice da interface eth0
    struct ifreq ifr{};
    strcpy(ifr.ifr_name, "eth0");

    if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1)
    {
    std::cerr << "Failed to obtain interface index\n";
    close(sockfd);
    return 1;
    }
    int if_index = ifr.ifr_ifindex;

    //Construir o quadro de ethernet
    struct ether_header eth_header{};
    eth_header.ether_type = htons(ETH_P_ALL);
    memcpy(eth_header.ether_shost, getServerMac().c_str(), 6);
    memcpy(eth_header.ether_dhost, MAC.c_str(), 6);

    //Construir o packet completo
    char packet[sizeof(struct ether_header) + 102];
    memcpy(packet, &eth_header, sizeof(struct ether_header));


    memset(packet, 0xFF, 6);

    for (int i = 6; i < 102; i += 6)
    {
        memcpy(packet + i, MAC.c_str(), 6);
    }

    struct sockaddr_ll sa{};
    sa.sll_family = AF_PACKET;
    sa.sll_protocol = htons(ETH_P_ALL);
    sa.sll_ifindex = if_index;

    int sent_bytes = sendto(sockfd, packet, sizeof(packet), 0, (struct sockaddr*)&sa, sizeof(sa));

    if (sent_bytes < 0)
    {
        std::cerr << "Failed to send Wake-on-LAN packet\n";
        close(sockfd);
        return 1;
    }

    close(sockfd);
    return 0;
}

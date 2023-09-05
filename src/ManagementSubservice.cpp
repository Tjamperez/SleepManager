#include "ManagementSubservice.h"

std::vector<NetworkPC> ManagementSubservice::network = {};
std::mutex ManagementSubservice::networkMutex;
bool ManagementSubservice::isClient = true;
bool ManagementSubservice::inElection = false;

ManagementSubservice::ManagementSubservice()
{
    //ctor
}

ManagementSubservice::~ManagementSubservice()
{
    //dtor
}

std::vector<NetworkPC> ManagementSubservice::getNetwork()
{
    networkMutex.lock();
    unsigned int ntSize = network.size();
    std::vector<NetworkPC> ret(ntSize);
    for (int i = ntSize - 1; i >= 0; i--)
    {
        ret[i] = network[i];
    }
    networkMutex.unlock();
    return ret;
}

bool ManagementSubservice::AddPCToNetwork(std::string IP, std::string MAC)
{
    networkMutex.lock();
    for (auto & element : network)
    {
        if (IP == element.getIP() && MAC == element.getMAC())
        {
            networkMutex.unlock();
            return false;
        }
    }
    NetworkPC newPC = NetworkPC(IP, MAC);
    network.push_back(newPC);
    networkMutex.unlock();
    return true;
}

void ManagementSubservice::setPCStatus(std::string IP, std::string MAC, pcStatus status)
{
    networkMutex.lock();
    unsigned int found = -1;
    for (unsigned int i = 0; i < network.size(); i++)
    {
        if (network[i].getIP() == IP && network[i].getMAC() == MAC)
        {
            found = i;
            break;
        }
    }
    if (found >= 0)
    {
        network[found].setStatus(status);
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
    networkMutex.lock();
    if (index >= network.size())
    {
        std::cerr << "Index out of bounds!\n";
        return 1;
    }
    std::string IP = network[index].getIP();
    std::string MAC = network[index].getMAC();
    networkMutex.unlock();

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        std::cerr << "Failed to create UDP socket.\n";
        return 1;
    }

    struct sockaddr_in sa{};
    sa.sin_family = AF_INET;
    sa.sin_port = htons(9);
    if (inet_pton(AF_INET, IP.c_str(), &(sa.sin_addr)) <= 0)
    {
        std::cerr << "Invalid IP address format.\n";
        close(sockfd);
        return 1;
    }

    // Encher pacote com 6 * 0xFF e 6 * o MAC
    char packet[102];
    memset(packet, 0xFF, 6);

    for (int i = 6; i < 102; i += 6)
    {
        memcpy(packet + i, MAC.c_str(), 6);
    }

    // Mandar pacote mágico
    ssize_t sent_bytes = sendto(sockfd, packet, sizeof(packet), 0, (struct sockaddr*)&sa, sizeof(sa));
    if (sent_bytes < 0)
    {
        std::cerr << "Failed to send Wake-on-LAN packet\n";
        close(sockfd);
        return 1;
    }

    close(sockfd);
    return 0;
}


void ManagementSubservice::syncList(std::vector<NetworkPC> syncList)
{
    networkMutex.lock();
    if (network.size() < syncList.size())
    {
        network.resize(syncList.size());
    }

    for (unsigned int i = 0; i < syncList.size(); i++)
    {
        network[i] = syncList[i];
    }
    networkMutex.unlock();
}
#include "ManagementSubservice.h"
#include "replicationSubservice.h"
#include "WebServices.h"
#include "ElectionMonitor.h"

std::vector<NetworkPC> ManagementSubservice::network = {};
std::mutex ManagementSubservice::networkMutex;
bool ManagementSubservice::isClient = true;
bool ManagementSubservice::inElection = false;
bool ManagementSubservice::shouldShutDown = false;
ReplicationSubservice *ManagementSubservice::RepManager = nullptr;
std::chrono::high_resolution_clock::time_point WebServices::startTime;


ManagementSubservice::ManagementSubservice()
{
    //ctor
}

ManagementSubservice::~ManagementSubservice()
{
    //dtor
}

/* int performElection(int currentProcessID, std::vector<ProcessInfo>& processes) {
    sendElectionRequest(currentProcessID, processes);

    int highestStatusProcessID = currentProcessID;

    // Simulate waiting for responses
    for (ProcessInfo& process : processes) {
        if (process.processID != currentProcessID) {
            // Simulate receiving and processing responses
            receiveElectionRequest(process.processID, processes);
            
            // Compare statuses and update the highest status process
            if (process.status > processes[highestStatusProcessID - 1].status) {
                highestStatusProcessID = process.processID;
            }
        }
    }

    // Election completed
    std::cout << "Process " << highestStatusProcessID << " is elected as the coordinator." << std::endl;
    
    return highestStatusProcessID;
} */


void ManagementSubservice::startElection()
{
    inElection = true;
    usleep(400);
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    int sockfd2 = socket(AF_INET, SOCK_DGRAM, 0);
    int broadcastEnable = 1;
    int ret = setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));
    ret = setsockopt(sockfd2, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));
    struct sockaddr_in myAddr,receptorAddr, myAddr2;
    memset(&myAddr, 0, sizeof(myAddr));
    myAddr.sin_family = AF_INET;
    myAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myAddr.sin_port = htons(ELECTION_PORT);

    /*memset(&myAddr2, 0, sizeof(myAddr2));
    myAddr2.sin_family = AF_INET;
    myAddr2.sin_addr.s_addr = htonl(INADDR_ANY);
    myAddr2.sin_port = htons(ELECTION_START_PORT);*/

    memset(&receptorAddr, 0, sizeof(myAddr));
    receptorAddr.sin_family = AF_INET;
    receptorAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    receptorAddr.sin_port = htons(ELECTION_PORT);

    basePacket rtPacket;
    rtPacket.type = PTYPE_NULL;

    std::string localIP = WebServices::getIPAddress();
    std::size_t machineID = std::hash<std::string>{}(localIP);
    std::string machineIDString = std::to_string(machineID);
    //std::vector<std::size_t> versionVector;
    socklen_t myAddr_len = sizeof(myAddr);
    basePacket sendRequest,leaderFound,sendResponse;

    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
    // Run the election
    std::vector<std::size_t> receivedMachineIDs; // Vector to store received machine IDs
    unsigned int allottedTimeMicros = 10000;

    if (bind(sockfd, (struct sockaddr*)&myAddr, sizeof(myAddr)) == -1) 
    {
        //std::cerr << "Error binding socket at line " << __LINE__  << " of file " << __FILE__ << std::endl;
        perror("Socket bind failed");
        std::cerr << "At line " << __LINE__ << " in file " << __FILE__ << "\n"; 
        close(sockfd);
        return;
    }
    /*if (bind(sockfd2, (struct sockaddr*)&myAddr2, sizeof(myAddr2)) == -1) 
    {
        //std::cerr << "Error binding socket at line " << __LINE__  << " of file " << __FILE__ << std::endl;
        perror("Socket bind failed");
        std::cerr << "At line " << __LINE__ << " in file " << __FILE__ << "\n"; 
        close(sockfd);
        return;
    }*/

    bool waitingForLeader = false;
        sendRequest.type = PTYPE_ELECTION_REQUEST;
        leaderFound.type = PTYPE_VICTORY_NOTIFICATION;
        sendResponse.timestamp = PTYPE_ELECTION_RESPONSE;
        unsigned long long version = RepManager->getListVersion();
        memcpy(leaderFound._payload,&version,sizeof(unsigned long long)); // if this is the leader it will have the version saved
        strcpy(sendRequest._payload,WebServices::getIPAddress().c_str());
        memcpy(sendResponse._payload,&version,sizeof(unsigned long long));
        bool broadcastSuccessful = false;
        while (!broadcastSuccessful)
        {
            broadcastSuccessful = WebServices::sendBroadcast(ElectionMonitor::elmSockfd, ElectionMonitor::election_addr, sendRequest);
            std::cout << "Broadcasting election\n";
            usleep(100);
        }
        if (WebServices::sendBroadcast(sockfd, myAddr, sendResponse)) 
        {
            // Receive and process responses from all participants
            receivedMachineIDs.clear(); // Clear the vector for each new election
            WebServices::startTimer();
            while (inElection and !(WebServices::hasElapsed(allottedTimeMicros))) 
            {
                basePacket receivedPacket = WebServices::waitForResponse(sockfd, receptorAddr, 80, nullptr);
               
                if (receivedPacket.type == PTYPE_NULL) 
                {
                    std::cout << "Received null packet\n";
                    continue; 
                } 
                else if (receivedPacket.type == PTYPE_ELECTION_RESPONSE) 
                {
                    std::cout << ("receivedPacket\n");
                    // Extract the sender's hashed IP address from the socket
                    memcpy(&version,receivedPacket._payload,sizeof(unsigned long long));

                    if(RepManager->getListVersion() == version)
                    {
                        //versionVector.push_back(version);
                        char senderIP[INET_ADDRSTRLEN];
                        inet_ntop(AF_INET, &(myAddr.sin_addr), senderIP, INET_ADDRSTRLEN);
                        
                        if (strcmp(WebServices::getIPAddress().c_str(), senderIP) == 0)
                        {
                            continue;
                        }
                        // Hash the sender's IP address
                        std::size_t senderMachineID = std::hash<std::string>{}(senderIP);

                        // Store the received machine ID in the vector
                        receivedMachineIDs.push_back(senderMachineID);
                    }
                    else if (RepManager->getListVersion() < version)
                    {
                        waitingForLeader = true;
                        break;
                    }
                }
            }       
        }
            //usleep(600);
        
        
        if (!waitingForLeader)
        {
            for (unsigned int i = 0; i < receivedMachineIDs.size(); i++)
            {
                if (receivedMachineIDs[i] > machineID)
                {
                    waitingForLeader = true;
                }
            }
            if (!waitingForLeader)
            {
                std::cout << "Setting self as server\n";
                if(WebServices::sendBroadcast(sockfd, myAddr, leaderFound))
                {
                    if(version == (RepManager->getListVersion()))
                    {
                        isClient = false;
                        inElection = false;
                    }
                }
            }
        }
        if (waitingForLeader)
        {
            basePacket receivedPacket = WebServices::waitForResponse(sockfd, myAddr, 80, &receptorAddr);
            if (receivedPacket.type == PTYPE_VICTORY_NOTIFICATION)
            {
                WebServices::server_addr = receptorAddr;
                isClient = true;
                inElection = false;
                std::cout << "Received leader\n";
            }
            else
            {
                std::cout << "NOT VICTORY\n";
            }
        }
        close(sockfd);
        close(sockfd2);

            // After processing all responses, determine the outcome of the election
            // by finding the highest machine ID among the received responses.


            

            /* std::size_t highestMachineID = machineID; // Initialize with own ID
            

            // Compare highestMachineID with your own machine ID to determine the outcome
            if (highestMachineID == machineID) {
                // You are the highest ID, become the coordinator
               
                // Perform coordinator actions here
            } else {
                // Another process has the highest ID, do not become the coordinator
            } */

            // Continue with the rest of your code for the election algorithm
}

/* void ManagementSubservice::startElection()
{
    int sockfd;
    int broadcastEnable=1;
    int ret=setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));
    struct sockaddr_in myAddr;
    memset(&myAddr, 0, sizeof(myAddr));
    myAddr.sin_family = AF_INET;
    myAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myAddr.sin_port = htons(ELECTION_PORT);

    basePacket rtPacket;
    rtPacket.type = PTYPE_NULL;

    std::string localIP = WebServices::getIPAddress();
    std::size_t machineID = std::hash<std::string>{}(localIP);
    std::string machineIDString = std::to_string(machineID);
    socklen_t myAddr_len = sizeof(myAddr);
    basePacket sendRequest;
    inElection = true;
    //rodar eleição
    while(inElection)
    {
        sendRequest.type = PTYPE_ELECTION_REQUEST;
        if (WebServices::sendBroadcastElection(sockfd, myAddr , sendRequest, machineIDString))
        {
            // Esperar resposta de todos os participantes:?!?!?!
            basePacket receivedPacket = waitForBroadcasts(sockfd, WebServices::server_addr, myAddr_len, 3);
            if (rtPacket.type == PTYPE_ELECTION_RESPONSE)
            {
                if (process.status > processes[highestStatusProcessID - 1].status) {
                highestStatusProcessID = process.processID;
            }
            }
        }
        
        //receber e enviar mensagens para decidir quem é o eleito
    }
}*/

basePacket ManagementSubservice::waitForBroadcasts(int sockfd, sockaddr_in client_addr, socklen_t &client_len, int timeoutInSeconds)
{
    basePacket nret;
    nret.type = PTYPE_NULL;
    char buffer[BUFFER_SIZE];
    size_t buffSize = sizeof(buffer);
    fd_set readSet;
    struct timeval timeout;

    FD_ZERO(&readSet);
    FD_SET(sockfd, &readSet);

    timeout.tv_sec = timeoutInSeconds;
    timeout.tv_usec = 0;

    int result = select(sockfd + 1, &readSet, nullptr, nullptr, &timeout);

    if (result < 0) {
        std::cerr << "select failed in waitForBroadcasts\n";
        return nret;
    } else if (result == 0) {
        // Timeout occurred, handle as needed
        std::cerr << "Timeout occurred while waiting for broadcasts\n";
        return nret;
    } else {
        ssize_t num_bytes = recvfrom(sockfd, buffer, buffSize - 1, 0, (struct sockaddr*)&client_addr, &client_len);

        if (num_bytes < 0) {
            std::cerr << "recvfrom failed in waitForBroadcasts\n";
            return nret;
        }

        basePacket receivedPacket = WebServices::deserializePacket(buffer); // Use WebServices function

        return receivedPacket;
    }
}
/* basePacket listenForBroadcasts(int &sockfd, struct sockaddr_in myAddr,  socklen_t &myAddr_len)
{
    basePacket nret;
    nret.type = PTYPE_NULL;
        char buffer[BUFFER_SIZE];
        size_t buffSize = sizeof(buffer);
        ssize_t num_bytes = recvfrom(sockfd, buffer, buffSize - 1, 0, (struct sockaddr*)&myAddr, &myAddr_len);
        if (num_bytes < 0) 
        {
            std::cerr << "recvfrom failed in listen for broadcasts\n";
            return nret;
        }
        basePacket receivedPacket = WebServices::deserializePacket(buffer);

        return receivedPacket;

} */


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
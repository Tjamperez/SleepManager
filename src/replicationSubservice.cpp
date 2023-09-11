#include "replicationSubservice.h"

ReplicationSubservice::ReplicationSubservice()
{
    this->mode = mode;
    this->managementS = managementS;
    listVersion = 0;
}

ReplicationSubservice::~ReplicationSubservice()
{

}

void ReplicationSubservice::changeMode(bool newMode)
{
    this->mode = newMode;
}

void ReplicationSubservice::serializePCListElement(listElement elem, basePacket &packet)
{
    char buff[128];
    char* buffLoc = buff;
    memcpy(buffLoc, &elem.position, sizeof(unsigned long long));
    buffLoc += sizeof(unsigned long long);
    memcpy(buffLoc, elem.ip, 32 * sizeof(char));
    buffLoc += sizeof(char) * 32;
    memcpy(buffLoc, elem.mac, 32 * sizeof(char));
    buffLoc += sizeof(char) * 32;
    memcpy(buffLoc, &elem.state, sizeof(int));

    memcpy(packet._payload, buff, 128 * sizeof(char));
    //std::cout << (char*)(packet._payload + sizeof(unsigned long long)) << "\n";
}

listElement ReplicationSubservice::deserializePCListElement(basePacket &packet)
{
    listElement ret;
    char* buffLoc = (char*)packet._payload;

    memcpy(&ret.position, buffLoc, sizeof(unsigned long long));
    buffLoc += sizeof(unsigned long long);
    memcpy(ret.ip, buffLoc, sizeof(char) * 32);
    buffLoc += sizeof(char) * 32;
    memcpy(ret.mac, buffLoc, sizeof(char) * 32);
    buffLoc += sizeof(char) * 32;
    memcpy(&ret.state, buffLoc, sizeof(int));
    
    return ret;
}

basePacket listenForBroadcasts(int &sockfd, struct sockaddr_in client_addr,  socklen_t &client_len)
{
    basePacket nret;
    nret.type = PTYPE_NULL;
        char buffer[BUFFER_SIZE];
        size_t buffSize = sizeof(buffer);
        ssize_t num_bytes = recvfrom(sockfd, buffer, buffSize - 1, 0, (struct sockaddr*)&client_addr, &client_len);
        if (num_bytes < 0) 
        {
            std::cerr << "recvfrom failed in listen for broadcasts\n";
            return nret;
        }
        basePacket receivedPacket = WebServices::deserializePacket(buffer);

        return receivedPacket;

}

unsigned int ReplicationSubservice::copyNetwork(unsigned long long version, std::vector<NetworkPC> &network)
{
    unsigned int networkS = network.size();
    if (networkS > pcList.size())
    {
        pcList.resize(networkS);
    }

    for (unsigned int i = 0; i < networkS; i++)
    {
        strcpy(pcList[i].ip, network[i].getIP().c_str());
        strcpy(pcList[i].mac, network[i].getMAC().c_str());
        pcList[i].state = network[i].getStatus();
        //std::cout << "I:" << i << "\n";
        pcList[i].position = i;
        pcList[i].version = version;
    }
    return networkS;
}

void ReplicationSubservice::syncList()
{
    unsigned int pcListSize = pcList.size();
    std::vector<NetworkPC> newList(pcListSize);
    
    for (unsigned int i = 0; i < pcListSize; i++)
    {
        NetworkPC syncPC(pcList[i].ip, pcList[i].mac);
        syncPC.setStatus((pcStatus)pcList[i].state);
        newList[i] = syncPC;
    }
    managementS->syncList(newList);
}

void ReplicationSubservice::runLoop()
{
    running = true;
    while (running)
    {
        if (!ManagementSubservice::inElection)
        {
            run();
        }
    }
}

void ReplicationSubservice::run()
{
    if (!ManagementSubservice::isClient)
    {
            int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
            if (sockfd == -1) {
                std::cerr << "Error creating socket" << std::endl;
                return;
            }

            int broadcastEnable = 1;
            if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable)) == -1) {
                std::cerr << "Error setting socket option" << std::endl;
                close(sockfd);
                return;
            }

            struct sockaddr_in address;
            memset(&address, 0, sizeof(address));
            address.sin_family = AF_INET;
            address.sin_addr.s_addr = htonl(INADDR_ANY);
            address.sin_port = htons(REPLICATION_PORT);


            // send size
            basePacket packet;

            std::cout << "Started server replication.\n";
            while (running && !ManagementSubservice::inElection)
            {
                listVersion += 1;

                std::vector<NetworkPC> network = managementS->getNetwork();
                unsigned int networkS = copyNetwork(listVersion, network);


                packet.type = PTYPE_LIST_SIZE;
                packet.timestamp = listVersion;
                unsigned long long listSize = pcList.size();
                memcpy(packet._payload, &listSize, sizeof(unsigned long long));

                WebServices::sendBroadcast(sockfd, address, packet);

                usleep(100);

                for (unsigned int i = 0; i < networkS; i++)
                {
                    
                    packet.type = PTYPE_LIST_ELEMENT;
                    packet.timestamp = (unsigned long long)(listVersion);
                    serializePCListElement(pcList[i], packet);
                    std::cout << "Sending " << pcList[i].position << " IP:" << pcList[i].ip << " Version: " << listVersion <<".\n";
                    WebServices::sendBroadcast(sockfd, address, packet);
                    usleep(100);
                }
                packet.type = PTYPE_LIST_END;
                WebServices::sendBroadcast(sockfd, address, packet);
                usleep(200);
            }

            // send list end
    }
    else
    {
        std::cout << "Started client replication.\n";
        int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd == -1) {
            std::cerr << "Error creating socket" << std::endl;
            return;
        }

        int broadcastEnable = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable)) == -1) {
            std::cerr << "Error setting socket option" << std::endl;
            close(sockfd);
            return;
        }

        struct sockaddr_in address;
        memset(&address, 0, sizeof(address));
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_ANY);
        address.sin_port = htons(REPLICATION_PORT);

        int flags = fcntl(sockfd, F_GETFL, 0);
        fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(sockfd, &readSet);

        timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 5000;
        if (bind(sockfd, (struct sockaddr*)&address, sizeof(address)) == -1) 
        {
            std::cerr << "Error binding socket" << std::endl;
            close(sockfd);
            return;
        }
        char buffer[BUFFER_SIZE];
        socklen_t sz = sizeof(address);
            // receive list element
        while (running && !ManagementSubservice::inElection)
        {
            //std::cout << "Waiting for broadcast.\n";
            basePacket packet;

            fd_set tempSet = readSet;
            int result = select(sockfd + 1, &tempSet, nullptr, nullptr, &timeout);
            if (result == -1) 
            {
                std::cerr << "select failed\n";
                return;
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
                    ssize_t num_bytes = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0, (struct sockaddr*)&address, &sz);
                    if (num_bytes < 0) 
                    {
                        std::cerr << "recvfrom failed in replication\n";
                        return;
                    }
                }
            }
            packet = WebServices::deserializePacket(buffer);
            unsigned long long newVersion = packet.timestamp;

            switch (packet.type)
            {
            case PTYPE_LIST_SIZE:
            {
                unsigned long long newSize;
                memcpy(&newSize, packet._payload, sizeof(unsigned long long));

                if (newSize > pcList.size())
                {
                    unsigned long long newElements = newSize - pcList.size();
                    std::cout << "Set new size: " << newSize << "\n";
                    for (unsigned long long i = 0; i < newElements; i++)
                    {
                        listElement newElem = {pcList.size(), "", "", false, packet.timestamp};
                        pcList.push_back(newElem);
                    }
                }
            }
            break;
            case PTYPE_LIST_ELEMENT:
            {
                listElement element = deserializePCListElement(packet);
                unsigned long long position = element.position;
                std::cout << "Received element " << element.position << " with ip: " << element.ip << " Version:" << element.version << "\n";
            if (position < pcList.size())
            {
                if (pcList[position].version < newVersion || (pcList[position].version > 65000 && newVersion < 200))
                {
                    element.version = newVersion;
                    pcList[position] = element;
                }
            }
            else
            {
                std::cerr << "Out of bounds!\n";
            }
            }
            break;
            case PTYPE_LIST_END:
            {
                std::cout << "End of list.\n";
                syncList();
            }
            break;
            dafault:
                std::cerr << "Received bad packet.\n";
            break;
            }
        }
    }
    return;
}

unsigned long long ReplicationSubservice::getListVersion()
{
    unsigned long long version = 0;

    for ( auto pcElem : pcList)
    {
        version = pcElem.version < version ? pcElem.version : version;
    }
    return version;
}
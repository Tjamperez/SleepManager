#ifndef REPLICATION_H
#define REPLICATION_H

#include "managementSubservice.h"
#include "WebServices.h"
#include <vector>

typedef struct _listElement
{
    unsigned long long position;
    char ip[32];
    char mac[32];
    int state;
    uint16_t version;
} listElement;

class ReplicationSubservice
{
    public:
        ReplicationSubservice();
        virtual ~ReplicationSubservice();
        void changeMode(bool newMode);
        void deactivate(){running = false;}
        void runLoop();
        uint16_t getListVersion();

        static void serializePCListElement(listElement elem, basePacket &packet);
        static listElement deserializePCListElement(basePacket &packet);
        
    
    private:
        void run();
        void syncList();
        unsigned int copyNetwork(uint16_t version,  std::vector<NetworkPC> &network);
        uint16_t listVersion;
        bool running = true;
        std::vector<listElement> pcList;
        ManagementSubservice* managementS;
        bool mode;
};

#endif
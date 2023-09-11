#ifndef REPLICATION_H
#define REPLICATION_H

#include "ManagementSubservice.h"
#include "WebServices.h"
#include <vector>

typedef struct _listElement
{
    unsigned long long position;
    char ip[32];
    char mac[32];
    int state;
    unsigned long long version;
} listElement;

class ReplicationSubservice
{
    public:
        ReplicationSubservice();
        virtual ~ReplicationSubservice();
        void changeMode(bool newMode);
        void deactivate(){running = false;}
        void runLoop();
        unsigned long long getListVersion();

        static void serializePCListElement(listElement elem, basePacket &packet);
        static listElement deserializePCListElement(basePacket &packet);
        
    
    private:
        void run();
        void syncList();
        unsigned int copyNetwork(unsigned long long version,  std::vector<NetworkPC> &network);
        unsigned long long listVersion;
        bool running = true;
        std::vector<listElement> pcList;
        ManagementSubservice* managementS;
        bool mode;
};

#endif
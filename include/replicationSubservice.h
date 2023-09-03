#ifndef REPLICATION_H
#define REPLICATION_H

#include "managementSubservice.h"

typedef struct _listElement
{
    int position;
    char ip[32];
    char mac[32];
    bool state;
} listElement;

class replicationSubsurvice
{
    public:
        replicationSubsurvice(bool mode, managementSubservice* managementS);
    
    private:
        run();
        managementSubservice* managementS;
        bool mode;
}

#endif
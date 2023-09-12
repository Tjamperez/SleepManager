#ifndef ELECTIONMONITOR_H
#define ELECTIONMONITOR_H

#include "ManagementSubservice.h"
#include "WebServices.h"
#include "MonitoringSubservice.h"

class ElectionMonitor{
    public:
        ElectionMonitor();
        void electionMonitoring(); 
        static int elmSockfd;
        static struct sockaddr_in election_addr;
    private:

};

#endif
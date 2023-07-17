#ifndef MONITORINGSUBSERVICE_H
#define MONITORINGSUBSERVICE_H

#include "WebServices.h"
#include "ManagementSubservice.h"

class MonitoringSubservice
{
    public:
        MonitoringSubservice();
        virtual ~MonitoringSubservice();

        int runMonitoringServer();
        int runMonitoringClient(struct sockaddr_in server_addr);

    protected:

    private:
};

#endif // MONITORINGSUBSERVICE_H

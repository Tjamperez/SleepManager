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
        int runMonitoringSubservice(bool server);
        int runMonitoringClient();
        static void shutDown();

    protected:
        static int runMonitor;

    private:
};

#endif // MONITORINGSUBSERVICE_H

#include "../include/work_station.h"
#include "../include/io_exception.h"
#include "../include/lock.h"
#include "../include/management.h"
#include <tuple>
#include <stdexcept>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <net/if.h>
#include <limits.h>
#include <algorithm>
#include <ifaddrs.h>
#include <cstring>

WorkStation::WorkStation(NodeAddresses addresses, WorkStation::Status status):
    addresses_(addresses),
    status_(status)
{
}

NodeAddresses WorkStation::addresses()
{
    return this->addresses_;
}

WorkStation::Status WorkStation::status()
{
    SharedLockGuard<shared_mutex> lock(this->rw_status_lock);
    return this->status_;
}

void ManagementService::dispatch_event(ManagementEvent event)
{
    for (function<void(ManagementEvent)> &handler : this->event_handlers) {
        handler(event);
    }
}


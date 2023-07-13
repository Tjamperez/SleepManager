#include "../include/work_station.h"
#include "../include/io_exception.h"
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

/*
class WriteLockGuard {
    private:
        shared_mutex &rw_mutex;
    public:
        WriteLockGuard(shared_mutex &rw_mutex_): rw_mutex(rw_mutex_)
        {
            this->rw_mutex.lock();
        }
        ~WriteLockGuard()
        {
            this->rw_mutex.unlock();
        }
};

class ReadLockGuard {
    private:
        shared_mutex &rw_mutex;
    public:
        ReadLockGuard(shared_mutex &rw_mutex_): rw_mutex(rw_mutex_)
        {
            this->rw_mutex.lock_shared();
        }
        ~ReadLockGuard() {
            this->rw_mutex.unlock_shared();
        }
};

bool WorkStationTable::insert(shared_ptr<WorkStation> node)
{
    WriteLockGuard lock(this->rw_mutex);
    bool inserted = get<1>(
        this->mac_address_index.insert(make_pair(node->mac_address, node))
    );
    if (inserted) {
        this->ip_address_index.insert(make_pair(node->ip_address, node));
    }
    return inserted;
}

shared_ptr<WorkStation> WorkStationTable::get_by_mac_address(
        MacAddress const &address)
{
    ReadLockGuard lock(this->rw_mutex);
    try {
        return this->mac_address_index.at(address);
    } catch (out_of_range exception) {
        return shared_ptr<WorkStation>(nullptr);
    }
}

shared_ptr<WorkStation> WorkStationTable::get_by_ip_address(
        IpAddress const &address)
{
    ReadLockGuard lock(this->rw_mutex);
    try {
        return this->ip_address_index.at(address);
    } catch (out_of_range exception) {
        return shared_ptr<WorkStation>(nullptr);
    }
}

vector<shared_ptr<WorkStation>> WorkStationTable::to_list()
{
    ReadLockGuard lock(this->rw_mutex);
    vector<shared_ptr<WorkStation>> vec(this->mac_address_index.size());
    for (auto entry : this->mac_address_index) {
        vec.push_back(get<1>(entry));
    }
    return vec;
}
*/

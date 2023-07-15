#include "../include/work_station.h"
#include "../include/io_exception.h"
#include "../include/lock.h"
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
    SharedLockGuard<shared_mutex> lock(this->rw_mutex);
    return this->status_;
}

bool WorkStationTable::insert(shared_ptr<WorkStation> node)
{
    lock_guard<shared_mutex> lock(this->rw_mutex);
    bool inserted = get<1>(
        this->mac_address_index.insert(make_pair(node->addresses().mac, node))
    );
    if (inserted) {
        this->ip_address_index.insert(make_pair(node->addresses().ip, node));
    }
    return inserted;
}

bool WorkStationTable::remove_by_mac_address(MacAddress const &address)
{
    lock_guard<shared_mutex> table_lock(this->rw_mutex);
    auto handle = this->mac_address_index.extract(address);
    if (handle.empty()) {
        return false;
    }
    this->ip_address_index.extract(handle.mapped()->addresses().ip);
    handle.mapped()->status_ = WorkStation::DISCONNECTED;
    return true;
}

bool WorkStationTable::remove_by_ip_address(IpAddress const &address)
{
    lock_guard<shared_mutex> table_lock(this->rw_mutex);
    auto handle = this->ip_address_index.extract(address);
    if (handle.empty()) {
        return false;
    }
    this->mac_address_index.extract(handle.mapped()->addresses().mac);
    lock_guard<shared_mutex> station_lock(handle.mapped()->rw_mutex);
    handle.mapped()->status_ = WorkStation::DISCONNECTED;
    return true;
}

shared_ptr<WorkStation> WorkStationTable::get_by_mac_address(
    MacAddress const &address)
{
    SharedLockGuard<shared_mutex> lock(this->rw_mutex);
    try {
        return this->mac_address_index.at(address);
    } catch (out_of_range exception) {
        return shared_ptr<WorkStation>(nullptr);
    }
}

shared_ptr<WorkStation> WorkStationTable::get_by_ip_address(
        IpAddress const &address)
{
    SharedLockGuard<shared_mutex> lock(this->rw_mutex);
    try {
        return this->ip_address_index.at(address);
    } catch (out_of_range exception) {
        return shared_ptr<WorkStation>(nullptr);
    }
}

vector<shared_ptr<WorkStation>> WorkStationTable::to_list()
{
    SharedLockGuard<shared_mutex> lock(this->rw_mutex);
    vector<shared_ptr<WorkStation>> vec(this->mac_address_index.size());
    for (auto entry : this->mac_address_index) {
        vec.push_back(get<1>(entry));
    }
    return vec;
}

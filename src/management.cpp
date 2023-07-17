#include "../include/management.h"
#include "../include/lock.h"

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


bool ManagementService::insert(shared_ptr<WorkStation> node)
{
    if (node->status() == WorkStation::DISCONNECTED) {
        return false;
    }
    lock_guard<shared_mutex> lock(this->rw_data_lock);
    bool inserted = get<1>(
        this->hostname_index.insert(make_pair(node->addresses().hostname, node))
    );
    if (inserted) {
        this->mac_address_index.insert(make_pair(node->addresses().mac, node));
        this->ip_address_index.insert(make_pair(node->addresses().ip, node));
        struct ManagementEvent event;
        event.type = ManagementEvent::INSERTION;
        event.work_station = node;
        this->dispatch_event(event);
    }
    return inserted;
}

void ManagementService::on_remove(shared_ptr<WorkStation> node)
{
    {
        lock_guard<shared_mutex> lock_guard(node->rw_status_lock);
        node->status_ = WorkStation::DISCONNECTED;
    }
    struct ManagementEvent event;
    event.type = ManagementEvent::REMOVAL;
    event.work_station = node;
    this->dispatch_event(event);
}

bool ManagementService::remove_by_mac_address(MacAddress const &address)
{
    lock_guard<shared_mutex> table_lock(this->rw_data_lock);
    auto handle = this->mac_address_index.extract(address);
    if (handle.empty()) {
        return false;
    }
    this->ip_address_index.extract(handle.mapped()->addresses().ip);
    this->hostname_index.extract(handle.mapped()->addresses().hostname);
    this->on_remove(handle.mapped());
    return true;
}

bool ManagementService::remove_by_ip_address(IpAddress const &address)
{
    lock_guard<shared_mutex> table_lock(this->rw_data_lock);
    auto handle = this->ip_address_index.extract(address);
    if (handle.empty()) {
        return false;
    }
    this->mac_address_index.extract(handle.mapped()->addresses().mac);
    this->hostname_index.extract(handle.mapped()->addresses().hostname);
    this->on_remove(handle.mapped());
    return true;
}

bool ManagementService::remove_by_hostname(string const &hostname)
{
    lock_guard<shared_mutex> table_lock(this->rw_data_lock);
    auto handle = this->hostname_index.extract(hostname);
    if (handle.empty()) {
        return false;
    }
    this->mac_address_index.extract(handle.mapped()->addresses().mac);
    this->ip_address_index.extract(handle.mapped()->addresses().ip);
    this->on_remove(handle.mapped());
    return true;
}

shared_ptr<WorkStation> ManagementService::get_by_mac_address(
    MacAddress const &address)
{
    SharedLockGuard<shared_mutex> lock(this->rw_data_lock);
    try {
        return this->mac_address_index.at(address);
    } catch (out_of_range exception) {
        return shared_ptr<WorkStation>(nullptr);
    }
}

shared_ptr<WorkStation> ManagementService::get_by_ip_address(
        IpAddress const &address)
{
    SharedLockGuard<shared_mutex> lock(this->rw_data_lock);
    try {
        return this->ip_address_index.at(address);
    } catch (out_of_range exception) {
        return shared_ptr<WorkStation>(nullptr);
    }
}

shared_ptr<WorkStation> ManagementService::get_by_hostname(
        string const &hostname)
{
    SharedLockGuard<shared_mutex> lock(this->rw_data_lock);
    try {
        return this->hostname_index.at(hostname);
    } catch (out_of_range exception) {
        return shared_ptr<WorkStation>(nullptr);
    }
}

bool ManagementService::update_sleep_status(
    MacAddress const& address,
    WorkStation::Status status)
{
    bool updated = false;
    shared_ptr<WorkStation> station = this->get_by_mac_address(address);
    if (bool(station)) {
        {
            lock_guard<shared_mutex> lock_guard(station->rw_status_lock);
            if (station->status_ != status) {
                station->status_ = status;
                updated = true;
            }
        }
        if (updated) {
            struct ManagementEvent event;
            event.type = ManagementEvent::CHANGED_STATUS;
            event.work_station = station;
            this->dispatch_event(event);
        }
    }
    return updated;
}

bool ManagementService::wakeup(string const& hostname)
{
    bool wokeup = false;
    shared_ptr<WorkStation> station = this->get_by_hostname(hostname);
    if (bool(station)) {
        {
            lock_guard<shared_mutex> lock_guard(station->rw_status_lock);
            if (station->status_ == WorkStation::ASLEEP) {
                station->status_ = WorkStation::AWAKEN;
                wokeup = true;
            }
        }
        if (wokeup) {
            struct ManagementEvent event;
            event.type = ManagementEvent::CHANGED_STATUS;
            event.work_station = station;
            this->dispatch_event(event);
        }
    }
    return wokeup;
}

vector<shared_ptr<WorkStation>> ManagementService::participant_list()
{
    SharedLockGuard<shared_mutex> lock(this->rw_data_lock);
    vector<shared_ptr<WorkStation>> vec(this->mac_address_index.size());
    for (auto entry : this->mac_address_index) {
        vec.push_back(get<1>(entry));
    }
    return vec;
}

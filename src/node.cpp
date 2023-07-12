#include "../include/node.h"
#include <tuple>
#include <stdexcept>

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

bool NodeCollection::insert(shared_ptr<Node> node)
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

shared_ptr<Node> NodeCollection::get_by_mac_address(MacAddress const &address)
{
    ReadLockGuard lock(this->rw_mutex);
    try {
        return this->mac_address_index.at(address);
    } catch (out_of_range exception) {
        return shared_ptr<Node>(nullptr);
    }
}

shared_ptr<Node> NodeCollection::get_by_ip_address(IpAddress const &address)
{
    ReadLockGuard lock(this->rw_mutex);
    try {
        return this->ip_address_index.at(address);
    } catch (out_of_range exception) {
        return shared_ptr<Node>(nullptr);
    }
}

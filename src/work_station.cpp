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

WorkStation::WorkStation(): mac_address {0}, ip_address {0}
{
}

WorkStation WorkStation::localhost()
{
    WorkStation work_station;

    char hostname_c[HOST_NAME_MAX + 1] = {0};
    if (gethostname(hostname_c, HOST_NAME_MAX + 1) < 0) {
        throw IOException("gethostname");
    }
    work_station.hostname = hostname_c;

    struct addrinfo addrinfo_hints = {0};
    struct addrinfo *addr_list;
    if (getaddrinfo(hostname_c, nullptr, &addrinfo_hints, &addr_list) < 0) {
        throw IOException("getaddrinfo for finding ipv4");
    }
    struct addrinfo *addr_curr = addr_list;
    bool found_ipv4 = false;
    while (addr_curr != nullptr && !found_ipv4) {
        if (addr_curr->ai_family == AF_INET) {
            copy_n(
                addr_curr->ai_addr->sa_data,
                4,
                work_station.ip_address.begin()
            );
            found_ipv4 = true;
        }
    }
    freeaddrinfo(addr_list);

    if (found_ipv4) {
        struct ifaddrs *ifaddr_list;
        getifaddrs(&ifaddr_list);
        struct ifaddrs *ifaddr_curr = ifaddr_list;
        bool found_mac = false;
        while (ifaddr_curr != nullptr && !found_mac) {
            int fd = socket(AF_INET, SOCK_DGRAM, 0);
            struct ifreq ifreq;
            if (fd < 0) {
                throw IOException("socket for finding MAC");
            }
            strcpy(ifreq.ifr_name, ifaddr_curr->ifa_name);
            if (ioctl(fd, SIOCGIFADDR, &ifreq) >= 0) {
                struct sockaddr_in *inet_addr =
                    (struct sockaddr_in *) &ifreq.ifr_addr;
                IpAddress if_ip_addr;
                copy_n(
                    (uint8_t *) &inet_addr->sin_addr.s_addr,
                    4,
                    if_ip_addr.begin()
                );
                if (if_ip_addr == work_station.ip_address) {
                    if (ioctl(fd, SIOCGIFHWADDR, &ifreq) < 0) {
                        throw IOException("get MAC address");
                    }
                    copy_n(
                        (uint8_t *) &ifreq.ifr_hwaddr.sa_data,
                        6,
                        work_station.mac_address.begin()
                    );
                    found_mac = true;
                }
            }
        }
        freeifaddrs(ifaddr_list);
    }


    return work_station;
}

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

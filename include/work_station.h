#ifndef NODE_H_
#define NODE_H_

#include <vector>
#include <shared_mutex>
#include <string>
#include <map>
#include <memory>
#include "../include/address.h"

using WorkStation = NodeAddresses;

class WorkStationTable {
    private:
        shared_mutex rw_mutex;
        map<MacAddress, shared_ptr<WorkStation>> mac_address_index;
        map<IpAddress, shared_ptr<WorkStation>> ip_address_index;
    public:
        WorkStationTable (const WorkStationTable& obj) = delete;
        WorkStationTable& operator=(const WorkStationTable& obj) = delete;

        bool insert(shared_ptr<WorkStation> node);

        shared_ptr<WorkStation> get_by_mac_address(MacAddress const &address);
        shared_ptr<WorkStation> get_by_ip_address(IpAddress const &address);

        vector<shared_ptr<WorkStation>> to_list();
};

#endif

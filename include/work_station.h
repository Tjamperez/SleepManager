#ifndef NODE_H_
#define NODE_H_

#include <vector>
#include <shared_mutex>
#include <string>
#include <map>
#include <memory>
#include "../include/address.h"

class WorkStation;

class WorkStationTable {
    private:
        shared_mutex rw_mutex;
        map<MacAddress, shared_ptr<WorkStation>> mac_address_index;
        map<IpAddress, shared_ptr<WorkStation>> ip_address_index;
    public:
        WorkStationTable (const WorkStationTable& obj) = delete;
        WorkStationTable& operator=(const WorkStationTable& obj) = delete;

        bool insert(shared_ptr<WorkStation> node);

        bool remove_by_mac_address(MacAddress const &address);
        bool remove_by_ip_address(IpAddress const &address);

        shared_ptr<WorkStation> get_by_mac_address(MacAddress const& address);
        shared_ptr<WorkStation> get_by_ip_address(IpAddress const& address);

        vector<shared_ptr<WorkStation>> to_list();
};

class WorkStation {
    friend bool WorkStationTable::remove_by_mac_address(
        MacAddress const& address
    );
    friend bool WorkStationTable::remove_by_ip_address(
        IpAddress const& address
    );

    public:
        enum Status {
            DISCONNECTED = -1,
            ASLEEP = 0,
            AWAKEN = 1
        };

    private:
        NodeAddresses addresses_;
        WorkStation::Status status_;
        shared_mutex rw_mutex;

    public:
        WorkStation(NodeAddresses addresses, WorkStation::Status status);

        NodeAddresses addresses();
        WorkStation::Status status();
};


#endif

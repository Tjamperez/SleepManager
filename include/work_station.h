#ifndef NODE_H_
#define NODE_H_

#include <vector>
#include <shared_mutex>
#include <string>
#include <map>
#include <memory>
#include "../include/address.h"

class WorkStation;

/** A concurrent table of work stations. */
class WorkStationTable {
    private:
        shared_mutex rw_mutex;
        map<MacAddress, shared_ptr<WorkStation>> mac_address_index;
        map<IpAddress, shared_ptr<WorkStation>> ip_address_index;
    public:
        WorkStationTable (const WorkStationTable& obj) = delete;
        WorkStationTable& operator=(const WorkStationTable& obj) = delete;

        /** Inserts the given work station, returning whether this is a new work
         * station.
         */
        bool insert(shared_ptr<WorkStation> node);

        /** Removes a work station given its MAC address. Returns whether the
         * element was actually present.
         */
        bool remove_by_mac_address(MacAddress const &address);

        /** Removes a work station given its IP address. Returns whether the
         * element was actually present.
         */
        bool remove_by_ip_address(IpAddress const &address);

        /** Gets a work station given its MAC address. Returns nullptr if not
         * found.
         */
        shared_ptr<WorkStation> get_by_mac_address(MacAddress const& address);

        /** Gets a work station given its IP address. Returns nullptr if not
         * found.
         */
        shared_ptr<WorkStation> get_by_ip_address(IpAddress const& address);

        /** Creates a list of work stations. */
        vector<shared_ptr<WorkStation>> to_list();
};

/** A participant work station. */
class WorkStation {
    friend bool WorkStationTable::remove_by_mac_address(
        MacAddress const& address
    );
    friend bool WorkStationTable::remove_by_ip_address(
        IpAddress const& address
    );

    public:
        /** Status of the work station in the network. */
        enum Status {
            DISCONNECTED = -1,
            ASLEEP = 0,
            AWAKE = 1
        };

    private:
        NodeAddresses addresses_;
        WorkStation::Status status_;
        shared_mutex rw_mutex;

    public:
        /** Creates a work station given its addresses and its initial status.
         */
        WorkStation(NodeAddresses addresses, WorkStation::Status status);

        /** Gets the addresses of this work station. */
        NodeAddresses addresses();

        /** Gets the status  of this work station. */
        WorkStation::Status status();
};


#endif

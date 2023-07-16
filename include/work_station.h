#ifndef NODE_H_
#define NODE_H_

#include <vector>
#include <shared_mutex>
#include <string>
#include <map>
#include <memory>
#include <functional>
#include "../include/address.h"

class WorkStation;

struct WorkStationEvent;

/** A concurrent table of work stations. */
class WorkStationTable {
    private:
        /* data */
        shared_mutex rw_data_lock;
        map<MacAddress, shared_ptr<WorkStation>> mac_address_index;
        map<IpAddress, shared_ptr<WorkStation>> ip_address_index;
        map<string, shared_ptr<WorkStation>> hostname_index;

        /* event handlers */
        mutex event_lock;
        vector<function<void(WorkStationEvent)>> event_handlers;
        
    public:
        WorkStationTable (const WorkStationTable& obj) = delete;
        WorkStationTable& operator=(const WorkStationTable& obj) = delete;

    private:
        void dispatch_event(WorkStationEvent event);
        void on_remove(shared_ptr<WorkStation> node);

    public:

        /** Registers an event handler function, fired every time a change
         * happens.
         */
        template<typename F>
        void register_event_handler(F event_handler);

        /** Inserts the given work station, returning whether this is a new work
         * station. Duplicate hostnames will not be allowed.
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

        /** Removes a work station given its hostname. Returns whether the
         * element was actually present.
         */
        bool remove_by_hostname(string const &hostname);

        /** Gets a work station given its MAC address. Returns nullptr if not
         * found.
         */
        shared_ptr<WorkStation> get_by_mac_address(MacAddress const& address);

        /** Gets a work station given its IP address. Returns nullptr if not
         * found.
         */
        shared_ptr<WorkStation> get_by_ip_address(IpAddress const& address);

        /** Gets a work station given its hostname. Returns nullptr if not
         * found.
         */
        shared_ptr<WorkStation> get_by_hostname(string const& hostname);

        /**
         * Wakes up by hostname. Returns whether it actually woke up.
         */
        bool wakeup(string const& hostname);

        /** Creates a list of work stations. */
        vector<shared_ptr<WorkStation>> to_list();
};

/** A participant work station. */
class WorkStation {
    friend WorkStationTable;

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
        shared_mutex rw_status_lock;

    public:
        /** Creates a work station given its addresses and its initial status.
         */
        WorkStation(NodeAddresses addresses, WorkStation::Status status);

        /** Gets the addresses of this work station. */
        NodeAddresses addresses();

        /** Gets the status  of this work station. */
        WorkStation::Status status();
};

/** An event related to a work station. */
struct WorkStationEvent {
    /** Event type's tag. */
    enum Type {
        INSERTION,
        REMOVAL,
        CHANGED_STATUS
    };

    /** What happned to the work station. */
    Type type;

    /** Old work station status. Only defined for CHANGED_STATUS. */
    WorkStation::Status old_status;
    /** New work station status. Only defined for CHANGED_STATUS. */
    WorkStation::Status new_status;
    /** The work station affected. Defined for every type. */
    shared_ptr<WorkStation> work_station;
};


template<typename F>
void WorkStationTable::register_event_handler(F event_handler)
{
    lock_guard<mutex> lock_guard(this->event_lock);
    this->event_handlers.push_back(event_handler);
}

#endif

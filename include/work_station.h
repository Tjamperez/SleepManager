#ifndef NODE_H_
#define NODE_H_

#include <shared_mutex>
#include <string>
#include <memory>
#include "../include/address.h"

class ManagementService;

/** A participant work station. */
class WorkStation {
    friend ManagementService;

    public:
        /** Status of the work station in the network. */
        enum Status {
            DISCONNECTED = -1,
            ASLEEP = 0,
            AWAKEN = 1
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

#endif

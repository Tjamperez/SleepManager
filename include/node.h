#ifndef NODE_H_
#define NODE_H_

#include <shared_mutex>
#include <string>
#include <map>
#include <memory>
#include "../include/address.h"

class Node {
public:
    MacAddress mac_address;
    IpAddress ip_address;
    string hostname;
};

class NodeCollection {
private:
    shared_mutex rw_mutex;
    map<MacAddress, shared_ptr<Node>> mac_address_index;
    map<IpAddress, shared_ptr<Node>> ip_address_index;
public:
    NodeCollection (const NodeCollection& obj) = delete;
    NodeCollection& operator=(const NodeCollection& obj) = delete;

    bool insert(shared_ptr<Node> node);

    shared_ptr<Node> get_by_mac_address(MacAddress const &address);
    shared_ptr<Node> get_by_ip_address(IpAddress const &address);
};

#endif

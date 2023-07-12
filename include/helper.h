#ifndef HELPER_H_
#define HELPER_H_

#include <string>
#include <list>
#include <cstdint>

using namespace std;

//Messages exhanged between processes
typedef struct __packet{
    uint16_t type; //Package type (p.ex. DATA | CMD)
    uint16_t seqn; //Sequence number
    uint16_t length; //Payload length
    uint16_t timestamp; // Data timestamp
    string _payload;
    string mac_adress;
    string ip_address;
    string hostname;
    bool is_manager;
} Packet;


#endif

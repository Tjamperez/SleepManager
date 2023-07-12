#ifndef HELPER_H_
#define HELPER_H_

#include <string>
#include <list>
#include <cstdint>

using namespace std;

//------------------  Definitions useful for multiple files ------------------//
#define AWAKEN 1
#define ASLEEP 0

typedef struct Work_Station{
    string hostname;
    string mac_address;
    string ip_address;
    int status; //AWAKEN or ASLEEP
    bool is_manager;
}Work_Station;

//list<Work_Station> participants;

//Messages exhanged between processes
typedef struct __packet{
    uint16_t type; //Package type (p.ex. DATA | CMD)
    uint16_t seqn; //Sequence number
    uint16_t length; //Payload length
    uint16_t timestamp; // Data timestamp
    string _payload;
    string mac_address;
    string ip_address;
    string hostname;
    bool is_manager;
    int status; //AWAKEN, ASLEEP or UNKNOWN
} Packet;
#endif


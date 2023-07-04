#ifndef HELPER_H_
#define HELPER_H_

#include <string>
#include <list>
#include <thread>
#include <mutex>
using namespace std;
//------------------  Definitions useful for multiple files ------------------//
#define AWAKEN 1
#define ASLEEP 0

typedef struct Work_Station{
    string hostname;
    string mac_adress;
    string ip_address;
    int status; //AWAKEN or ASLEEP
}Work_Station;

typedef struct{
    string hostname;
}Wakeup;

typedef struct{
    bool exit;
    Wakeup wakeup;
}Command;

//Messages exhanged between processes
typedef struct __packet{
    uint16_t type; //Package type (p.ex. DATA | CMD)
    uint16_t seqn; //Sequence number
    uint16_t length; //Payload length
    uint16_t timestamp; // Data timestamp
    const char* _payload; //Message information
} Packet_Struct;

class Packet
{
    protected:
        Packet_Struct packet;

    public:
        Packet(uint16_t type, uint16_t seqn, uint16_t length, uint16_t timestamp, const char* _payload)
        {
            this->packet.type = type;
            this->packet.seqn = seqn;
            this->packet.length = length;
            this->packet.timestamp = timestamp;
            this->packet._payload = _payload;
        }
};

class Service //Superclass for subservices
{
    protected:
        
    
};

#endif
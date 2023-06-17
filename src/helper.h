#ifndef HELPER_H_
#define HELPER_H_

//------------------  Definitions useful for multiple files ------------------//
#define HOSTNAME_SIZE 10
#define MAC_ADRESS_SIZE 17
#define IP_ADRESS_SIZE 15

#define AWAKEN 1
#define ASLEEP 0

typedef struct Work_Station{
    char hostname[HOSTNAME_SIZE];
    char mac_adress[MAC_ADRESS_SIZE];
    char ip_address[IP_ADRESS_SIZE];
    int status; //AWAKEN or ASLEEP
}Work_Station;

list<Work_Station> participants;

//Messages exhanged between processes
typedef struct __packet{
    uint16_t type; //Package type (p.ex. DATA | CMD)
    uint16_t seqn; //Sequence number
    uint16_t length; //Payload length
    uint16_t timestamp; // Data timestamp
    const char* _payload; //Message information
} Packet;

class Service //Superclass for subservices
{
    protected:
        
    
};

#endif
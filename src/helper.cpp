#include "../include/helper.h"
#include "../include/server.h"
#include <string>
#include <list>
#include <cstdint>
#include <map>
#include <mutex>

class Participants_Controller
{
    protected:
        map<string,Work_Station> participants; //like a dictionary: <hostname, participant> 
        mutex participants_mutex; //access control
    
    public:
        Participants_Controller()
        {
            participants.clear();
        };

        Work_Station create_participant(string hostname, string mac_adress, string ip_address, int status, bool is_manager)
        {
            Work_Station participant;
            participant.hostname = hostname;
            participant.mac_address = mac_adress;
            participant.ip_address = ip_address;
            participant.status = UNKNOWN;
            participant.is_manager = true;

            return participant;
        };

        void add_participant(string hostname, string mac_adress, string ip_address, int status, bool is_manager)
        {
            this->participants_mutex.lock();

            Work_Station participant = create_participant(hostname, mac_adress, ip_address, status, is_manager);
            this->participants[participant.hostname] = participant;
            
            this->participants_mutex.unlock();
        };

        map<string,Work_Station> get_participants()
        {
            return this->participants;
        };

        //Update status of a given participant, searching for its hostname
        void update_participant_status(string hostname, int status)
        {
            this->participants_mutex.lock();
            this->participants[hostname].status = status;
            this->participants_mutex.unlock();
        };

        //Remove a given participant from list
        void remove_participant(string hostname)
        {
            this->participants_mutex.lock();
            this->participants.erase(hostname);
            this->participants_mutex.unlock();
        };
};

// class Service
// {
//     public:
//         bool is_manager;
//         Service(bool is_manager){};
//         void start()
//         {
            
//         };

//     protected:
        
// }
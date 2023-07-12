#ifndef SERVER_H_
#define SERVER_H_

#include "../include/discovery.h"
#include "../include/helper.h"
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <map>
#include <mutex>

#define PORT 4000
#define BUFFER_SIZE 10000
#define MAX_TRIES 3
#define SLEEP_SERVICE_DISCOVERY 1 
#define SLEEP_SERVICE_BROADCAST 2
#define SLEEP_SERVICE_EXIT 3
#define SLEEP_STATUS_REQUEST 4
#define FIND_MANAGER "FIND_MANAGER"
#define FIND_PARTICIPANTS "FIND_PARTICIPANTS"
#define MANAGER_FOUND "MANAGER_FOUND"
#define PARTICIPANT_FOUND "PARTICIPANT_FOUND"
#define SEND_STATUS_UPDATE "SEND_STATUS_UPDATE"
#define SENDING_STATUS_UPDATE "SENDING_STATUS_UPDATE"
#define EXIT "EXIT"
#define WAKEUP "WAKEUP"
#define UNKNOWN 0
#define NO_CHANGE -1

class Server_Connection
{
    public:
        Server_Connection(bool is_manager);
        void start();
        void start_client(uint16_t packet_type,string _payload);
        void start_server(uint16_t packet_type,string _payload);

    protected:
        //clock_t start;//começo do programa
        struct sockaddr_in serv_addr, from;        
        struct hostent *server;
        uint16_t seqn; //Sequence number used in packets
        string hostname;
        Work_Station work_station; //work station where program is running
        map<string,Work_Station> participants; //like a dictionary: <hostname, participant>
        mutex participants_mutex; //access control
        bool is_manager;
        bool active;
        uint16_t status_update;
        bool socket_is_bound;
        bool socket_options_set;
        int bound_socket;
        int server_socket;
        int socket_option;

        bool hasTimeoutPassed( int timeoutInMicroseconds);
        Packet create_packet(uint16_t type, string _payload);
        string packet_to_string(Packet packet);
        Packet string_to_packet(string packet_str);
        char* get_mac();
        char* get_ip();
        Work_Station process_message(char packet_string[BUFFER_SIZE]);
        void add_participant(Work_Station participant);
        Work_Station create_participant(string hostname, string mac_adress, string ip_address, int status, bool is_manager);
        void discovery();
        void monitoring();
        void interface();
        void terminal_cmds();
        void show_participants();
        int get_bound_socket(struct sockaddr_in serv_addr);
        int get_socket();
        void set_socket_option();
};

#endif

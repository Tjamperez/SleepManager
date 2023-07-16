#ifndef INTERFACE_H_
#define INTERFACE_H_

#include <iostream>
#include <list>
#include <string>
#include <vector>
#include <algorithm>
#include <memory>
#include "../include/channel.h"
#include "../include/work_station.h"

#define EXIT "EXIT"
#define WAKEUP "WAKEUP"
using namespace std;

enum InterfaceMessage {
    INTERFACE_EXIT,
    INTERFACE_REFRESH
};

/*
void interface_main(
    shared_ptr<WorkStationTable> participants,
    Broadcast<InterfaceMessage> channel);

class Interface {
    protected:
        bool is_participant;
        shared_ptr<WorkStationTable> participants;

        void show_participants();
        void show_manager();
  
    public:
        Interface(
            bool is_participant,
            shared_ptr<WorkStationTable> participants
        );
        void terminal();
};
    */

#endif

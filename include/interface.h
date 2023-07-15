#ifndef INTERFACE_H_
#define INTERFACE_H_

#include <iostream>
#include <list>
#include <string>
#include <vector>
#include <algorithm>
#include <memory>
#include "../include/work_station.h"

#define EXIT "EXIT"
#define WAKEUP "WAKEUP"
using namespace std;

void interface_main(shared_ptr<WorkStationTable> participants);

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

#endif

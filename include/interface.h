#ifndef INTERFACE_H_
#define INTERFACE_H_

#include <iostream>
#include <list>
#include <string>
#include <vector>
#include <algorithm>
#include "../include/helper.h"

#define EXIT "EXIT"
#define WAKEUP "WAKEUP"
using namespace std;

class Interface{
    protected:
        bool is_participant;
        list<Work_Station> participants;

        void show_participants();
        void show_manager();
  
    public:
        Interface(bool is_participant, list<Work_Station> participants);
        void terminal();
};

#endif
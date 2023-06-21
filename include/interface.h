#ifndef INTERFACE_H_
#define INTERFACE_H_

#include <iostream>
#include <list>
#include <thread>
#include <future>
#include <string>
#include "helper.h"
using namespace std;

class Interface{

    public:
        Interface(){};
  
        void get_terminal_input(promise<string>* promise_terminal_input)
        {
            string terminal_input;
            getline(cin,terminal_input);
            promise_terminal_input->set_value(terminal_input);
        }
  
        void show_participants(promise<list<Work_Station>> participants)
        {
            /*
             * TODO: implement this function
             */
        };
};

int interface_main();

#endif
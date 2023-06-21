#include "../include/interface.h"

int interface_main()
{
    Interface interface;
    
    //promise and future are used to retrieve values from functions/methods executed in threads
    promise<string> promise_terminal_input;
    future<string> future_terminal_input = promise_terminal_input.get_future();
  
    //syntax for creating threads for class methods:
    //thread <thread_name> (&<class::method>,<class_instance>,<promise_object>)
    thread terminal_input(&Interface::get_terminal_input, interface, &promise_terminal_input);
  
    //retrieving value from get_terminal_input after terminal_input thread has been executed
    cout<<future_terminal_input.get()<<endl;
  
    //thread show_participants(&Interface::show_participants, interface, participants);
    
    //safely terminate threads
    terminal_input.detach();
    //show_participants.detach();

    return 0;
}
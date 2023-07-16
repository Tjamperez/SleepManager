#include "../include/interface.h"
#include <thread>

/*
void interface_main(
    shared_ptr<WorkStationTable> participants,
    Broadcast<InterfaceMessage> channel)
{
    bool exit = false;
    Broadcast<InterfaceMessage>::Subscriber subscriber
        = channel.make_subscriber();

    thread input_thread([& channel] () {
        Broadcast<InterfaceMessage>::Publisher publisher
            = channel.make_publisher();
        bool exit = false;
        while (!exit) {
            string input;
            getline(cin, input);

            transform(input.begin(), input.end(), input.begin(), ::toupper);

            if (input == "EXIT") {
                publisher.publish(INTERFACE_EXIT);
                exit = true;
            }
        }
    });
    input_thread.detach();

    while (!exit) {
        switch (subscriber.subscribe()) {
            case INTERFACE_EXIT:
                exit = true;
                break;
            case INTERFACE_REFRESH:
                cout << "\033[2J\033[1;1H";
                break;
        }
    }
}

Interface::Interface(
    bool is_participant,
    shared_ptr<WorkStationTable> participants)
{
    this->is_participant = is_participant;
    this->participants = participants;
};

//Get terminal input and validate it according to station (participant or manager)
void Interface::terminal()
{
    bool valid_input = false;
    
    while (valid_input == false)
    {
        vector<string> input_vec;
        string terminal_input;

        getline(cin,terminal_input);
        transform(terminal_input.begin(), terminal_input.end(), terminal_input.begin(), ::toupper);

        int space_position = terminal_input.find(" ");
        string cmd = terminal_input.substr(0, space_position);
        string cmd_argument = terminal_input.substr(space_position+1);

        if (this->is_participant)
        {
            if (cmd == EXIT)
                valid_input = true;
            else
                cout << "The only valid command for participant station is 'EXIT'" << endl;
        }
        else
        {
            if (cmd == WAKEUP && cmd_argument != cmd) //if no hostname is informed, no argument will have been found in the terminal command and substrings will be equal
            {
                string hostname = cmd_argument;

                //wakeup(hostname,&this->participants); //Not a real method, just writing out program flow
                show_participants(); //Show updated participants list

                valid_input = true;
            }
            else
                cout << "The only valid command for manager station is 'WAKEUP <hostname>'" << endl;
        };
    };

};

void Interface::show_participants()
{
};

void Interface::show_manager()
{
};
*/

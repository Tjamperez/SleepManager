#include "../include/interface.h"
#include <thread>
#include <algorithm>

enum InterfaceMsg {
    INTERFACE_EXIT,
    INTERFACE_REFRESH
};

static void trim_whitespace(string &input);

static void render(vector<shared_ptr<WorkStation>> participants);

void interface_main(
    InterfaceType type,
    shared_ptr<WorkStationTable> participants)
{
    auto channel = Mpsc<InterfaceMsg>::open();
    Mpsc<InterfaceMsg>::Sender sender = get<0>(channel);
    Mpsc<InterfaceMsg>::Receiver receiver = move(get<1>(channel));

    participants->register_event_handler([& sender] (WorkStationEvent event) {
        sender.send(INTERFACE_REFRESH);
    });

    thread input_thread([sender = move(sender), participants, type] () {
        bool exit = false;
        while (!exit) {
            string input;
            getline(cin, input);

            trim_whitespace(input);

            size_t whitespace_pos = input.find(' ');
            string command = input.substr(0, whitespace_pos);
            transform(
                command.begin(),
                command.end(),
                command.begin(),
                ::toupper);
            string argument = input.substr(whitespace_pos);
            trim_whitespace(argument);

            if (input == "EXIT") {
                if (argument.size() > 0) {
                    cerr << "EXIT does not expect an argument." << endl;
                }
                sender.send(INTERFACE_EXIT);
                exit = true;
            } else if (input == "WAKEUP") {
                if (type == INTERFACE_MANAGER) {
                    cerr << "WAKEUP cannot be used by managers." << endl;
                } else if (argument.size() == 0) {
                    cerr << "WAKEUP expects a hostname." << endl;
                } else {
                    bool woke_up = participants->wakeup(argument);
                    if (!woke_up) {
                        cerr << "could not wake up " << argument << endl;
                    }
                }
            }
        }
    });
    input_thread.detach();

    render(participants->to_list());

    bool exit = false;
    while (!exit) {
        auto maybe_message = receiver.receive();
        if (bool(maybe_message)) {
            switch (maybe_message.value()) {
                case INTERFACE_EXIT:
                    exit = true;
                    break;
                case INTERFACE_REFRESH:
                    render(participants->to_list());
                    break;
            }
        } else {
            exit = true;
        }
    }
}

static void render(vector<shared_ptr<WorkStation>> participants)
{
    sort(
        participants.begin(),
        participants.end(),
        [] (shared_ptr<WorkStation> const& a, shared_ptr<WorkStation> const& b)
        {
            return a->addresses().ip < b->addresses().ip;
        }
    );

    unsigned row_size = 80;
    unsigned mac_col_size = 6 * 2 + 5;
    unsigned ip_col_size = 4 * 3 + 3;
    unsigned status_col_size = 6;
    unsigned hostname_col_size
        = row_size - mac_col_size - ip_col_size - status_col_size;

    string mac_header = "MAC Address";
    string ip_header = "IP Address";
    string status_header = "Status";
    string hostname_header = "Hostname";

    cout << "\033[2J\033[1;1H";
    cout << mac_header;
    for (size_t i = mac_header.size(); i < mac_col_size; i++) {
        cout << " ";
    }
    cout << " | ";
    cout << ip_header;
    for (size_t i = ip_header.size(); i < ip_col_size; i++) {
        cout << " ";
    }
    cout << " | ";
    cout << status_header;
    for (size_t i = status_header.size(); i < status_col_size; i++) {
        cout << " ";
    }
    cout << " | ";
    cout << hostname_header;
    for (size_t i = hostname_header.size(); i < hostname_col_size; i++) {
        cout << " ";
    }
    cout << endl;
    for (size_t i = 0; i < row_size; i++) {
        cout << "-";
    }
    cout << endl;

    for (auto participant : participants) {
        string column = render_mac_address(participant->addresses().mac);
        cout << column;
        for (size_t i = column.size(); i < mac_col_size; i++) {
            cout << " ";
        }
        cout << " | ";
        column = render_ip_address(participant->addresses().ip);
        cout << column;
        for (size_t i = column.size(); i < ip_col_size; i++) {
            cout << " ";
        }
        cout << " | ";
        switch (participant->status()) {
            case WorkStation::ASLEEP:
                column = "ASLEEP";
                break;
            case WorkStation::AWAKEN:
                column = "AWAKEN";
                break;
        }
        cout << column;
        for (size_t i = column.size(); i < status_col_size; i++) {
            cout << " ";
        }
        cout << " | ";
        column = participant->addresses().hostname;
        column.resize(column.size(), hostname_col_size);
        cout << column;
        for (size_t i = column.size(); i < hostname_col_size; i++) {
            cout << " ";
        }
        cout << endl;
    }
}

static void trim_whitespace(string &input)
{
    input.erase(input.begin(), find_if(input.begin(), input.end(), [](char ch) {
        return !iswspace(ch);
    }));
    input.erase(
        find_if(input.rbegin(), input.rend(), [](char ch) {
            return !iswspace(ch);
        }).base(),
        input.end()
    );
}

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

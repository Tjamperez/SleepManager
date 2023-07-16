#include "../include/interface.h"
#include <thread>
#include <algorithm>

enum ManagerIfaceMsg {
    MANAGER_IFACE_EXIT,
    MANAGER_IFACE_REFRESH
};

static void trim_whitespace(string &input);

static void render(vector<shared_ptr<WorkStation>> participants);

static void handle_manager_input(
    Mpsc<ManagerIfaceMsg>::Sender sender,
    shared_ptr<WorkStationTable> participants
);

void participant_interface_main(Mpsc<ParticipantMsg>::Sender channel)
{
    bool exit = false;
    while (!exit) {
        string input;
        getline(cin, input);

        trim_whitespace(input);
        transform(input.begin(), input.end(), input.begin(), ::toupper);

        if (input == "EXIT") {
            channel.send(PARTICIPANT_EXIT);
            exit = true;
        } else {
            cerr << "Unknown command." << endl;
        }
    }
}

void manager_interface_main(shared_ptr<WorkStationTable> participants)
{
    auto channel = Mpsc<ManagerIfaceMsg>::open();
    Mpsc<ManagerIfaceMsg>::Sender sender = get<0>(channel);
    Mpsc<ManagerIfaceMsg>::Receiver receiver = move(get<1>(channel));

    participants->register_event_handler([& sender] (WorkStationEvent event) {
        sender.send(MANAGER_IFACE_REFRESH);
    });

    thread input_thread(handle_manager_input, move(sender), participants);
    input_thread.detach();

    render(participants->to_list());

    bool exit = false;
    while (!exit) {
        auto maybe_message = receiver.receive();
        if (bool(maybe_message)) {
            switch (maybe_message.value()) {
                case MANAGER_IFACE_EXIT:
                    exit = true;
                    break;
                case MANAGER_IFACE_REFRESH:
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

static void handle_manager_input(
    Mpsc<ManagerIfaceMsg>::Sender sender,
    shared_ptr<WorkStationTable> participants)
{
    bool exit = false;
    while (!exit) {
        string input;
        if (getline(cin, input)) {
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

            if (command == "WAKEUP") {
                if (argument.size() == 0) {
                    cerr << "WAKEUP expects a hostname." << endl;
                } else {
                    bool woke_up = participants->wakeup(argument);
                    if (!woke_up) {
                        cerr << "could not wake up " << argument << endl;
                    }
                }
            } else {
                cerr << "Unknown command." << endl;
            }
        } else {
            exit = true;
        }
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

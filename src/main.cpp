#include <stdio.h>
#include <string.h>
#include <string>
#include <cctype>
#include <list>
#include <cstdint>
#include <iostream>
#include "../include/participant.h"
#include "../include/manager.h"
//#include "../include/discovery.h"
//#include "../include/management.h"
//#include "../include/monitoring.h"
//#include "../include/interface.h"
//#include "../include/helper.h"
#include "../include/address.h"

using namespace std;

struct Arguments {
    enum NodeType {
        PARTICIPANT,
        ḾANAGER
    };

    string interface;
    NodeType node_type;

    int parse(int argc, char const *argv[]);
};

void print_help();

int main(int argc, char const *argv[])
{
    Arguments arguments;
    arguments.parse(argc, argv);

    set_global_interface_name(arguments.interface);

    switch (arguments.node_type) {
        case Arguments::PARTICIPANT:
            cout << "participant" << endl;
            participant_main();
            break;
        case Arguments::ḾANAGER:
            cout << "manager" << endl;
            manager_main();
            break;
    }

    return 0;
}

int Arguments::parse(int argc, char const *argv[])
{
    this->interface = "eth0";
    this->node_type = Arguments::PARTICIPANT;

    if (argv[0] != nullptr) {
        size_t i = 1;

        while (argv[i] != nullptr) {
            if (strcmp(argv[i], "manager") == 0) {
                this->node_type = Arguments::ḾANAGER;
            } else if (
                strcmp(argv[i], "--interface") == 0
                || strcmp(argv[i], "-i") == 0
            ) {
                i++;
                if (argv[i] == nullptr) {
                    cerr << "missing interface name argument" << endl;
                    print_help();
                    exit(1);
                }
                this->interface = argv[i];
            } else if (
                strcmp(argv[i], "--help") == 0
                || strcmp(argv[i], "-h") == 0
                || strcmp(argv[i], "help") == 0
            ) {
                print_help();
                exit(1);
            } else {
                cerr << "unknown argument " << argv[i] << endl;
                print_help();
                exit(1);
            }
            i++;
        }
    }

    return 0;
}

void print_help()
{
    cerr << "Usage: ./sleep_server [OPTIONS] [manager]" << endl;
    cerr << "Arguments:" << endl;
    cerr << "    manager                    -- starts a manager node" << endl;
    cerr << "    -i, --interface <name>     -- uses interface of <name>" << endl;
}

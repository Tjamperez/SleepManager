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

int main(int argc, char const *argv[])
{
    int exit_code = 0;

    NodeAddresses addrs = NodeAddresses::load_host();
    cout << addrs.mac << endl;
    
    return 0;

    if (argc <= 1) {
        cout << "participant" << endl;
        participant_main();
    } else if (argc > 2) {
        cerr << "program expects at most one additional argument" << endl;
        exit_code = -1;
    } else if (strcmp(argv[1], "manager") != 0) {
        cerr << "if additional argument given, it must be \"maanger\"" << endl;
        exit_code = -1;
    } else {
        cout << "manager" << endl;
        manager_main();
    }

    return exit_code;
}

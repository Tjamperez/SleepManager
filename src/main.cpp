#include <stdio.h>
#include <string.h>
#include <string>
#include <cctype>
#include <list>
#include <cstdint>
#include <iostream>
#include <cstdint>
#include "../include/participant.h"
#include "../include/manager.h"
#include "../include/server.h"

using namespace std;

int main(int argc, char const *argv[])
{
    int exit_code = 0;

    if (argc <= 1) {
        cout << "participant" << endl;
        //participant_main();
        Server_Connection server(false);
        server.start();
    } else if (argc > 2) {
        fputs("program expects at most one additional argument\n", stderr);
        exit_code = -1;
    } else if (strcmp(argv[1], "manager") != 0) {
        fputs("if additional argument given, it must be \"maanger\"\n", stderr);
        exit_code = -1;
    } else {
        cout << "manager" << endl;
        Server_Connection server(true);
        server.start();
        //manager_main();
    }

    return exit_code;
}

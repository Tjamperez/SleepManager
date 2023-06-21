#include <stdio.h>
#include <string.h>
#include <string>
#include <cctype>
#include <list>
#include <cstdint>
#include "../include/participant.h"
#include "../include/manager.h"
#include "../include/discovery.h"
#include "../include/management.h"
#include "../include/monitoring.h"
#include "../include/interface.h"
#include "../include/helper.h"

using namespace std;

int main(int argc, char const *argv[])
{
    int exit_code = 0;

    if (argc <= 1) {
        participant_main();
    } else if (argc > 2) {
        fputs("program expects at most one additional argument\n", stderr);
        exit_code = -1;
    } else if (strcmp(argv[1], "manager") != 0) {
        fputs("if additional argument given, it must be \"maanger\"\n", stderr);
        exit_code = -1;
    } else {
        manager_main();
    }

    return exit_code;
}

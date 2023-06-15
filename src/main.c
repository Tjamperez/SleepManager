#include <stdio.h>
#include <string.h>
#include "participant.h"
#include "manager.h"

int main(int argc, char const *argv[])
{
    int exit_code = 0;

    if (argc <= 1) {
        participant_main();
    } else if (argc > 2) {
        fputs("program expects at most one additional argument\n", stderr);
        exit_code = -1;
    } else if (strcasecmp(argv[1], "manager") != 0) {
        fputs("if additional argument given, it must be \"maanger\"\n", stderr);
        exit_code = -1;
    } else {
        manager_main();
    }

    return exit_code;
}

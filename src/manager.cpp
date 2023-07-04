#include "../include/manager.h"
#include "../include/interface.h"
#include "../include/discovery.h"

void manager_main(void)
{
    discovery_main();
    list<Work_Station> participants; //Adding this just to test interface. Discovery subservice will eiter return a list of participants or set a global variable

    bool is_participant = false;
    Interface interface(is_participant,participants);
    
    interface.terminal();
}

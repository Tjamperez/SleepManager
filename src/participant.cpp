#include "../include/participant.h"
#include "../include/interface.h"
#include "../include/discovery.h"

void participant_main(void)
{
    discovery_main();
    list<Work_Station> participants; //Adding this just to test interface. Discovery subservice will either return a list of participants or set a global variable

    bool is_participant = true;
    Interface interface(is_participant,participants);
  
    interface.terminal();
}
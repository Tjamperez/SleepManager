#include "../include/work_station.h"
#include "../include/manager.h"
#include "../include/interface.h"

void manager_main(void)
{
    Server_Connection server;
    server.start_server();

    shared_ptr<WorkStationTable> participants; //Adding this just to test interface. Discovery subservice will eiter return a list of participants or set a global variable

    bool is_participant = false;
    Interface interface(is_participant,participants);
    
    interface.terminal();
}

#ifndef INTERFACE_H_
#define INTERFACE_H_

#include <iostream>
#include <list>
#include <string>
#include <vector>
#include <algorithm>
#include <memory>
#include "../include/channel.h"
#include "../include/management.h"
#include "../include/participant.h"

using namespace std;

void participant_interface_main(Mpsc<ParticipantMsg>::Sender channel);

void manager_interface_main(shared_ptr<ManagementService> management_service);

#endif

#ifndef PARTICIPANT_H_
#define PARTICIPANT_H_

#include <string>

using namespace std;

enum ParticipantMsg {
    PARTICIPANT_WAKEUP,
    PARTICIPANT_MANG_EXITED,
    PARTICIPANT_EXIT
};

void participant_main();

#endif

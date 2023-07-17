#ifndef PARTICIPANT_H_
#define PARTICIPANT_H_

enum ParticipantMsg {
    PARTICIPANT_WAKEUP,
    PARTICIPANT_EXIT
};

void participant_main(void);

#endif

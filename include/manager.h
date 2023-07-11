#ifndef MANAGER_H_
#define MANAGER_H_

#include <mutex>
#include <vector>
#include <unordered_set>

void manager_main(void);

class Manager {
private:
    mutex participants_mutex;
    unordered_set<uint64_t> participant_macs;
    vector<Participant> participants;
};

#endif

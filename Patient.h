// Patient.h - record stored in the hash table and referenced by the heap
#pragma once
#include <string>

struct Patient {
    int         id;         // unique key (hash table key)
    std::string name;
    int         age;        // 0..120
    int         severity;   // 1 (minor) .. 5 (critical)
    std::string complaint;
    long        arrival;    // arrival sequence number (tie-breaker: smaller = earlier)
    bool        waiting;    // true = in the triage queue, false = already treated
};

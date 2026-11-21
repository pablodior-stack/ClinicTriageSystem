// TriageSystem.h - University Clinic Triage System
// Hash table (std::unordered_map) = patient records, O(1) lookup by ID
// TriageHeap (own binary heap)    = who is served next
#pragma once
#include <string>
#include <unordered_map>
#include "Patient.h"
#include "TriageHeap.h"

class TriageSystem {
    std::unordered_map<int, Patient*> records;   // owns every Patient (heap only borrows)
    TriageHeap queue;
    long nextArrival = 1;

public:
    TriageSystem() = default;
    ~TriageSystem();                              // frees every Patient
    TriageSystem(const TriageSystem&)            = delete;
    TriageSystem& operator=(const TriageSystem&) = delete;

    // ---- basic record operations ----
    bool     addPatient(int id, const std::string& name, int age, int priority,
                        const std::string& complaint);
    Patient* findPatient(int id) const;
    bool     updatePatient(int id, int newpriority, const std::string& newComplaint);
    bool     deletePatient(int id);               // <-- individual component
    // ---- scenario-specific operations ----
    Patient* treatNext();                         // serve highest priority patient
    const Patient* peekNext() const;
    void     rebuildQueue();                      // <-- individual component
    // ---- reports ----
    void displayQueue() const;                    // ordered by priority
    void displayAllById() const;                  // ordered by patient ID
    // ---- file I/O ----
    bool loadFromFile(const std::string& path);
    bool saveToFile(const std::string& path) const;
    // ---- helpers for tests / menu ----
    size_t recordCount() const { return records.size(); }
    size_t queueSize()   const { return queue.size(); }
};

// TriageSystem.cpp
#include "TriageSystem.h"
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

static const char* SEV_NAME[] = {"", "Minor", "Low", "Moderate", "Urgent", "Critical"};

TriageSystem::~TriageSystem() {
    for (auto& kv : records) delete kv.second;
    records.clear();
}

// ------------------------------------------------------------------ add / find / update
bool TriageSystem::addPatient(int id, const std::string& name, int age, int priority,
                              const std::string& complaint) {
    if (id <= 0 || priority < 1 || priority > 5 || records.count(id)) return false;
    Patient* p = new Patient{id, name, age, priority, complaint, nextArrival++, true};
    records[id] = p;
    queue.push(p);
    return true;
}

Patient* TriageSystem::findPatient(int id) const {
    auto it = records.find(id);
    return it == records.end() ? nullptr : it->second;
}

bool TriageSystem::updatePatient(int id, int newpriority, const std::string& newComplaint) {
    Patient* p = findPatient(id);
    if (!p || newpriority < 1 || newpriority > 5) return false;
    p->priority = newpriority;
    if (!newComplaint.empty()) p->complaint = newComplaint;
    if (p->waiting) queue.update(id);             // restore heap order
    return true;
}

// ================================================================ INDIVIDUAL COMPONENT
// deletePatient: remove a patient's record permanently.
//  1. O(1) average hash lookup of the record
//  2. if still waiting, remove from the heap in O(log n) using the position map
//     (MUST happen before delete, because the heap reads p->id)
//  3. erase the hash-table entry and free the memory
// Time: O(log n) if waiting, O(1) average if already treated.   Space: O(1).
bool TriageSystem::deletePatient(int id) {
    auto it = records.find(id);
    if (it == records.end()) return false;        // unknown ID (also empty system)
    Patient* p = it->second;
    if (p->waiting) queue.remove(id);
    records.erase(it);
    delete p;
    return true;
}

// rebuildQueue: discard the heap and rebuild it from the records of patients
// that are still waiting, using bottom-up heapify (Floyd).
//  - used after loading a file and as a repair/consistency operation
//  - O(n) heapify is cheaper than n inserts which cost O(n log n)
// Time: O(n) over the n records.   Space: O(w) for the temporary vector of w waiting patients.
void TriageSystem::rebuildQueue() {
    std::vector<Patient*> waiting;
    waiting.reserve(records.size());
    for (auto& kv : records)
        if (kv.second->waiting) waiting.push_back(kv.second);
    queue.buildFrom(std::move(waiting));
}
// ================================================================ END INDIVIDUAL COMPONENT

// ------------------------------------------------------------------ scenario operations
Patient* TriageSystem::treatNext() {
    Patient* p = queue.pop();
    if (p) p->waiting = false;                    // record is kept for history
    return p;
}
const Patient* TriageSystem::peekNext() const { return queue.top(); }

// ------------------------------------------------------------------ reports
static void header() {
    std::cout << std::left << std::setw(8) << "ID" << std::setw(22) << "Name"
              << std::setw(5) << "Age" << std::setw(11) << "priority"
              << std::setw(10) << "Status" << "Complaint\n"
              << std::string(70, '-') << "\n";
}
static void row(const Patient* p) {
    std::cout << std::left << std::setw(8) << p->id << std::setw(22) << p->name
              << std::setw(5) << p->age
              << std::setw(11) << (std::to_string(p->priority) + "-" + SEV_NAME[p->priority])
              << std::setw(10) << (p->waiting ? "Waiting" : "Treated")
              << p->complaint << "\n";
}

void TriageSystem::displayQueue() const {
    if (queue.empty()) { std::cout << "The waiting queue is empty.\n"; return; }
    std::cout << "\n=== WAITING QUEUE (serving order) ===\n";
    header();
    for (const Patient* p : queue.orderedSnapshot()) row(p);
}

void TriageSystem::displayAllById() const {
    if (records.empty()) { std::cout << "No patient records.\n"; return; }
    std::vector<const Patient*> v;
    for (auto& kv : records) v.push_back(kv.second);
    std::sort(v.begin(), v.end(), [](const Patient* x, const Patient* y) { return x->id < y->id; });
    std::cout << "\n=== ALL RECORDS (by patient ID) ===\n";
    header();
    for (const Patient* p : v) row(p);
}

// ------------------------------------------------------------------ file I/O
// Format, one patient per line:  id|name|age|severity|complaint|arrival|W-or-T
bool TriageSystem::loadFromFile(const std::string& path) {
    std::ifstream in(path);
    if (!in) return false;
    for (auto& kv : records) delete kv.second;
    records.clear();
    queue.clear();
    nextArrival = 1;

    std::string line;
    int lineNo = 0, skipped = 0;
    while (std::getline(in, line)) {
        ++lineNo;
        if (line.empty()) continue;
        std::vector<std::string> f;
        std::stringstream ss(line);
        std::string tok;
        while (std::getline(ss, tok, '|')) f.push_back(tok);
        if (!line.empty() && line.back() == '|') f.push_back("");
        bool ok = (f.size() == 7);
        int id = 0, age = 0, sev = 0; long arr = 0;
        if (ok) {
            try {
                id = std::stoi(f[0]); age = std::stoi(f[2]);
                sev = std::stoi(f[3]); arr = std::stol(f[5]);
            } catch (...) { ok = false; }
        }
        if (ok) ok = id > 0 && age >= 0 && age <= 120 && sev >= 1 && sev <= 5 && arr > 0
                     && (f[6] == "W" || f[6] == "T") && !f[1].empty() && !records.count(id);
        if (!ok) { ++skipped; std::cerr << "Skipped bad line " << lineNo << "\n"; continue; }
        records[id] = new Patient{id, f[1], age, sev, f[4], arr, f[6] == "W"};
        nextArrival = std::max(nextArrival, arr + 1);
    }
    rebuildQueue();                               // heap is rebuilt from the loaded records
    if (skipped) std::cerr << skipped << " invalid line(s) ignored.\n";
    return true;
}

bool TriageSystem::saveToFile(const std::string& path) const {
    std::ofstream out(path);
    if (!out) return false;
    std::vector<const Patient*> v;
    for (auto& kv : records) v.push_back(kv.second);
    std::sort(v.begin(), v.end(), [](const Patient* x, const Patient* y) { return x->arrival < y->arrival; });
    for (const Patient* p : v)
        out << p->id << '|' << p->name << '|' << p->age << '|' << p->priority << '|'
            << p->complaint << '|' << p->arrival << '|' << (p->waiting ? 'W' : 'T') << '\n';
    return static_cast<bool>(out);
}

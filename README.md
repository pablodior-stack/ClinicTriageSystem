# University Clinic Triage System (BCS 2204 - Group Coursework 2, Scenario 2)

Priority-queue triage: the most critical patient is always served next.

* `TriageHeap.h` - binary max-heap with a position map (O(log n) remove/update, O(n) rebuild).
* **Hash table:** `std::unordered_map<int, Patient*>` - O(1) average patient lookup by ID.

## Build and run
    g++ -std=c++17 -Wall -Wextra -o triage main.cpp TriageSystem.cpp
    ./triage                       # reads/writes patients.txt in the current folder

## Run the tests
    g++ -std=c++17 -Wall -Wextra -o tests tests.cpp TriageSystem.cpp && ./tests

## Data file format (`patients.txt`)
`id|name|age|severity(1-5)|complaint|arrival|W(waiting) or T(treated)`

## Files
| File | Purpose |
|---|---|
| Patient.h | Patient record |
| TriageHeap.h | Own binary heap (priority queue) |
| TriageSystem.h/.cpp | Records, queue, reports, file I/O, `deletePatient`, `rebuildQueue` |
| main.cpp | Menu and input validation |
| tests.cpp | Boundary tests |
| patients.txt | Sample data |

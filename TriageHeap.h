// TriageHeap.h - hand-written binary MAX-heap (priority queue) of Patient*.
// Higher severity = served first; equal severity -> earlier arrival first (FIFO).
// A position map (id -> index in the array) lets us remove / re-prioritise ANY
// patient in O(log n) instead of O(n).
#pragma once
#include <vector>
#include <unordered_map>
#include <utility>
#include <cstddef>
#include "Patient.h"

class TriageHeap {
    std::vector<Patient*>              a;    // 0-based array representation of the heap
    std::unordered_map<int, size_t>    pos;  // patient id -> index in a

    // true if x must be served BEFORE y
    static bool higher(const Patient* x, const Patient* y) {
        if (x->severity != y->severity) return x->severity > y->severity;
        return x->arrival < y->arrival;
    }
    void place(size_t i, Patient* p) { a[i] = p; pos[p->id] = i; }

    void siftUp(size_t i) {
        Patient* p = a[i];
        while (i > 0) {
            size_t parent = (i - 1) / 2;
            if (!higher(p, a[parent])) break;
            place(i, a[parent]);
            i = parent;
        }
        place(i, p);
    }
    void siftDown(size_t i) {
        Patient* p = a[i];
        const size_t n = a.size();
        while (true) {
            size_t l = 2 * i + 1;
            if (l >= n) break;
            size_t best = l, r = l + 1;
            if (r < n && higher(a[r], a[l])) best = r;
            if (!higher(a[best], p)) break;
            place(i, a[best]);
            i = best;
        }
        place(i, p);
    }
    void removeAt(size_t i) {
        Patient* removed = a[i];
        Patient* last    = a.back();
        a.pop_back();
        pos.erase(removed->id);
        if (i < a.size()) {            // removed element was not the last one
            place(i, last);
            siftUp(i);
            siftDown(pos[last->id]);
        }
    }

public:
    bool   empty() const { return a.empty(); }
    size_t size()  const { return a.size(); }
    bool   contains(int id) const { return pos.count(id) > 0; }
    const Patient* top() const { return a.empty() ? nullptr : a[0]; }

    void push(Patient* p) {                    // O(log n)
        a.push_back(p);
        siftUp(a.size() - 1);
    }
    Patient* pop() {                           // O(log n)
        if (a.empty()) return nullptr;
        Patient* t = a[0];
        removeAt(0);
        return t;
    }
    bool remove(int id) {                      // O(log n)
        auto it = pos.find(id);
        if (it == pos.end()) return false;
        removeAt(it->second);
        return true;
    }
    void update(int id) {                      // O(log n) - call after severity changed
        auto it = pos.find(id);
        if (it == pos.end()) return;
        siftUp(it->second);
        siftDown(pos[id]);
    }
    void buildFrom(std::vector<Patient*> items) {   // Floyd's bottom-up heapify, O(n)
        a = std::move(items);
        pos.clear();
        for (size_t i = 0; i < a.size(); ++i) pos[a[i]->id] = i;
        for (size_t i = a.size() / 2; i-- > 0; ) siftDown(i);
    }
    void clear() { a.clear(); pos.clear(); }

    // Patients in serving order, without changing the real heap. O(n log n)
    std::vector<Patient*> orderedSnapshot() const {
        TriageHeap copy = *this;               // copies pointers only, not patients
        std::vector<Patient*> out;
        out.reserve(a.size());
        while (!copy.empty()) out.push_back(copy.pop());
        return out;
    }
};

// main.cpp - menu-driven University Clinic Triage System
#include <cerrno>
#include <climits>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include "TriageSystem.h"

static const char* DATA_FILE = "patients.txt";

static bool parseInt(const std::string& s, int& out) {
    size_t b = s.find_first_not_of(" \t");
    if (b == std::string::npos) return false;
    size_t e = s.find_last_not_of(" \t");
    std::string t = s.substr(b, e - b + 1);
    char* end = nullptr;
    errno = 0;
    long v = std::strtol(t.c_str(), &end, 10);
    if (errno != 0 || *end != '\0' || v < INT_MIN || v > INT_MAX) return false;
    out = static_cast<int>(v);
    return true;
}
static std::string readLine(const std::string& prompt) {
    std::cout << prompt;
    std::string s;
    if (!std::getline(std::cin, s)) throw std::runtime_error("input closed");
    return s;
}
// VALIDATION 1: integers must be numeric and inside [lo, hi]
static int readInt(const std::string& prompt, int lo, int hi) {
    while (true) {
        int v;
        if (parseInt(readLine(prompt), v) && v >= lo && v <= hi) return v;
        std::cout << "  Invalid input. Enter a whole number from " << lo << " to " << hi << ".\n";
    }
}
// VALIDATION 2: text must be non-empty (unless allowEmpty), <= 60 chars, and contain no '|'
static std::string readText(const std::string& prompt, bool allowEmpty = false) {
    while (true) {
        std::string s = readLine(prompt);
        bool bad = s.find('|') != std::string::npos || s.size() > 60 || (!allowEmpty && s.find_first_not_of(" \t") == std::string::npos);
        if (!bad) return s;
        std::cout << "  Invalid text (cannot be blank, longer than 60 chars, or contain '|').\n";
    }
}
static void menu() {
    std::cout << "\n===== UNIVERSITY CLINIC TRIAGE SYSTEM =====\n"
                 " 1. Register patient\n 2. Search patient\n 3. Update severity\n"
                 " 4. Delete patient\n 5. Treat next patient\n 6. View waiting queue\n"
                 " 7. View all records (by ID)\n 8. Rebuild queue\n 9. Save\n 0. Save and exit\n";
}
static void show(const Patient* p) {
    std::cout << "  ID " << p->id << " | " << p->name << " | age " << p->age << " | severity "
              << p->severity << " | " << (p->waiting ? "Waiting" : "Treated") << " | " << p->complaint << "\n";
}

int main() {
    TriageSystem sys;
    if (sys.loadFromFile(DATA_FILE)) std::cout << "Loaded " << sys.recordCount() << " record(s) from " << DATA_FILE << ".\n";
    else std::cout << "No data file found - starting empty.\n";

    try {
        while (true) {
            menu();
            int c = readInt("Choice: ", 0, 9);
            if (c == 0) break;
            switch (c) {
            case 1: {
                int id = readInt("Patient ID (1-999999): ", 1, 999999);
                if (sys.findPatient(id)) { std::cout << "  ID already exists.\n"; break; }
                std::string name = readText("Name: ");
                int age = readInt("Age (0-120): ", 0, 120);
                int sev = readInt("Severity (1=minor ... 5=critical): ", 1, 5);
                std::string cmp = readText("Complaint: ");
                std::cout << (sys.addPatient(id, name, age, sev, cmp) ? "  Registered.\n" : "  Could not register.\n");
                break; }
            case 2: {
                int id = readInt("Patient ID: ", 1, 999999);
                Patient* p = sys.findPatient(id);
                if (p) show(p); else std::cout << "  Not found.\n";
                break; }
            case 3: {
                int id = readInt("Patient ID: ", 1, 999999);
                if (!sys.findPatient(id)) { std::cout << "  Not found.\n"; break; }
                int sev = readInt("New severity (1-5): ", 1, 5);
                std::string cmp = readText("New complaint (blank = keep): ", true);
                std::cout << (sys.updatePatient(id, sev, cmp) ? "  Updated.\n" : "  Update failed.\n");
                break; }
            case 4: {
                int id = readInt("Patient ID to delete: ", 1, 999999);
                Patient* p = sys.findPatient(id);
                if (!p) { std::cout << "  Not found - nothing deleted.\n"; break; }
                show(p);
                std::string yn = readLine("  Delete this patient permanently? (y/n): ");
                if (yn == "y" || yn == "Y")
                    std::cout << (sys.deletePatient(id) ? "  Deleted.\n" : "  Delete failed.\n");
                else std::cout << "  Cancelled.\n";
                break; }
            case 5: {
                Patient* p = sys.treatNext();
                if (p) { std::cout << "  Now treating:\n"; show(p); }
                else std::cout << "  No patients are waiting.\n";
                break; }
            case 6: sys.displayQueue(); break;
            case 7: sys.displayAllById(); break;
            case 8: sys.rebuildQueue(); std::cout << "  Queue rebuilt: " << sys.queueSize() << " patient(s) waiting.\n"; break;
            case 9: std::cout << (sys.saveToFile(DATA_FILE) ? "  Saved.\n" : "  Save FAILED.\n"); break;
            }
        }
    } catch (const std::exception&) {
        std::cout << "\nInput closed - saving and exiting.\n";
    }
    std::cout << (sys.saveToFile(DATA_FILE) ? "Data saved. Goodbye.\n" : "Could not save data!\n");
    return 0;                                     // destructor frees all memory
}

// tests.cpp - boundary tests for deletePatient() and rebuildQueue()
#include <cstdio>
#include <fstream>
#include <vector>
#include <cstdlib>
#include "TriageSystem.h"
static int pass = 0, fail = 0;
static void check(const char* id, bool ok) { std::printf("%-4s %s\n", id, ok ? "PASS" : "FAIL"); ok ? ++pass : ++fail; }
static std::vector<int> drain(TriageSystem& s) {          // serve everyone, return order of IDs
    std::vector<int> v; while (Patient* p = s.treatNext()) v.push_back(p->id); return v; }
using V = std::vector<int>;

int main() {
    // ---------- deletePatient ----------
    { TriageSystem s;                                      check("D1", !s.deletePatient(1)); }            // empty system
    { TriageSystem s; s.addPatient(1,"A",20,3,"x");
      check("D2", !s.deletePatient(99) && s.recordCount()==1 && s.queueSize()==1); }                         // unknown ID
    { TriageSystem s; s.addPatient(1,"A",20,3,"x");
      check("D3", s.deletePatient(1) && s.recordCount()==0 && s.queueSize()==0 && !s.peekNext()); }          // only patient
    { TriageSystem s; s.addPatient(1,"A",20,2,"x"); s.addPatient(2,"B",20,5,"x"); s.addPatient(3,"C",20,4,"x");
      check("D4", s.deletePatient(2) && s.peekNext()->id==3 && drain(s)==V{3,1}); }                          // delete top
    { TriageSystem s; s.addPatient(1,"A",20,5,"x"); s.addPatient(2,"B",20,4,"x"); s.addPatient(3,"C",20,1,"x");
      check("D5", s.deletePatient(3) && drain(s)==V{1,2}); }                                                 // delete lowest
    { TriageSystem s; int sev[]={5,4,4,3,3,2,1}; for (int i=0;i<7;i++) s.addPatient(i+1,"P",20,sev[i],"x");
      check("D6", s.deletePatient(4) && s.queueSize()==6 && drain(s)==V{1,2,3,5,6,7}); }                     // middle
    { TriageSystem s; s.addPatient(1,"A",20,3,"x"); s.addPatient(2,"B",20,3,"x"); s.treatNext();
      check("D7", s.deletePatient(1) && s.queueSize()==1 && s.recordCount()==1 && s.peekNext()->id==2); }    // treated patient
    { TriageSystem s; s.addPatient(1,"A",20,3,"x");
      bool a = s.deletePatient(1); bool b = s.deletePatient(1); check("D8", a && !b); }                       // twice
    { TriageSystem s; s.addPatient(1,"A",20,3,"x");
      check("D9", !s.deletePatient(0) && !s.deletePatient(-5) && s.recordCount()==1); }                      // invalid IDs
    { TriageSystem s; s.addPatient(1,"A",20,3,"x"); s.addPatient(2,"B",20,3,"x"); s.deletePatient(1); s.addPatient(1,"A2",20,3,"y");
      check("D10", drain(s)==V{2,1}); }                                                                      // re-add same ID -> new arrival
    { TriageSystem s; for (int i=1;i<=50;i++) s.addPatient(i,"P",20,1+i%5,"x");
      bool ok=true; for (int i=1;i<=50;i++) ok = ok && s.deletePatient(i);
      check("D11", ok && s.recordCount()==0 && s.queueSize()==0); }                                          // delete everything

    // ---------- rebuildQueue ----------
    { TriageSystem s; s.rebuildQueue();                    check("R1", s.queueSize()==0 && !s.peekNext()); }
    { TriageSystem s; s.addPatient(1,"A",20,3,"x"); s.rebuildQueue();
      check("R2", s.queueSize()==1 && s.peekNext()->id==1); }
    { TriageSystem s; s.addPatient(1,"A",20,3,"x"); s.addPatient(2,"B",20,4,"x"); s.treatNext(); s.treatNext(); s.rebuildQueue();
      check("R3", s.queueSize()==0 && s.recordCount()==2); }                                                 // all treated
    { TriageSystem s; for (int i=1;i<=6;i++) s.addPatient(i,"P",20,3,"x"); s.treatNext(); s.treatNext(); s.rebuildQueue();
      check("R4", s.queueSize()==4 && s.recordCount()==6); }                                                 // waiting only
    { TriageSystem s; for (int i=1;i<=8;i++) s.addPatient(i,"P",20,3,"x"); s.rebuildQueue();
      check("R5", drain(s)==V{1,2,3,4,5,6,7,8}); }                                                           // FIFO ties
    { std::ofstream o("t_load.txt");
      o<<"5|E|20|1|x|5|W\n4|D|20|5|x|4|W\n3|C|20|3|x|3|W\n2|B|20|5|x|2|W\n1|A|20|3|x|1|T\nbad line\n";
      o.close(); TriageSystem s; bool ok=s.loadFromFile("t_load.txt");
      check("R6", ok && s.recordCount()==5 && s.queueSize()==4 && drain(s)==V{2,4,3,5}); std::remove("t_load.txt"); }
    { TriageSystem s; s.addPatient(1,"A",20,2,"x"); s.addPatient(2,"B",20,5,"x"); s.addPatient(3,"C",20,4,"x");
      s.rebuildQueue(); s.rebuildQueue(); check("R7", s.queueSize()==3 && drain(s)==V{2,3,1}); }              // idempotent
    { TriageSystem s; std::srand(7); for (int i=1;i<=1000;i++) s.addPatient(i,"P",20,1+std::rand()%5,"x");
      s.rebuildQueue(); bool ok=true; size_t n=0; Patient last{};
      while (s.peekNext()) { Patient cur=*s.peekNext(); s.treatNext(); ++n;
        if (n>1) ok = ok && (last.severity>cur.severity || (last.severity==cur.severity && last.arrival<cur.arrival));
        last=cur; }
      check("R8", ok && n==1000); }                                                                           // 1000 patients ordered
    { TriageSystem s; for (int i=1;i<=10;i++) s.addPatient(i,"P",20,1+i%5,"x");
      s.deletePatient(3); s.deletePatient(10); s.rebuildQueue();
      check("R9", s.recordCount()==8 && s.queueSize()==8); }                                                 // after deletions
    std::printf("\n%d passed, %d failed\n", pass, fail);
    return fail ? 1 : 0;
}

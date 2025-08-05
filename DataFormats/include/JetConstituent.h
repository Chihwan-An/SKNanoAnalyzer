#ifndef JetConstituent_h
#define JetConstituent_h

#include <iostream>
#include "TLorentzVector.h"
#include "TMath.h"
#include "Particle.h"
#include "TString.h"

class JetConstituent: public Particle {
public:
    JetConstituent();
    ~JetConstituent();
    inline void SetPUPPIWeight(float weight) { j_puppiWeight = weight; }
    inline float PUPPIWeight() const { return j_puppiWeight; }

    inline void SetPID(int pid) { j_pid = pid; }
    inline int PID() const { return j_pid; }
    


private:
    int j_pid; // Particle ID
    float j_puppiWeight; // PUPPI weight
    ClassDef(JetConstituent, 1)

};

#endif

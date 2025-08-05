#ifndef JetConstitient_h
#define JetConstitient_h

#include <iostream>
#include "TLorentzVector.h"
#include "TMath.h"
//#include "Math/LorentzVector.h"
//#include "Math/Vector4D.h"
#include "TString.h"
#include "ROOT/RVec.hxx"
using namespace std;
using namespace ROOT::VecOps;

//typedef ROOT::Math::LorentzVector<ROOT::Math::PtEtaPhiM4D<float>> fLorentzVector;

class JetConstitient: public Particle {
public:
    // Default constructor with p4
    JetConstitient();
    // LorentzVector copy constructor
    JetConstitient(const TLorentzVector &p);
    // Copy constructor
    JetConstitient(const JetConstitient &p);

    inline void SetPUPPIWeight(float weight) { j_puppiWeight = weight; }
    inline float PUPPIWeight() const { return j_puppiWeight; }

    inline void SetPID(int pid) { j_pid = pid; }
    inline int PID() const { return j_pid; }
    


private:
    int j_pid; // Particle ID
    float j_puppiWeight; // PUPPI weight
    ClassDef(JetConstitient, 1)

};

#endif

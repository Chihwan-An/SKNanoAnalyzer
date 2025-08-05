#ifndef JetConstituent_h
#define JetConstituent_h

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

class JetConstituent: public Particle {
public:
    // Default constructor with p4
    JetConstituent();
    // LorentzVector copy constructor
    JetConstituent(const TLorentzVector &p);
    // Copy constructor
    JetConstituent(const JetConstituent &p);

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

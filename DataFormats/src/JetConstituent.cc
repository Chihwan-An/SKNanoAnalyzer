#include "JetConstitient.h"
ClassImp(JetConstitient);

JetConstitient::JetConstitient(): TLorentzVector(), j_Charge(0) {}
JetConstitient::JetConstitient(const TLorentzVector &p): TLorentzVector(p), j_Charge(0) {}
JetConstitient::JetConstitient(const JetConstitient &p): TLorentzVector(p), j_Charge(p.Charge()) {}
//JetConstitient::JetConstitient(float px, float py, float pz, float e): LorentzVector<PtEtaPhiM4D<float>>(px, py, pz, e), j_Charge(0) {}
JetConstitient::~JetConstitient() {}

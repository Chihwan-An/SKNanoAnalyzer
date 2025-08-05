#include "JetConstituent.h"
ClassImp(JetConstituent);

JetConstituent::JetConstituent(): TLorentzVector(), j_Charge(0) {}
JetConstituent::JetConstituent(const TLorentzVector &p): TLorentzVector(p), j_Charge(0) {}
JetConstituent::JetConstituent(const JetConstituent &p): TLorentzVector(p), j_Charge(p.Charge()) {}
//JetConstituent::JetConstituent(float px, float py, float pz, float e): LorentzVector<PtEtaPhiM4D<float>>(px, py, pz, e), j_Charge(0) {}
JetConstituent::~JetConstituent() {}

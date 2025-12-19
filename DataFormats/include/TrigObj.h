#ifndef TrigObj_h
#define TrigObj_h

#include <cstddef>
#include <limits>
#include <memory>

#include "Particle.h"
#include "Rtypes.h"
#include "TrigObjView.h"

class TrigObj : public Particle{
public:
    TrigObj();
    TrigObj(std::shared_ptr<const TrigObjSoA> storage, std::size_t index);
    ~TrigObj();
    
    // Setting functions
    inline void SetId(int id) { materialize(); j_id = id; }
    inline void SetFilterBits(ULong64_t filterBits) { materialize(); j_filterBits = filterBits; }
    inline void SetRun(int run) { materialize(); j_run = run; }
    inline void SetL1Charge(short charge) { materialize(); j_l1charge = charge; }
    inline void SetL1Iso(int iso) { materialize(); j_l1iso = iso; }
    inline void SetL1Pt(float pt) { materialize(); j_l1pt = pt; }
    inline void SetL1Pt2(float pt) { materialize(); j_l1pt2 = pt; }
    inline void SetL2Pt(float pt) { materialize(); j_l2pt = pt; }
    
    // Getter functions
    inline int id() const { materialize(); return j_id; }
    inline ULong64_t filterBits() const { materialize(); return j_filterBits; }
    inline int run() const { materialize(); return j_run; }
    inline short l1Charge() const { materialize(); return j_l1charge; }
    inline int l1Iso() const { materialize(); return j_l1iso; }
    inline float l1Pt() const { materialize(); return j_l1pt; }
    inline float l1Pt2() const { materialize(); return j_l1pt2; }
    inline float l2Pt() const { materialize(); return j_l2pt; }
    
    // Helper functions to check object type based on ID
    inline bool isElectron() const { return id() == 11; }
    inline bool isPhoton() const { return id() == 22; }
    inline bool isMuon() const { return id() == 13; }
    inline bool isTau() const { return id() == 15; }
    inline bool isJet() const { return id() == 1; }
    inline bool isFatJet() const { return id() == 6; }
    inline bool isMET() const { return id() == 2; }
    inline bool isHT() const { return id() == 3; }
    inline bool isMHT() const { return id() == 4; }
    inline bool isBoostedTau() const { return id() == 1515; }
    
    // Helper function to check if a specific bit is set
    inline bool hasBit(int bit) const { return (filterBits() & (ULong64_t(1) << bit)) != 0; }
    
    // Main helper function for trigger matching
    bool passTriggerFilter(int id, const TString& hltPath) const;
    
    // Electron-specific filter checks
    bool passElectronFilter(const TString& hltPath) const;
    
    // Muon-specific filter checks
    bool passMuonFilter(const TString& hltPath) const;
    
    // Tau-specific filter checks
    bool passTauFilter(const TString& hltPath) const;
    
    // Jet-specific filter checks
    bool passJetFilter(const TString& hltPath) const;
    
    // HT-specific filter checks
    bool passHTFilter(const TString& hltPath) const;
    
    // MHT-specific filter checks
    bool passMHTFilter(const TString& hltPath) const;
    
    // Photon-specific filter checks
    bool passPhotonFilter(const TString& hltPath) const;
    
    // FatJet-specific filter checks
    bool passFatJetFilter(const TString& hltPath) const;
    
    // BoostedTau-specific filter checks
    bool passBoostedTauFilter(const TString& hltPath) const;

private:
    void initializeMembers();
    void materialize() const;

    // Object ID: 11=Electron, 22=Photon, 13=Muon, 15=Tau, 1=Jet, 6=FatJet, 2=MET, 3=HT, 4=MHT, 1515=BoostedTau
    int j_id;
    // Filter bits with associated trigger information
    ULong64_t j_filterBits;
    // Run number (2 or 3) to determine which filterBits definition to use
    int j_run;
    // L1/L2 seed info
    short j_l1charge;
    int j_l1iso;
    float j_l1pt;
    float j_l1pt2;
    float j_l2pt;

    mutable std::shared_ptr<const TrigObjSoA> storage_;
    mutable std::size_t index_ = std::numeric_limits<std::size_t>::max();
    
    ClassDef(TrigObj, 1)
};

#endif

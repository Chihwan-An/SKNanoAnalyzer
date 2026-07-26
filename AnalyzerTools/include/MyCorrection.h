#ifndef MyCorrection_h
#define MyCorrection_h

#include <array>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <memory>
#include <sstream>
using namespace std;

#include "AnalysisException.h"
#include "TString.h"
#include "TRandom3.h"
#include "TMath.h"
#include "correction.h"
#include "RoccoR.h"
#include "JetView.h"
#include "JetTaggingParameter.h"
#include "GenView.h"
#include "MuonView.h"
#include "ElectronView.h"
#include "FatJetView.h"
#include "Particle.h"
#include "GoldenJsonParser.h"
#include "Variation.h"
#include "CorrectionBatch.h"

#include "MLHelper.h"
#include "TString.h"

using correction::CorrectionSet;

class MyCorrection {
public:
    using variation = SKNano::Variation;

    struct MuonScaleAndError {
        float scale = 1.f;
        float error = 0.f;
    };

    struct JERSFSet {
        float nom = 1.f;
        float up = 1.f;
        float down = 1.f;
    };

    struct JERSFVariations {
        float up = 1.f;
        float down = 1.f;
    };

    enum class POG {
        MUO,
        LUM,
        BTV,
        EGM,
        JME
    };

    enum class XYCorrection_MetType {
        Type1PFMET,
        Type1PuppiMET
    };
    MyCorrection();
    MyCorrection(const TString &era, const TString &period, const TString &sample, bool IsData,const string &btagging_eff_file = "btaggingEff.json", const string &ctagging_eff_file = "ctaggingEff.json", const string &btagging_R_file = "btaggingR.json", const string &ctagging_R_file = "ctaggingR.json");
    ~MyCorrection();


    //GoldenLumi
    bool IsGoldenLumi(const unsigned int runNumber, const unsigned int lumiSection) const;

    // Muon
    MuonScaleAndError GetMuonScaleAndError(int charge, float pt, float eta,
                                           float phi, int trackerLayers,
                                           float matchedPt = 0.f) const;
    float GetMuonRECOSF(const MuonView &muon, const variation syst = variation::nom) const;
    float GetMuonRECOSF(const MuonViewCollection &muons, const variation syst = variation::nom) const;
    float GetMuonIDSF(const TString &key, const MuonView &muon,
                      variation syst = variation::nom) const;
    float GetMuonIDSF(const TString &key, const MuonViewCollection &muons,
                      const std::vector<std::size_t> &indices,
                      variation syst = variation::nom) const;
    float GetMuonIDSF(const TString &key, const MuonViewCollection &muons,
                      variation syst = variation::nom) const;
    inline float GetMuonISOSF(const TString &key,
                              const MuonViewCollection &muons,
                              variation syst = variation::nom,
                              const TString &source = "") const {
      static_cast<void>(source);
      return GetMuonIDSF(key, muons, syst);
    }

    // electron
    float GetElectronScaleUnc(const float scEta, const unsigned char seedGain, const unsigned int runNumber, const float r9, const float pt, const variation syst = variation::nom) const;
    float GetElectronRECOSF(const float abseta, const float pt, const float phi, const variation syst = variation::nom) const;
    float GetElectronRECOSF(const ElectronView &electron, const variation syst = variation::nom) const;
    float GetElectronRECOSF(const ElectronViewCollection &electrons, const variation syst = variation::nom) const;
    float GetElectronIDSF(const TString &Electron_ID_SF_Key, const float abseta, const float pt, const float phi, const variation syst = variation::nom) const;
    float GetElectronIDSF(const TString &key, const ElectronView &electron, const variation syst = variation::nom) const;
    float GetElectronIDSF(const TString &key, const ElectronViewCollection &electrons, const variation syst = variation::nom) const;
    // photon

    // Trigger
    // Single lepton trigger from POG
    float GetMuonTriggerEff(const TString &Muon_Trigger_Eff_Key, const float abseta, const float pt, const bool isData, const variation syst = variation::nom) const;
    float GetMuonTriggerSF(const TString &key, const MuonView &muon, const variation syst = variation::nom) const;
    float GetMuonTriggerSF(const TString &key, const MuonViewCollection &muons, const variation syst = variation::nom) const;
    float GetElectronTriggerEff(const TString &Electron_ID_SF_Key, const float eta, const float pt, const float phi, const bool isDATA, const variation syst = variation::nom) const;
    inline float GetElectronTriggerDataEff(const TString &Electron_ID_SF_Key, const float eta, const float pt, const float phi, const variation syst = variation::nom) {
        return GetElectronTriggerEff(Electron_ID_SF_Key, eta, pt, phi, true, syst);
    };
    inline float GetElectronTriggerMCEff(const TString &Electron_ID_SF_Key, const float eta, const float pt, const float phi, const variation syst = variation::nom) {
        return GetElectronTriggerEff(Electron_ID_SF_Key, eta, pt, phi, false, syst);
    };
    float GetElectronTriggerSF(const TString &Electron_Trigger_SF_Key, const float eta, const float pt, const float phi, const variation syst = variation::nom) const;

    // PUWeights
    float GetPUWeight(const float nTrueInt, const variation syst = variation::nom, const TString &source = "") const;

    // tagging param
    void SetTaggingParam(JetTagging::JetFlavTagger tagger, JetTagging::JetFlavTaggerWP wp);

    // btaging
    float GetBTaggingWP() const;
    float GetBTaggingWP(JetTagging::JetFlavTagger tagger, JetTagging::JetFlavTaggerWP wp) const;
    float GetBTaggingEff(const float eta, const float pt, const int flav, JetTagging::JetFlavTagger tagger, JetTagging::JetFlavTaggerWP wp, const variation syst = variation::nom);

    // ctagging
    pair<float, float> GetCTaggingWP() const;
    pair<float, float> GetCTaggingWP(JetTagging::JetFlavTagger tagger, JetTagging::JetFlavTaggerWP wp) const;
    float GetCTaggingEff(const float eta, const float pt, const int flav, JetTagging::JetFlavTagger tagger, JetTagging::JetFlavTaggerWP wp, const variation syst = variation::nom);
    float GetCTaggingR(const float npvs, const float HT, const JetTagging::JetFlavTagger tagger, const TString &processName = "", const TString &ttBarCategory = "total", const TString &syst_str = "") const;

    // Jet ID
    bool PassJetID(const JetView &jet, const JetView::JetID &id) const;
    bool PassFatJetID(const FatJetView &fatjet, FatJetView::ID id) const;

    // JERC
    float GetJER(const float eta, const float pt, const float rho) const;
    float GetJERSF(const float eta, const float pt, const variation syst = variation::nom, const TString &source = "total") const;
    JERSFSet GetJERSFSet(const float eta, const float pt, const TString &source = "total") const;
    JERSFVariations GetJERSFVariations(const float eta, const float pt,
                                       const float nominal,
                                       const TString &source = "total") const;
    float GetJESSF(const float area, const float eta, const float pt, const float phi, const float rho, const unsigned int runNumber) const;
    float GetJESUncertaintySF(const float eta, const float pt, const variation syst = variation::nom, const TString &source = "total") const;
    float GetJESUncertainty(const float eta, const float pt, const TString &source = "total") const;
    void EvaluateJetCorrectionBatch(
        const SKNano::JetCorrectionBatchInput &input,
        SKNano::JetCorrectionBatchOutput &output,
        SKNano::CorrectionLaneMask jerLanes = {},
        SKNano::CorrectionLaneMask jesLanes = {},
        const TString &jesSource = "total") const;
    // jerc_fatjet
    
    // jetvetomap
    bool IsJetVetoZone(const float eta, const float phi, TString mapCategory) const;
    
    // MET
    void METXYCorrection(Particle &Met, const int RunNumber, const int npvs, const XYCorrection_MetType MetType);
    
    // reweighting
    float GetTopPtReweight(const TLorentzVector &LastCopyTop, const TLorentzVector &LastCopyAntiTop) const;
    float GethDampReweight(const TLorentzVector &FirstCopyTop, const TLorentzVector &FirstCopyAntiTop, const variation &syst) const;
    float GetBFragReweight(const TLorentzVector &LastCopyTop, const TLorentzVector &LastCopyAntiTop, const TLorentzVector &LastCopyWPlus, const TLorentzVector &LastCopyWMinus,
                            const TLorentzVector &FirstCopyBHadronFromTop, const TLorentzVector &FirstCopyBHadronFromAntiTop, const variation &syst) const;
    // helper function for getbfrag
    std::array<size_t, 6> GetGenIdxofTopDecayProducts(const GenViewCollection &gens) const;
    // Safe evaluation function for correction sets with comprehensive error handling
    template<typename... Args>
    inline float safeEvaluate(const correction::Correction::Ref &cset, 
                              const string &function_name,
                              const vector<correction::Variable::Type> &args) const {
        if (!cset) {
            throw SKNano::ConfigError("[MyCorrection::" + function_name + "] Correction set is null");
        }
        
        try {
            return cset->evaluate(args);
        } catch (const std::exception &e) {
            std::ostringstream oss;
            oss << "[MyCorrection::" << function_name << "] Error during evaluation: "
                << e.what() << "; arguments (" << args.size() << "): ";
            for (const auto &arg : args) {
                std::visit([&oss](const auto &value) { oss << value << " "; }, arg);
            }
            throw SKNano::CorrectionError(oss.str());
        }
    }

    // Overload for CompoundCorrection
    template<typename... Args>
    inline float safeEvaluate(const correction::CompoundCorrection::Ref &cset, 
                              const string &function_name,
                              const vector<correction::Variable::Type> &args) const {
        if (!cset) {
            throw SKNano::ConfigError("[MyCorrection::" + function_name + "] CompoundCorrection set is null");
        }
        
        try {
            return cset->evaluate(args);
        } catch (const std::exception &e) {
            std::ostringstream oss;
            oss << "[MyCorrection::" << function_name << "] Error during evaluation: "
                << e.what() << "; arguments (" << args.size() << "): ";
            for (const auto &arg : args) {
                std::visit([&oss](const auto &value) { oss << value << " "; }, arg);
            }
            throw SKNano::CorrectionError(oss.str());
        }
    }

    // correctionlib's public evaluator accepts a vector<variant>.  JEC calls
    // have a fixed, all-floating signature, so keep one buffer per arity and
    // thread instead of allocating and constructing an initializer-list
    // vector for every correction level of every jet.
    template <typename CorrectionRef, std::size_t N>
    inline float safeEvaluateFloats(const CorrectionRef &cset,
                                    const string &function_name,
                                    const std::array<float, N> &values) const {
        static thread_local vector<correction::Variable::Type> args(N);
        if (args.size() != N)
            args.resize(N);
        for (std::size_t index = 0; index < N; ++index)
            args[index] = static_cast<double>(values[index]);
        return safeEvaluate(cset, function_name, args);
    }

private:
    struct EraConfig {
        string json_muon;
        string json_muon_trig_sf;
        string json_muon_trig_eff;
        string json_puWeights;
        string json_btagging;
        string json_ctagging;
        string json_btagging_eff;
        string json_ctagging_eff;
        string json_btagging_R;
        string json_ctagging_R;
        string json_electron;
        string json_electron_id;
        string json_electron_hlt;
        string json_electron_variation;
        string json_photon;
        string json_jetid;
        string json_jerc;
        string json_jerc_fatjet;
        string json_jetvetomap;
        string json_jmar;
        string json_met;
        string txt_roccor;
        string golden_json;

        //onnx file location
        string onnx_toppt_reweight;
        string onnx_hDampUp;
        string onnx_hDampDown;
        string onnx_rBnom;
        string onnx_rBUp;
    };
    EraConfig GetEraConfig(TString era, const string &btagging_eff_file, const string &ctagging_eff_file, const string &btagging_R_file, const string &ctagging_R_file) const;

    inline void SetEra(TString era) {
        DataEra = era;
        if (era.Contains("2016") || era.Contains("2017") || era.Contains("2018")) {
            Run = 2;
        } else if (era.Contains("2022") || era.Contains("2023") || era.Contains("2024")) {
            Run = 3;
        } else {
            throw runtime_error("Invalid era: " + era);
        }
    }
    inline void SetPeriod(TString period) { DataPeriod = period; }
    inline TString GetEra() const { return DataEra; }
    inline TString GetPeriod() const { return DataPeriod; }
    inline void SetSample(TString sample) { Sample = sample; }
    inline void setIsData(bool isData) { IsDATA = isData; }

    inline bool loadCorrectionSet(const string &name,
                                  const string &file,
                                  unique_ptr<CorrectionSet> &cset,
                                  bool optional = false) {
        cout << "[MyCorrection::loadCorrectionSet] " << name << ": " << file << endl;
        try {
            cset = CorrectionSet::from_file(file);
            return true;
        } catch (const exception &e) {
            if (optional) {
                cerr << "[MyCorrection::loadCorrectionSet] Warning: Failed to load " << name << " (" << file << "): " << e.what() << endl;
                return false;
            } else {
                cerr << "[MyCorrection::loadCorrectionSet] Error: Failed to load " << name << " (" << file << "): " << e.what() << endl;
                throw;
            }
        }
    }

    inline bool loadRoccoR(const string &file, bool optional=false) {
        cout << "[MyCorrection::loadRoccoR] " << file << endl;
        try {
            rc.init(file);
            return true;
        } catch (const exception &e) {
            if (optional) {
                cerr << "[MyCorrection::loadRoccoR] Warning: Failed to load " << file << " (" << e.what() << ")" << endl;
                return false;
            } else {
                cerr << "[MyCorrection::loadRoccoR] Error: Failed to load " << file << " (" << e.what() << ")" << endl;
                throw;
            }
        }
    }

    inline bool loadGoldenJson(const string &file, bool optional = false) {
        cout << "[MyCorrection::loadGoldenJson] " << file << endl;
        try {
            golden_json_parser = make_unique<GoldenJsonParser>(file);
            return true;
        } catch (const exception &e) {
            if (optional) {
                cerr << "[MyCorrection::loadGoldenJson] Warning: Failed to load " << file << " (" << e.what() << ")" << endl;
                return false;
            } else {
                cerr << "[MyCorrection::loadGoldenJson] Error: Failed to load " << file << " (" << e.what() << ")" << endl;
                throw;
            }
        }
    }

    inline bool isInputInCorrection(const string &key, const correction::Correction::Ref &cset) const {
        vector<string> inputs;
        for (const auto &input : cset->inputs()) {
            inputs.push_back(input.name());
        }
        return find(inputs.begin(), inputs.end(), key) != inputs.end();
    }

    const correction::Correction::Ref &getJERPtResolutionCorrection() const;
    const correction::Correction::Ref &getJERScaleFactorCorrection() const;
    const correction::Correction::Ref &getJERSFUncertaintyCorrection() const;
    const correction::Correction::Ref &getJESUncertaintyCorrection(const string &source) const;
    float safeEvaluate2D(const correction::Correction::Ref &cset, const string &function_name, float x, float y) const;
    float safeEvaluate3D(const correction::Correction::Ref &cset, const string &function_name, float x, float y, float z) const;

    JetTagging::JetFlavTaggerWP global_wp;
    JetTagging::JetFlavTagger global_tagger;
    string global_wpStr;
    string global_taggerStr;
    TString DataEra;
    TString DataPeriod;
    int Run;
    TString Sample;
    bool IsDATA;

    unique_ptr<MLHelper> MLHelper_TopPtReweight;
    unique_ptr<MLHelper> MLHelper_hDampUp;
    unique_ptr<MLHelper> MLHelper_hDampDown;
    unique_ptr<MLHelper> MLHelper_rBnom;
    unique_ptr<MLHelper> MLHelper_rBUp;
    

    unique_ptr<CorrectionSet> cset_muon;
    unique_ptr<CorrectionSet> cset_muon_trig_eff;
    unique_ptr<CorrectionSet> cset_muon_trig_sf;
    unique_ptr<CorrectionSet> cset_puWeights;
    unique_ptr<CorrectionSet> cset_btagging;
    unique_ptr<CorrectionSet> cset_ctagging;
    unique_ptr<CorrectionSet> cset_btagging_eff;
    unique_ptr<CorrectionSet> cset_ctagging_eff;
    unique_ptr<CorrectionSet> cset_btagging_R;
    unique_ptr<CorrectionSet> cset_ctagging_R;
    unique_ptr<CorrectionSet> cset_electron;
    unique_ptr<CorrectionSet> cset_electron_id;
    unique_ptr<CorrectionSet> cset_electron_hlt;
    unique_ptr<CorrectionSet> cset_electron_variation;
    unique_ptr<CorrectionSet> cset_photon;
    unique_ptr<CorrectionSet> cset_jetid;
    unique_ptr<CorrectionSet> cset_jerc;
    unique_ptr<CorrectionSet> cset_jerc_fatjet;
    unique_ptr<CorrectionSet> cset_jetvetomap;
    unique_ptr<CorrectionSet> cset_jmar;
    unique_ptr<CorrectionSet> cset_met;

    unordered_map<string, string> MUO_keys;
    unordered_map<string, string> LUM_keys;
    unordered_map<string, string> BTV_keys;
    unordered_map<string, string> EGM_keys;
    unordered_map<string, string> JME_JER_GT;
    unordered_map<string, string> JME_JES_GT;
    unordered_map<string, string> JME_vetomap_keys;
    unordered_map<string, string> JME_PILEUP_keys;
    unordered_map<string, string> JME_MET_keys;

    mutable correction::Correction::Ref cachedJERPtResolution;
    mutable correction::Correction::Ref cachedJERScaleFactor;
    mutable correction::Correction::Ref cachedJERSFUncertainty;
    mutable unordered_map<string, correction::Correction::Ref> cachedJESUncertaintyCorrections;
    // Immutable for a configured era/data mode.  The first JEC request binds
    // the correctionlib refs once instead of doing string construction and
    // CorrectionSet lookups for every jet.
    mutable bool preparedJESValid = false;
    mutable string preparedJESEra;
    mutable bool preparedJESIsData = false;
    mutable correction::CompoundCorrection::Ref preparedJESCompound;
    mutable correction::Correction::Ref preparedJESL1;
    mutable correction::Correction::Ref preparedJESL2;
    mutable correction::Correction::Ref preparedJESL3;
    mutable correction::Correction::Ref preparedJESResidual;
    
    RoccoR rc;
    unique_ptr<GoldenJsonParser> golden_json_parser;
    // All POG choose different string for the systematics, so we need to convert the enum to the string....
    // Here I implement every single function instead of a general one, because heavy use of switch-case might be slow.
    inline string getSystString_CUSTOM(const variation syst) const {
        string sys_string = "nom";
        switch (syst) {
        case variation::nom:
            sys_string = "nom";
            break;
        case variation::up:
            sys_string = "up";
            break;
        case variation::down:
            sys_string = "down";
            break;
        };
        return sys_string;
    };

    inline string getSystString_MUO(const variation syst) const {
        string sys_string = "nominal";
        switch (syst) {
        case variation::nom:
            sys_string = "nominal";
            break;
        case variation::up:
            sys_string = "systup";
            break;
        case variation::down:
            sys_string = "systdown";
            break;
        };
        return sys_string;
    };

    inline string getSystString_LUM(const variation syst) const {
        string sys_string = "nominal";
        switch (syst) {
        case variation::nom:
            sys_string = "nominal";
            break;
        case variation::up:
            sys_string = "up";
            break;
        case variation::down:
            sys_string = "down";
            break;
        };
        return sys_string;
    };

    inline string getSystString_BTV(const variation syst) const {
        string sys_string = "central";
        switch (syst) {
        case variation::nom:
            sys_string = "central";
            break;
        case variation::up:
            sys_string = "up";
            break;
        case variation::down:
            sys_string = "down";
            break;
        };
        return sys_string;
    };

    inline string getSystString_EGM(const variation syst) const {
        string sys_string = "sf";
        switch (syst) {
        case variation::nom:
            sys_string = "sf";
            break;
        case variation::up:
            sys_string = "sfup";
            break;
        case variation::down:
            sys_string = "sfdown";
            break;
        };
        return sys_string;
    };
    
    inline string getSystString_EGMScale(const variation syst) const {
        // Only for Run2
        if (! (Run == 2)) {
            throw runtime_error("[MyCorrection::getSystString_EGMScale] Use getSystString_EGM for Run3");
        }
        string sys_string = "";
        switch(syst) {
        case variation::nom:
            sys_string = "";
            break;
        case variation::up:
            sys_string = "scaleup";
            break;
        case variation::down:
            sys_string = "scaledown";
            break;
        };
        return sys_string;
    }
    
    inline string getSystString_JME(const variation syst) const {
        string sys_string = "nom";
        switch (syst) {
        case variation::nom:
            sys_string = "nom";
            break;
        case variation::up:
            sys_string = "up";
            break;
        case variation::down:
            sys_string = "down";
            break;
        };
        return sys_string;
    };
};

#endif

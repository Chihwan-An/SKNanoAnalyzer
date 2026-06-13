#ifndef SKNanoLoader_h
#define SKNanoLoader_h

#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

#include "TROOT.h"
#include "TChain.h"
#include "TChainElement.h"
#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TRandom3.h"
#include "ROOT/RVec.hxx"
#include "ROOT/RDataFrame.hxx"
#include <nlohmann/json.hpp>
#include "BranchManager.h"
#include "Triggerinfo.h"
using namespace ROOT::VecOps;

class SKNanoLoader
{
public:
    SKNanoLoader();
    virtual ~SKNanoLoader() = default;

    // virtual long GetEntry(long entry);
    virtual void SetTreeName(TString tname) { fChain = new TChain(tname); }
    virtual int AddFile(TString filename) { return fChain->Add(filename, -1); }
    void RegisterBranches();
    void ResetBranchStates();

    long MaxEvent, NSkipEvent;
    int LogEvery;
    bool IsDATA;
    TString DataStream;
    TString MCSample;
    TString Campaign;
    // bool IsFastSim;
    int DataYear;
    TString DataEra;
    TString DataPeriod;
    int Run;
    float xsec, sumW, sumSign;
    RVec<TString> Userflags;

    virtual void Init();
    virtual void Loop();
    virtual void executeEvent() {};

    virtual void SetEra(TString era)
    {
        DataEra = era;
        DataYear = TString(era(0, 4)).Atoi();
        // 2016, 2017, 2018 are Run2
        if (DataYear == 2016 or DataYear == 2017 or DataYear == 2018)
            Run = 2;
        else if (DataYear == 2022 or DataYear == 2023 or DataYear == 2024 or DataYear == 2025)
            Run = 3;
    }
    virtual void SetPeriod(TString period) { DataPeriod=period; }
    virtual void SetCampaign(TString campaign) { Campaign=campaign; }

    void SetTreeCacheSize(Long64_t cacheBytes) { treeCacheSizeBytes = cacheBytes; }
    void SetTreeCacheLearnEntries(int learnEntries) { treeCacheLearnEntries = learnEntries; }
    void EnableTreePrefetching(bool enable) { enableTreePrefetching = enable; }

    virtual TString GetEra() const { return DataEra; }
    virtual TString GetCampaign() const { return Campaign; }
    virtual int GetYear() const { return DataYear; }
    Long64_t CurrentEntry() const { return currentEntry; }
    Long64_t CurrentLocalEntry() const { return currentLocalEntry; }

    TChain *fChain = nullptr;
    BranchManager branchManager;
    TriggerMap_t TriggerMap;
    Long64_t currentEntry = -1;
    Long64_t currentLocalEntry = -1;
    int currentTreeNumber = -1;

    // A larger default cache and longer learn phase help steady throughput
    // across file boundaries.
    Long64_t treeCacheSizeBytes = 200LL * 1024 * 1024; // 200 MB default cache
    int treeCacheLearnEntries = 100;                   // learn quickly for short jobs
    bool enableTreePrefetching = true;
    static constexpr Long64_t CACHE_PREFETCH_WARMUP_EVENTS = 100;
    bool cachePrefetchConfigured = false;
    Long64_t cachePrefetchWarmupEntries = 0;

protected:
    void configureTreeCache(TTree *tree, bool resetCache = false);

#include "generated_branch_decls.inc"
};

#endif

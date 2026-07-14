#ifndef SKNanoLoader_h
#define SKNanoLoader_h

#include <cstddef>
#include <iostream>
#include <cstdint>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <unordered_map>
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
#include "AnalysisException.h"
#include "BranchManager.h"
#include "ClusterTaskPlanner.h"
#include "CounterRNG.h"
#include "FailureContext.h"
#include "FailurePolicy.h"
#include "ExecutionPlan.h"
#include "EventArena.h"
#include "EventBlock.h"
#include "PerformanceTelemetry.h"
#include "TriggerDecision.h"
#include "TreeCacheTuner.h"
#include "Triggerinfo.h"
#include "ViewColumns.h"
using namespace ROOT::VecOps;

class SKNanoLoader : public SKNano::TriggerDecisionProvider
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
    virtual std::string CurrentSystematicName() const { return ""; }
    bool lookupTrigger(const std::string &name,
                       SKNano::TriggerDecision &decision) const override;
    std::uint64_t triggerEpoch() const noexcept override { return eventEpoch; }

    template <typename T>
    ColumnHandle<T> GetColumnHandle(
        const std::string &name,
        ColumnRequirement requirement = ColumnRequirement::Required) {
        return branchManager.getColumnHandle<T>(name, requirement);
    }

    template <typename T>
    ScalarHandle<T> GetScalarHandle(
        const std::string &name,
        ColumnRequirement requirement = ColumnRequirement::Required) {
        return branchManager.getScalarHandle<T>(name, requirement);
    }

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

    void SetTreeCacheSize(Long64_t cacheBytes) {
        treeCacheSizeBytes = cacheBytes;
        autoTuneTreeCache = false;
    }
    void EnableTreeCacheAutoTune(bool enable = true) noexcept {
        autoTuneTreeCache = enable;
    }
    void SetIOMemoryBudget(std::size_t bytes) {
        if (bytes == 0)
            throw SKNano::ConfigError(
                "[SKNanoLoader] I/O memory budget must be positive");
        ioMemoryBudgetBytes = bytes;
    }
    void SetTreeCacheLearnEntries(int learnEntries) { treeCacheLearnEntries = learnEntries; }
    void EnableTreePrefetching(bool enable) { enableTreePrefetching = enable; }
    void SetAnalyzerName(const std::string &name) { analyzerName = name; }
    void SetFailurePolicy(SKNano::FailurePolicy policy) { failurePolicy = policy; }
    void SetFailurePolicy(const std::string &policy) { failurePolicy = SKNano::ParseFailurePolicy(policy); }
    void SetMaxEventErrors(int maxErrors) { maxEventErrors = maxErrors; }
    void SetErrorReportPath(const std::string &path) { errorReportPath = path; }
    void EnablePerformanceTelemetry(bool enable = true) {
        performanceTelemetry.setEnabled(enable);
    }
    void SetPerformanceReportPath(const std::string &path) {
        performanceTelemetry.setOutputPath(path);
        performanceTelemetry.setEnabled(true);
    }
    SKNano::PerformanceTelemetry::ScopedPhase
    MeasurePerformancePhase(const std::string &name) {
        return performanceTelemetry.measure(name);
    }
    void AddPerformanceCounter(const std::string &name, double value = 1.) {
        performanceTelemetry.addCounter(name, value);
    }
    void SetExecutionPlan(SKNano::ExecutionPlan plan) {
        executionPlan = std::move(plan);
        hasExecutionPlan = true;
    }
    const SKNano::ExecutionPlan *GetExecutionPlan() const noexcept {
        return hasExecutionPlan ? &executionPlan : nullptr;
    }
    std::vector<SKNano::ClusterTask>
    BuildCurrentTreeClusterTasks(std::size_t maximumEventsPerTask) const;
    void SetRngMode(SKNano::RngMode mode) noexcept { rngMode = mode; }
    SKNano::RngMode GetRngMode() const noexcept { return rngMode; }
    void EnableEventBlockMode(bool enable = true) noexcept {
        eventBlockMode = enable;
    }
    void SetEventBlockLimits(std::size_t memoryBudgetBytes,
                             std::size_t maximumEvents) {
        if (memoryBudgetBytes == 0 || maximumEvents == 0)
            throw SKNano::ConfigError(
                "[SKNanoLoader] EventBlock limits must be positive");
        eventBlockMemoryBudgetBytes = memoryBudgetBytes;
        eventBlockMaximumEvents = maximumEvents;
    }
    void EnableEventArena(std::size_t initialBytes = 8 * 1024 * 1024) {
        eventArena = std::make_unique<SKNano::EventArena>(initialBytes);
    }
    bool HasEventArena() const noexcept { return static_cast<bool>(eventArena); }
    SKNano::EventArena &GetEventArena() {
        if (!eventArena)
            throw SKNano::LogicError(
                "[SKNanoLoader] EventArena was not enabled");
        return *eventArena;
    }

    virtual TString GetEra() const { return DataEra; }
    virtual TString GetCampaign() const { return Campaign; }
    virtual int GetYear() const { return DataYear; }
    Long64_t CurrentEntry() const { return currentEntry; }
    Long64_t CurrentLocalEntry() const { return currentLocalEntry; }

    TChain *fChain = nullptr; //!
    BranchManager branchManager; //!
    TriggerMap_t TriggerMap;
    mutable SKNano::TriggerDecisionMap triggerDecisionCache; //!
    mutable Long64_t triggerDecisionCacheEntry = -1;
    Long64_t currentEntry = -1;
    Long64_t currentLocalEntry = -1;
    std::uint64_t eventEpoch = 0;
    int currentTreeNumber = -1;
    std::string analyzerName = "SKNanoLoader";
    SKNano::FailurePolicy failurePolicy = SKNano::FailurePolicy::FailFast;
    int maxEventErrors = 1;
    int eventErrorCount = 0;
    std::string errorReportPath;
    std::unordered_map<std::string, int> errorCountsByCategory;
    SKNano::PerformanceTelemetry performanceTelemetry; //!
    SKNano::ExecutionPlan executionPlan; //!
    bool hasExecutionPlan = false;
    SKNano::RngMode rngMode = SKNano::RngMode::StrictLegacy;
    bool eventBlockMode = false;
    std::size_t eventBlockMemoryBudgetBytes = 64 * 1024 * 1024;
    std::size_t eventBlockMaximumEvents = 256;
    std::unique_ptr<SKNano::EventArena> eventArena; //!

    // A larger default cache and longer learn phase help steady throughput
    // across file boundaries.
    Long64_t treeCacheSizeBytes = 200LL * 1024 * 1024; // 200 MB default cache
    Long64_t effectiveTreeCacheSizeBytes = treeCacheSizeBytes;
    std::size_t ioMemoryBudgetBytes = 512ULL * 1024 * 1024;
    bool autoTuneTreeCache = true;
    int treeCacheLearnEntries = 100;                   // learn quickly for short jobs
    bool enableTreePrefetching = true;
    static constexpr Long64_t CACHE_PREFETCH_WARMUP_EVENTS = 100;
    bool cachePrefetchConfigured = false;
    Long64_t cachePrefetchWarmupEntries = 0;
    Long64_t performanceStartBytesRead = 0;
    int performanceStartReadCalls = 0;
    Long64_t performanceEventsProcessed = 0;

protected:
    // Experimental D2b seam. An analyzer must opt in explicitly and own all
    // data needed by executeEventBlock(); ordinary analyzers remain on D2a.
    virtual bool SupportsOwnedEventBlocks() const noexcept { return false; }
    virtual void ConfigureEventBlock(SKNano::EventBlock &) {}
    virtual bool CanGatherCurrentEventIntoBlock() const { return false; }
    virtual std::size_t CurrentEventBlockPayloadBytes() const { return 0; }
    virtual void AppendCurrentEventToBlock(SKNano::EventBlock &) {
        throw SKNano::LogicError(
            "[SKNanoLoader] EventBlock gather hook is not implemented");
    }
    virtual void executeEventBlock(const SKNano::EventBlock &) {
        throw SKNano::LogicError(
            "[SKNanoLoader] EventBlock execute hook is not implemented");
    }
    void LoopEventBlocks();
    void configureTreeCache(TTree *tree, bool resetCache = false);
    void AddActivatedBranchToCache(const std::string &name);
    SKNano::FailureContext BuildFailureContext() const;
    void RecordFailure(const SKNano::FailureContext &context,
                       const std::string &category,
                       const std::string &message,
                       const std::string &exceptionType);
    void WriteFailureSummary() const;
    void WritePerformanceSummary();
    void ValidateExecutionPlanTree(TTree *tree) const;

#include <generated_loader_api.inc>
};

#endif

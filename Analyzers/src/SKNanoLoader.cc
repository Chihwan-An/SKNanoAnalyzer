#include "SKNanoLoader.h"
#include "TTreeCache.h"
#include "TBranch.h"
#include "TLeaf.h"
#include "TObjArray.h"
#include "TIterator.h"
#include <algorithm>
#include <exception>
#include <iomanip>
using json = nlohmann::json;

namespace {

std::string summaryPathFor(const std::string &jsonlPath)
{
    if (jsonlPath.size() >= 6 &&
        jsonlPath.compare(jsonlPath.size() - 6, 6, ".jsonl") == 0)
        return jsonlPath.substr(0, jsonlPath.size() - 6) + ".summary.json";
    return jsonlPath + ".summary.json";
}

template <typename BranchT, typename ValueT>
void tryReadBranch(const BranchT &branch, ValueT &value)
{
    try
    {
        if (branch.valid())
            value = static_cast<ValueT>(branch);
    }
    catch (...)
    {
    }
}

} // namespace

SKNanoLoader::SKNanoLoader()
{
    MaxEvent = -1;
    NSkipEvent = 0;
    LogEvery = 1000;
    IsDATA = false;
    DataStream = "";
    MCSample = "";
    SetEra("2018");
    xsec = 1.;
    sumW = 1.;
    sumSign = 1.;
    Userflags.clear();
    branchManager.setActivationCallback(
        [this](const std::string &name) { AddActivatedBranchToCache(name); });
}

void SKNanoLoader::AddActivatedBranchToCache(const std::string &name)
{
    if (!cachePrefetchConfigured || !enableTreePrefetching || !fChain)
        return;

    TTree *tree = fChain->GetTree();
    if (!tree || !tree->GetBranch(name.c_str()))
        return;
    tree->AddBranchToCache(name.c_str(), true);
    if (auto *file = tree->GetCurrentFile())
    {
        if (auto *cache = tree->GetReadCache(file, false))
            cache->StopLearningPhase();
    }
}

void SKNanoLoader::configureTreeCache(TTree *tree, bool resetCache)
{
    if (!tree)
        return;

    effectiveTreeCacheSizeBytes = treeCacheSizeBytes;
    if (autoTuneTreeCache && treeCacheSizeBytes > 0) {
        std::unordered_set<std::string> names(
            branchManager.getActiveBranches().begin(),
            branchManager.getActiveBranches().end());
        if (hasExecutionPlan)
            for (const auto &column : executionPlan.columns())
                names.insert(column.name);
        std::uint64_t activeCompressedBytes = 0;
        for (const auto &name : names)
            if (auto *branch = tree->GetBranch(name.c_str()))
                activeCompressedBytes +=
                    static_cast<std::uint64_t>(std::max<Long64_t>(
                        branch->GetZipBytes(), 0));
        std::uint64_t clusterEntries = 0;
        TTree *clusterTree = tree;
        if (auto *chain = dynamic_cast<TChain *>(tree))
            clusterTree = chain->GetTree();
        if (clusterTree) {
            auto cluster = clusterTree->GetClusterIterator(0);
            const Long64_t clusterBegin = cluster();
            if (clusterBegin >= 0) {
                const Long64_t clusterEnd = cluster.GetNextEntry();
                if (clusterEnd > clusterBegin)
                    clusterEntries = static_cast<std::uint64_t>(
                        clusterEnd - clusterBegin);
            }
        }
        const auto tuned = SKNano::TreeCacheTuner::tune({
            static_cast<std::size_t>(treeCacheSizeBytes),
            ioMemoryBudgetBytes,
            eventBlockMode ? eventBlockMemoryBudgetBytes : 0,
            activeCompressedBytes,
            static_cast<std::uint64_t>(std::max<Long64_t>(tree->GetEntries(), 0)),
            clusterEntries,
            false // this loader owns only one ROOT read context
        });
        effectiveTreeCacheSizeBytes =
            static_cast<Long64_t>(tuned.cacheBytes);
        performanceTelemetry.setCounter(
            "active_cluster_estimated_bytes",
            static_cast<double>(tuned.estimatedActiveClusterBytes));
    }

    if (effectiveTreeCacheSizeBytes <= 0)
    {
        tree->SetCacheSize(0);
        return;
    }

    if (resetCache)
        tree->SetCacheSize(0);
    tree->SetCacheSize(effectiveTreeCacheSizeBytes);
    const Long64_t cacheBegin = currentEntry >= 0 ? currentEntry : 0;
    const Long64_t cacheEnd = fChain ? fChain->GetEntries() : tree->GetEntries();
    tree->SetCacheEntryRange(cacheBegin, cacheEnd);
    tree->SetCacheLearnEntries(treeCacheLearnEntries >= 0 ? treeCacheLearnEntries : -1);

    if (auto *file = tree->GetCurrentFile())
    {
        if (auto *cache = tree->GetReadCache(file, true))
        {
            cache->SetBufferSize(effectiveTreeCacheSizeBytes);
            cache->SetLearnPrefill(TTreeCache::kNoPrefill);
            if (resetCache)
                cache->ResetCache();
        }
    }

    // Prefetch only branches that were actually activated (recorded by BranchManager)
    if (enableTreePrefetching)
    {
        const auto &activeBranches = branchManager.getActiveBranches();
        auto *branchList = tree->GetListOfBranches();
        const int capacity = branchList ? branchList->GetSize() : -1;
        for (const auto &name : activeBranches)
        {
            auto *branch = tree->GetBranch(name.c_str());
            if (!branch)
                continue;
            const int id = static_cast<int>(branch->GetUniqueID());
            if (capacity >= 0 && id >= capacity)
                continue; // skip pathological IDs that would overflow TObjArray
            tree->AddBranchToCache(name.c_str(), true);
        }
        if (hasExecutionPlan) {
            for (const auto &column : executionPlan.columns()) {
                if (tree->GetBranch(column.name.c_str()))
                    tree->AddBranchToCache(column.name.c_str(), true);
            }
        }
        if (auto *file = tree->GetCurrentFile())
        {
            if (auto *cache = tree->GetReadCache(file, false))
                cache->StopLearningPhase();
        }
    }
}

void SKNanoLoader::ValidateExecutionPlanTree(TTree *tree) const
{
    if (!hasExecutionPlan || !tree)
        return;
    const TFile *file = tree->GetCurrentFile();
    const std::string context =
        std::string(file ? file->GetName() : "<memory>") + ":" +
        tree->GetName();
    for (const auto &column : executionPlan.columns()) {
        TBranch *branch = tree->GetBranch(column.name.c_str());
        if (!branch) {
            if (column.requirement == SKNano::PlanRequirement::Required)
                throw SKNano::ConfigError(
                    "[ExecutionPlan] required branch '" + column.name +
                    "' is missing in " + context);
            continue;
        }
        TLeaf *leaf = branch->GetLeaf(column.name.c_str());
        if (!leaf)
            throw SKNano::ConfigError(
                "[ExecutionPlan] branch '" + column.name +
                "' has no matching leaf in " + context);
        if (!column.leafType.empty() && column.leafType != leaf->GetTypeName())
            throw SKNano::ConfigError(
                "[ExecutionPlan] branch '" + column.name + "' has type " +
                leaf->GetTypeName() + ", expected " + column.leafType +
                " in " + context);
        const bool vector = leaf->GetLeafCount() != nullptr;
        if ((column.cardinality == SKNano::PlanCardinality::Scalar &&
             (vector || leaf->GetLenStatic() != 1)) ||
            (column.cardinality == SKNano::PlanCardinality::Vector && !vector))
            throw SKNano::ConfigError(
                "[ExecutionPlan] branch '" + column.name +
                "' has incompatible cardinality in " + context);
    }
}

std::vector<SKNano::ClusterTask>
SKNanoLoader::BuildCurrentTreeClusterTasks(
    std::size_t maximumEventsPerTask) const
{
    TTree *tree = fChain ? fChain->GetTree() : nullptr;
    if (!tree)
        throw SKNano::LogicError(
            "[SKNanoLoader] no current tree for cluster planning");
    std::vector<SKNano::ClusterRange> ranges;
    auto iterator = tree->GetClusterIterator(0);
    const Long64_t entries = tree->GetEntries();
    for (Long64_t begin = iterator(); begin >= 0 && begin < entries;
         begin = iterator()) {
        const Long64_t end = std::min(iterator.GetNextEntry(), entries);
        if (end <= begin)
            break;
        ranges.push_back({begin, end});
    }
    const auto fileToken = static_cast<std::uint64_t>(
        fChain->GetTreeNumber() >= 0 ? fChain->GetTreeNumber() : 0);
    return SKNano::ClusterTaskPlanner::plan(fileToken, ranges,
                                            maximumEventsPerTask);
}

void SKNanoLoader::Loop()
{
    if (!fChain)
        return;
    if (eventBlockMode) {
        LoopEventBlocks();
        return;
    }

    Long64_t totalEntries = fChain->GetEntries();
    Long64_t targetEntries = totalEntries;
    if (MaxEvent > 0)
        targetEntries = std::min(targetEntries, static_cast<Long64_t>(MaxEvent));

    Long64_t skipEntries = std::clamp<Long64_t>(NSkipEvent, 0, targetEntries);
    Long64_t entriesToProcess = targetEntries - skipEntries;

    if (entriesToProcess <= 0)
    {
        cout << "[SKNanoLoader::Loop] Nothing to process after applying skip/max settings" << endl;
        return;
    }

    auto startTime = std::chrono::steady_clock::now();
    cout << "[SKNanoLoader::Loop] Event Loop Started" << endl;
    performanceStartBytesRead = TFile::GetFileBytesRead();
    performanceStartReadCalls = TFile::GetFileReadCalls();
    performanceEventsProcessed = 0;
    performanceTelemetry.setMetadata("analyzer", analyzerName);
    performanceTelemetry.setMetadata("era", DataEra.Data());
    performanceTelemetry.startRun();

    currentTreeNumber = -1;
    currentEntry = -1;
    currentLocalEntry = -1;
    branchManager.bindEntrySource(&currentLocalEntry);
    branchManager.bindEpochSource(&eventEpoch);
    cachePrefetchConfigured = false;
    cachePrefetchWarmupEntries = 0;

    Long64_t processed = 0;

    int next_log_percent = 0;
    const int log_step_percent = 5;                        // 5% 진행될 때마다 로깅
    auto last_log_time = startTime;
    const auto log_step_time = std::chrono::seconds(60);   // 또는 60초가 지날 때마다 로깅

    for (Long64_t globalEntry = 0; globalEntry < targetEntries; ++globalEntry)
    {
        {
            auto headerPhase = performanceTelemetry.measure("header_io");
            Long64_t localEntry = fChain->LoadTree(globalEntry);
            if (localEntry < 0)
                break;

            currentEntry = globalEntry;
            currentLocalEntry = localEntry;
            ++eventEpoch;

            if (fChain->GetTreeNumber() != currentTreeNumber)
            {
                currentTreeNumber = fChain->GetTreeNumber();
                TTree *currentTree = fChain->GetTree();
                ValidateExecutionPlanTree(currentTree);
                branchManager.attachTree(currentTree);
                ResetBranchStates();
                cachePrefetchConfigured = false;
                cachePrefetchWarmupEntries = 0;
                if (hasExecutionPlan && enableTreePrefetching) {
                    configureTreeCache(currentTree, true);
                    cachePrefetchConfigured = true;
                }
            }
        }

        if (globalEntry < skipEntries)
            continue;

        int current_percent = (entriesToProcess > 0) ? (processed * 100) / entriesToProcess : 100;

        if (current_percent >= next_log_percent || (processed % 1000 == 0))
        {
            auto currentTime = std::chrono::steady_clock::now();
            
            if (current_percent >= next_log_percent || (currentTime - last_log_time) >= log_step_time)
            {
                std::chrono::duration<double> elapsedTime = currentTime - startTime;
                double timePerEvent = processed > 0 ? elapsedTime.count() / static_cast<double>(processed) : 0.0;
                Long64_t remainingEvents = std::max(entriesToProcess - processed, 0LL);
                double estimatedRemaining = remainingEvents * timePerEvent;

                cout << "[SKNanoLoader::Loop] Progress: " << std::setw(3) << current_percent << "% "
                     << "(" << (skipEntries + processed) << " / " << targetEntries << ") "
                     << "| Elapsed: " << std::fixed << std::setprecision(1) << elapsedTime.count() << "s "
                     << "| ETA: " << std::fixed << std::setprecision(1) << estimatedRemaining << "s" << endl;

                next_log_percent = ((current_percent / log_step_percent) + 1) * log_step_percent;
                last_log_time = currentTime; // 마지막 로그 시간 갱신
            }
        }

        bool skipCurrentEvent = false;
        try
        {
            if (eventArena)
                eventArena->reset();
            auto eventPhase = performanceTelemetry.measure("event_total");
            executeEvent();
        }
        catch (const SKNano::AnalysisException &e)
        {
            const auto context = BuildFailureContext();
            RecordFailure(context, SKNano::ErrorCategoryName(e.category()),
                          e.what(), "SKNano::AnalysisException");
            const bool tooManyErrors =
                maxEventErrors >= 0 && eventErrorCount > maxEventErrors;
            if (failurePolicy == SKNano::FailurePolicy::FailFast ||
                !e.eventLocal() || tooManyErrors)
            {
                WriteFailureSummary();
                WritePerformanceSummary();
                throw;
            }
            skipCurrentEvent = true;
        }
        catch (const std::exception &e)
        {
            const auto context = BuildFailureContext();
            RecordFailure(context, "Unknown", e.what(), "std::exception");
            WriteFailureSummary();
            WritePerformanceSummary();
            throw;
        }
        catch (...)
        {
            const auto context = BuildFailureContext();
            RecordFailure(context, "Unknown", "non-std exception", "unknown");
            WriteFailureSummary();
            WritePerformanceSummary();
            throw;
        }
        ++processed;
        performanceEventsProcessed = processed;
        ++cachePrefetchWarmupEntries;

        if (!cachePrefetchConfigured &&
            enableTreePrefetching &&
            cachePrefetchWarmupEntries >= CACHE_PREFETCH_WARMUP_EVENTS)
        {
            configureTreeCache(fChain, true);
            cachePrefetchConfigured = true;
        }
        if (processed >= entriesToProcess)
            break;
        if (skipCurrentEvent)
            continue;
    }

    auto endTime = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsedTime = endTime - startTime;
    
    cout << "[SKNanoLoader::Loop] Progress: 100% (" << targetEntries << " / " << targetEntries << ") "
         << "| Event Loop Finished in " << std::fixed << std::setprecision(2) << elapsedTime.count() << "s" << endl;
    WriteFailureSummary();
    WritePerformanceSummary();
}

void SKNanoLoader::LoopEventBlocks()
{
    if (!fChain)
        return;
    if (!SupportsOwnedEventBlocks())
        throw SKNano::ConfigError(
            "[SKNanoLoader] EventBlock mode was enabled for an analyzer that "
            "does not implement the owned-block contract");
    if (failurePolicy != SKNano::FailurePolicy::FailFast)
        throw SKNano::ConfigError(
            "[SKNanoLoader] EventBlock mode currently requires FailFast "
            "because a block exception cannot be attributed to one event");

    const Long64_t totalEntries = fChain->GetEntries();
    Long64_t targetEntries = totalEntries;
    if (MaxEvent > 0)
        targetEntries = std::min(targetEntries, static_cast<Long64_t>(MaxEvent));
    const Long64_t skipEntries =
        std::clamp<Long64_t>(NSkipEvent, 0, targetEntries);
    if (targetEntries <= skipEntries)
        return;

    const auto startTime = std::chrono::steady_clock::now();
    cout << "[SKNanoLoader::LoopEventBlocks] EventBlock loop started" << endl;
    performanceStartBytesRead = TFile::GetFileBytesRead();
    performanceStartReadCalls = TFile::GetFileReadCalls();
    performanceEventsProcessed = 0;
    performanceTelemetry.setMetadata("analyzer", analyzerName);
    performanceTelemetry.setMetadata("era", DataEra.Data());
    performanceTelemetry.setMetadata("event_mode", "owned_block");
    performanceTelemetry.startRun();

    currentTreeNumber = -1;
    currentEntry = -1;
    currentLocalEntry = -1;
    branchManager.bindEntrySource(&currentLocalEntry);
    branchManager.bindEpochSource(&eventEpoch);
    cachePrefetchConfigured = false;
    cachePrefetchWarmupEntries = 0;

    SKNano::EventBlock block(eventBlockMemoryBudgetBytes,
                             eventBlockMaximumEvents);
    ConfigureEventBlock(block);
    Long64_t globalEntry = skipEntries;
    std::size_t blockHighWatermarkBytes = 0;

    auto attachCurrentTree = [&] {
        if (fChain->GetTreeNumber() == currentTreeNumber)
            return;
        currentTreeNumber = fChain->GetTreeNumber();
        TTree *currentTree = fChain->GetTree();
        ValidateExecutionPlanTree(currentTree);
        branchManager.attachTree(currentTree);
        ResetBranchStates();
        cachePrefetchConfigured = false;
        cachePrefetchWarmupEntries = 0;
        if (hasExecutionPlan && enableTreePrefetching) {
            configureTreeCache(currentTree, true);
            cachePrefetchConfigured = true;
        }
    };

    try {
        while (globalEntry < targetEntries) {
            {
                auto headerPhase = performanceTelemetry.measure("header_io");
                currentLocalEntry = fChain->LoadTree(globalEntry);
                if (currentLocalEntry < 0)
                    break;
                currentEntry = globalEntry;
                ++eventEpoch;
                attachCurrentTree();
            }

            TTree *tree = fChain->GetTree();
            const Long64_t treeGlobalBegin = globalEntry - currentLocalEntry;
            const Long64_t treeGlobalEnd = std::min(
                targetEntries, treeGlobalBegin + tree->GetEntries());
            const std::uint64_t treeToken = static_cast<std::uint64_t>(
                currentTreeNumber < 0 ? 0 : currentTreeNumber);
            block.begin(treeToken, globalEntry);

            while (globalEntry < treeGlobalEnd &&
                   block.eventCount() < eventBlockMaximumEvents) {
                if (currentEntry != globalEntry) {
                    auto headerPhase = performanceTelemetry.measure("header_io");
                    currentLocalEntry = fChain->LoadTree(globalEntry);
                    if (currentLocalEntry < 0)
                        break;
                    currentEntry = globalEntry;
                    ++eventEpoch;
                    attachCurrentTree();
                }

                const bool supported = CanGatherCurrentEventIntoBlock();
                const std::size_t payload = supported
                    ? CurrentEventBlockPayloadBytes() : 0;
                if (!supported || !block.beginEvent(treeToken, globalEntry,
                                                    payload)) {
                    if (block.eventCount() != 0)
                        break;
                    if (eventArena)
                        eventArena->reset();
                    auto eventPhase = performanceTelemetry.measure("event_total");
                    executeEvent();
                    performanceTelemetry.addCounter("event_block_fallback_events");
                    ++globalEntry;
                    ++performanceEventsProcessed;
                    break;
                }

                {
                    auto gatherPhase =
                        performanceTelemetry.measure("event_block_gather");
                    AppendCurrentEventToBlock(block);
                    block.finishEvent(treeToken, globalEntry);
                }
                performanceTelemetry.addCounter("event_block_copied_bytes",
                                                static_cast<double>(payload));
                ++globalEntry;
            }

            if (block.eventCount() != 0) {
                const std::size_t eventsInBlock = block.eventCount();
                const std::size_t bytesInBlock = block.bytesUsed();
                block.seal();
                {
                    if (eventArena)
                        eventArena->reset();
                    auto eventPhase =
                        performanceTelemetry.measure("event_total");
                    auto blockPhase =
                        performanceTelemetry.measure("event_block_execute");
                    executeEventBlock(block);
                }
                performanceEventsProcessed +=
                    static_cast<Long64_t>(eventsInBlock);
                performanceTelemetry.addCounter("event_blocks");
                blockHighWatermarkBytes =
                    std::max(blockHighWatermarkBytes, bytesInBlock);
            }
            currentEntry = -1; // force LoadTree/epoch refresh on next block
        }
    } catch (const SKNano::AnalysisException &error) {
        RecordFailure(BuildFailureContext(),
                      SKNano::ErrorCategoryName(error.category()), error.what(),
                      "SKNano::AnalysisException");
        WriteFailureSummary();
        WritePerformanceSummary();
        throw;
    } catch (const std::exception &error) {
        RecordFailure(BuildFailureContext(), "Unknown", error.what(),
                      "std::exception");
        WriteFailureSummary();
        WritePerformanceSummary();
        throw;
    } catch (...) {
        RecordFailure(BuildFailureContext(), "Unknown", "non-std exception",
                      "unknown");
        WriteFailureSummary();
        WritePerformanceSummary();
        throw;
    }

    const auto elapsed = std::chrono::duration<double>(
        std::chrono::steady_clock::now() - startTime);
    cout << "[SKNanoLoader::LoopEventBlocks] EventBlock loop finished in "
         << std::fixed << std::setprecision(2) << elapsed.count() << "s" << endl;
    performanceTelemetry.setCounter(
        "event_block_high_watermark_bytes",
        static_cast<double>(blockHighWatermarkBytes));
    WriteFailureSummary();
    WritePerformanceSummary();
}

void SKNanoLoader::WritePerformanceSummary()
{
    if (!performanceTelemetry.enabled())
        return;
    performanceTelemetry.setCounter("events_processed", performanceEventsProcessed);
    performanceTelemetry.setCounter("event_errors", eventErrorCount);
    performanceTelemetry.setCounter("active_branches", branchManager.getActiveBranches().size());
    performanceTelemetry.setCounter("file_bytes_read",
        TFile::GetFileBytesRead() - performanceStartBytesRead);
    performanceTelemetry.setCounter("file_read_calls",
        TFile::GetFileReadCalls() - performanceStartReadCalls);
    performanceTelemetry.setCounter("tree_total_bytes",
        fChain ? static_cast<double>(fChain->GetTotBytes()) : 0.);
    performanceTelemetry.setCounter("tree_zip_bytes",
        fChain ? static_cast<double>(fChain->GetZipBytes()) : 0.);
    performanceTelemetry.setCounter("tree_cache_bytes",
                                    effectiveTreeCacheSizeBytes);
    performanceTelemetry.writeJson();
}

void SKNanoLoader::Init()
{
    cout << "[SKNanoLoader::Init] Initializing. Era = " << DataEra << " Run =  " << Run << endl;
    if (const char *report = std::getenv("SKNANO_PERFORMANCE_REPORT"))
        SetPerformanceReportPath(report);
    if (fChain->GetEntries() == 0)
    {
        cout << "[SKNanoLoader::Init] No Entries in the Tree" << endl;
        throw SKNano::ConfigError("[SKNanoLoader::Init] No entries in the tree");
    }

    fChain->SetBranchStatus("*", 0);
    branchManager.clear();
    branchManager.bindEntrySource(&currentLocalEntry);
    branchManager.bindEpochSource(&eventEpoch);
    RegisterBranches();
    ResetBranchStates();

    // Attach the first concrete tree before analyzer initialization so typed
    // required/optional handles can validate the real leaf schema up front.
    if (fChain->LoadTree(0) < 0 || !fChain->GetTree()) {
        throw SKNano::ConfigError(
            "[SKNanoLoader::Init] Failed to load the first input tree");
    }
    branchManager.attachTree(fChain->GetTree());
    ValidateExecutionPlanTree(fChain->GetTree());
    ResetBranchStates();

    TriggerMap.clear();

    string json_path = string(getenv("SKNANO_DATA")) + "/" + DataEra.Data() + "/Trigger/HLT_Path.json";
    ifstream json_file(json_path);
    if (json_file.is_open())
    {
        cout << "[SKNanoLoader::Init] Loading HLT Paths in " << json_path << endl;
        json j;
        json_file >> j;
        for (auto &[key, value] : j.items())
        {
            auto info = std::make_unique<TriggerInfo>();
            info->lumi = value["lumi"];

            if (key == "Full")
            {
                info->alwaysTrue = true;
                TriggerMap.emplace(key, std::move(info));
                continue;
            }

            if (!fChain->GetBranch(key.c_str()))
            {
                cout << "[SKNanoLoader::Init] " << key << " branch not in tree – skipped" << endl;
                continue;
            }

            info->hlt = &branchManager.getOrCreateScalar<Bool_t>(key);
            TriggerMap.emplace(key, std::move(info));
        }
    }
    else
    {
        cerr << "[SKNanoLoader::Init] Cannot open " << json_path << endl;
    }
}

bool SKNanoLoader::lookupTrigger(const std::string &name,
                                  SKNano::TriggerDecision &decision) const
{
    if (triggerDecisionCacheEntry != currentEntry) {
        triggerDecisionCache.clear();
        triggerDecisionCacheEntry = currentEntry;
    }
    const auto cached = triggerDecisionCache.find(name);
    if (cached != triggerDecisionCache.end()) {
        decision = cached->second;
        return true;
    }

    const auto it = TriggerMap.find(TString(name));
    if (it == TriggerMap.end() || !it->second)
        return false;

    const TriggerInfo &info = *it->second;
    decision.lumi = info.lumi;
    decision.pass = info.alwaysTrue ||
                    (info.hlt && info.hlt->valid() &&
                     static_cast<bool>(*info.hlt));
    triggerDecisionCache.emplace(name, decision);
    return true;
}


void SKNanoLoader::RegisterBranches()
{
#include <generated_branch_register.inc>
}

void SKNanoLoader::ResetBranchStates()
{
    branchManager.resetAll();
#include <generated_branch_reset.inc>
}

SKNano::FailureContext SKNanoLoader::BuildFailureContext() const
{
    SKNano::FailureContext context;
    context.entry = currentEntry;
    context.localEntry = currentLocalEntry;
    context.treeNumber = currentTreeNumber;
    context.analyzer = analyzerName;
    context.sample = MCSample.Data();
    context.dataStream = DataStream.Data();
    context.era = DataEra.Data();
    context.period = DataPeriod.Data();
    context.campaign = Campaign.Data();
    context.systematic = CurrentSystematicName();

    if (fChain)
    {
        if (auto *file = fChain->GetCurrentFile())
            context.inputFile = file->GetName();
    }

    tryReadBranch(RunNumber, context.run);
    tryReadBranch(luminosityBlock, context.lumi);
    tryReadBranch(event, context.event);
    return context;
}

void SKNanoLoader::RecordFailure(const SKNano::FailureContext &context,
                                 const std::string &category,
                                 const std::string &message,
                                 const std::string &exceptionType)
{
    ++eventErrorCount;
    ++errorCountsByCategory[category];

    json record = {
        {"category", category},
        {"exception_type", exceptionType},
        {"message", message},
        {"entry", context.entry},
        {"local_entry", context.localEntry},
        {"tree_number", context.treeNumber},
        {"input_file", context.inputFile},
        {"run", context.run},
        {"lumi", context.lumi},
        {"event", context.event},
        {"analyzer", context.analyzer},
        {"sample", context.sample},
        {"data_stream", context.dataStream},
        {"era", context.era},
        {"period", context.period},
        {"campaign", context.campaign},
        {"systematic", context.systematic},
        {"failure_policy", SKNano::FailurePolicyName(failurePolicy)},
        {"event_error_count", eventErrorCount},
        {"max_event_errors", maxEventErrors},
    };

    cerr << "[SKNanoLoader::Loop] Event failure: " << record.dump() << endl;

    if (errorReportPath.empty())
        return;
    std::ofstream out(errorReportPath, std::ios::app);
    if (!out)
    {
        cerr << "[SKNanoLoader::RecordFailure] Cannot open error report "
             << errorReportPath << endl;
        return;
    }
    out << record.dump() << '\n';
}

void SKNanoLoader::WriteFailureSummary() const
{
    if (errorReportPath.empty())
        return;

    json byCategory = json::object();
    for (const auto &kv : errorCountsByCategory)
        byCategory[kv.first] = kv.second;

    json summary = {
        {"failure_policy", SKNano::FailurePolicyName(failurePolicy)},
        {"event_error_count", eventErrorCount},
        {"max_event_errors", maxEventErrors},
        {"errors_by_category", byCategory},
        {"analyzer", analyzerName},
        {"sample", MCSample.Data()},
        {"data_stream", DataStream.Data()},
        {"era", DataEra.Data()},
        {"period", DataPeriod.Data()},
        {"campaign", Campaign.Data()},
    };

    const std::string summaryPath = summaryPathFor(errorReportPath);
    std::ofstream out(summaryPath);
    if (!out)
    {
        cerr << "[SKNanoLoader::WriteFailureSummary] Cannot open error summary "
             << summaryPath << endl;
        return;
    }
    out << summary.dump(2) << '\n';
}

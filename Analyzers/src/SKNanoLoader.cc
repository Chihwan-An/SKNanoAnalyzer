#include "SKNanoLoader.h"
#include "TTreeCache.h"
#include "TBranch.h"
#include "TObjArray.h"
#include "TIterator.h"
#include <algorithm>
using json = nlohmann::json;

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
}

void SKNanoLoader::configureTreeCache(TTree *tree, bool resetCache)
{
    if (!tree)
        return;

    if (treeCacheSizeBytes <= 0)
    {
        tree->SetCacheSize(0);
        return;
    }

    if (resetCache)
        tree->SetCacheSize(0);
    tree->SetCacheSize(treeCacheSizeBytes);
    const Long64_t cacheBegin = currentEntry >= 0 ? currentEntry : 0;
    const Long64_t cacheEnd = fChain ? fChain->GetEntries() : tree->GetEntries();
    tree->SetCacheEntryRange(cacheBegin, cacheEnd);
    tree->SetCacheLearnEntries(treeCacheLearnEntries >= 0 ? treeCacheLearnEntries : -1);

    if (auto *file = tree->GetCurrentFile())
    {
        if (auto *cache = tree->GetReadCache(file, true))
        {
            cache->SetBufferSize(treeCacheSizeBytes);
            cache->SetLearnPrefill(TTreeCache::kNoPrefill);
            if (resetCache)
                cache->ResetCache();
        }
    }

    // Prefetch only branches that were actually activated (recorded by BranchManager)
    if (enableTreePrefetching)
    {
        const auto &activeBranches = BranchBase::GetActiveBranches();
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
        if (auto *file = tree->GetCurrentFile())
        {
            if (auto *cache = tree->GetReadCache(file, false))
                cache->StopLearningPhase();
        }
    }
}

void SKNanoLoader::Loop()
{
    if (!fChain)
        return;

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

    currentTreeNumber = -1;
    currentEntry = -1;
    currentLocalEntry = -1;
    branchManager.bindEntrySource(&currentLocalEntry);
    cachePrefetchConfigured = false;
    cachePrefetchWarmupEntries = 0;

    Long64_t processed = 0;

    int next_log_percent = 0;
    const int log_step_percent = 5;                        // 5% 진행될 때마다 로깅
    auto last_log_time = startTime;
    const auto log_step_time = std::chrono::seconds(60);   // 또는 60초가 지날 때마다 로깅

    for (Long64_t globalEntry = 0; globalEntry < targetEntries; ++globalEntry)
    {
        Long64_t localEntry = fChain->LoadTree(globalEntry);
        if (localEntry < 0)
            break;

        currentEntry = globalEntry;
        currentLocalEntry = localEntry;

        if (fChain->GetTreeNumber() != currentTreeNumber)
        {
            currentTreeNumber = fChain->GetTreeNumber();
            TTree *currentTree = fChain->GetTree();
            branchManager.attachTree(currentTree);
            ResetBranchStates();
            cachePrefetchConfigured = false;
            cachePrefetchWarmupEntries = 0;
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

        executeEvent();
        ++processed;
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
    }

    auto endTime = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsedTime = endTime - startTime;
    
    cout << "[SKNanoLoader::Loop] Progress: 100% (" << targetEntries << " / " << targetEntries << ") "
         << "| Event Loop Finished in " << std::fixed << std::setprecision(2) << elapsedTime.count() << "s" << endl;
}

void SKNanoLoader::Init()
{
    cout << "[SKNanoLoader::Init] Initializing. Era = " << DataEra << " Run =  " << Run << endl;
    if (fChain->GetEntries() == 0)
    {
        cout << "[SKNanoLoader::Init] No Entries in the Tree" << endl;
        cout << "[SKNanoLoader::Init] Exiting without make output..." << endl;
        exit(0);
    }

    fChain->SetBranchStatus("*", 0);
    BranchBase::ClearActiveBranches();

    branchManager.clear();
    branchManager.bindEntrySource(&currentLocalEntry);
    RegisterBranches();
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

            auto hltBranch = std::make_unique<BranchScalar<Bool_t>>(key.c_str());
            branchManager.registerScalar(*hltBranch);
            info->hlt = std::move(hltBranch);
            TriggerMap.emplace(key, std::move(info));
        }
    }
    else
    {
        cerr << "[SKNanoLoader::Init] Cannot open " << json_path << endl;
    }
}


void SKNanoLoader::RegisterBranches()
{
#include "../include/generated_branch_register.inc"
}

void SKNanoLoader::ResetBranchStates()
{
    branchManager.resetAll();
#include "../include/generated_branch_reset.inc"
}

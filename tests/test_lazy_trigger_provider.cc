#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "SKNanoLoader.h"
#include "TFile.h"
#include "TTree.h"

namespace fs = std::filesystem;

namespace {

struct ObservedDecision {
    Long64_t entry = -1;
    std::string name;
    bool found = false;
    bool pass = false;
    float lumi = 0.f;
};

class TriggerProbeLoader final : public SKNanoLoader {
public:
    std::vector<std::string> requestedTriggers;
    std::vector<ObservedDecision> observed;

    void executeEvent() override {
        for (const auto &name : requestedTriggers) {
            SKNano::TriggerDecision decision;
            const bool found = lookupTrigger(name, decision);
            observed.push_back({CurrentEntry(), name, found, decision.pass, decision.lumi});

            // A second lookup in the same event must reuse the entry-local
            // provider cache without changing its result.
            SKNano::TriggerDecision cached;
            if (found) {
                const bool cachedFound = lookupTrigger(name, cached);
                if (!cachedFound || cached.pass != decision.pass || cached.lumi != decision.lumi)
                    throw std::runtime_error("trigger decision cache changed within one event");
            }
        }
    }
};

class BlockProbeLoader final : public SKNanoLoader {
public:
    std::vector<std::pair<Long64_t, bool>> observed;
    std::vector<std::uint64_t> arenaEpochs;
    std::size_t executedBlocks = 0;

    void executeEvent() override {
        auto scratch = GetEventArena().allocateArray<int>(1);
        scratch[0] = 1;
        arenaEpochs.push_back(GetEventArena().epoch());
        observed.emplace_back(CurrentEntry(), static_cast<bool>(triggerA->get()));
    }

protected:
    bool SupportsOwnedEventBlocks() const noexcept override { return true; }
    void ConfigureEventBlock(SKNano::EventBlock &block) override {
        triggerA.emplace(GetScalarHandle<Bool_t>("HLT_A"));
        block.registerColumn<std::uint8_t>(SKNano::ColumnId(0));
    }
    bool CanGatherCurrentEventIntoBlock() const override {
        // Exercise the required D2a fallback in the middle of the stream.
        return CurrentEntry() != 2;
    }
    std::size_t CurrentEventBlockPayloadBytes() const override {
        return sizeof(std::uint8_t);
    }
    void AppendCurrentEventToBlock(SKNano::EventBlock &block) override {
        const std::uint8_t value = static_cast<std::uint8_t>(triggerA->get());
        block.append(SKNano::ColumnId(0), &value, 1);
    }
    void executeEventBlock(const SKNano::EventBlock &block) override {
        auto scratch = GetEventArena().allocateArray<int>(block.eventCount());
        arenaEpochs.push_back(GetEventArena().epoch());
        ++executedBlocks;
        for (std::size_t index = 0; index < block.eventCount(); ++index) {
            scratch[index] = static_cast<int>(index);
            observed.emplace_back(
                block.firstEntry() + static_cast<Long64_t>(index),
                block.event<std::uint8_t>(SKNano::ColumnId(0), index)[0] != 0);
        }
    }

private:
    std::optional<ScalarHandle<Bool_t>> triggerA;
};

void require(bool condition, const std::string &message) {
    if (!condition)
        throw std::runtime_error(message);
}

void writeTriggerFile(const fs::path &path, bool includeB, bool hltA, bool hltB) {
    TFile file(path.c_str(), "RECREATE");
    if (file.IsZombie())
        throw std::runtime_error("cannot create " + path.string());
    TTree tree("Events", "Events");
    Bool_t branchA = hltA;
    tree.Branch("HLT_A", &branchA, "HLT_A/O");
    if (includeB) {
        Bool_t branchB = hltB;
        tree.Branch("HLT_B", &branchB, "HLT_B/O");
    }
    tree.Fill();
    tree.Write();
}

void writeBlockFile(const fs::path &path) {
    TFile file(path.c_str(), "RECREATE");
    TTree tree("Events", "Events");
    Bool_t branchA = false;
    tree.Branch("HLT_A", &branchA, "HLT_A/O");
    for (int entry = 0; entry < 5; ++entry) {
        branchA = entry % 2 == 0;
        tree.Fill();
    }
    tree.Write();
}

void writeTriggerMetadata(const fs::path &dataDirectory) {
    const fs::path metadata = dataDirectory / "2024" / "Trigger" / "HLT_Path.json";
    fs::create_directories(metadata.parent_path());
    std::ofstream output(metadata);
    output << R"({
  "Full": {"lumi": 100.0},
  "HLT_A": {"lumi": 3.5},
  "HLT_B": {"lumi": 7.0}
})";
}

std::vector<ObservedDecision> runProbe(const fs::path &dataDirectory,
                                       const fs::path &firstFile,
                                       const fs::path &secondFile,
                                       std::vector<std::string> requested,
                                       std::vector<std::string> &activeBranches) {
    if (setenv("SKNANO_DATA", dataDirectory.c_str(), 1) != 0)
        throw std::runtime_error("cannot set SKNANO_DATA for trigger test");

    TriggerProbeLoader loader;
    loader.SetInputFormat("ttree");
    loader.SetTreeName("Events");
    loader.SetEra("2024");
    SKNano::ExecutionPlanBuilder planBuilder;
    planBuilder.column("HLT_A", SKNano::PlanRequirement::Required, "Bool_t",
                       SKNano::PlanCardinality::Scalar);
    planBuilder.column("HLT_B", SKNano::PlanRequirement::Optional, "Bool_t",
                       SKNano::PlanCardinality::Scalar);
    loader.SetExecutionPlan(planBuilder.compile());
    const fs::path performanceReport =
        dataDirectory.parent_path() / "trigger-performance.json";
    loader.SetPerformanceReportPath(performanceReport.string());
    loader.requestedTriggers = std::move(requested);
    require(loader.AddFile(firstFile.c_str()) == 1, "cannot add first trigger fixture");
    require(loader.AddFile(secondFile.c_str()) == 1, "cannot add second trigger fixture");
    loader.Init();
    loader.Loop();
    const auto clusterTasks = loader.BuildCurrentTreeClusterTasks(1);
    require(clusterTasks.size() == 1 && clusterTasks[0].begin == 0 &&
                clusterTasks[0].end == 1,
            "ROOT cluster planner must cover the current tree without overlap");

    std::ifstream performanceInput(performanceReport);
    const std::string performanceJson(
        (std::istreambuf_iterator<char>(performanceInput)),
        std::istreambuf_iterator<char>());
    require(performanceJson.find("\"events_processed\": 2") !=
                std::string::npos &&
            performanceJson.find("\"header_io\"") != std::string::npos &&
            performanceJson.find("\"event_total\"") != std::string::npos &&
            performanceJson.find("\"file_bytes_read\"") !=
                std::string::npos,
            "loader telemetry must report event, phase, and ROOT I/O metrics");

    const auto &managerActiveBranches = loader.branchManager.getActiveBranches();
    activeBranches.assign(managerActiveBranches.begin(),
                          managerActiveBranches.end());
    std::sort(activeBranches.begin(), activeBranches.end());
    return loader.observed;
}

bool contains(const std::vector<std::string> &values, const std::string &needle) {
    return std::find(values.begin(), values.end(), needle) != values.end();
}

void testZeroOneAndManyRequestedTriggers() {
    const fs::path directory = fs::temp_directory_path() / "sknano_lazy_trigger_provider";
    fs::remove_all(directory);
    fs::create_directories(directory);
    try {
        const fs::path firstFile = directory / "first.root";
        const fs::path secondFile = directory / "second.root";
        writeTriggerFile(firstFile, true, true, false);
        writeTriggerFile(secondFile, false, false, false);
        writeTriggerMetadata(directory / "data");

        std::vector<std::string> active;
        const auto none = runProbe(directory / "data", firstFile, secondFile, {}, active);
        require(none.empty(), "no trigger requests must produce no decisions");
        require(!contains(active, "HLT_A") && !contains(active, "HLT_B"),
                "no trigger requests must not activate HLT branches");

        const auto one = runProbe(directory / "data", firstFile, secondFile, {"HLT_A"}, active);
        require(one.size() == 2, "one requested trigger must produce one decision per entry");
        require(one[0].found && one[0].pass && one[0].lumi == 3.5f,
                "HLT_A must match the first tree value and lumi");
        require(one[1].found && !one[1].pass && one[1].lumi == 3.5f,
                "HLT_A must rebind and read the second tree value");
        require(contains(active, "HLT_A") && !contains(active, "HLT_B"),
                "one requested trigger must activate only its HLT branch");

        const auto many = runProbe(directory / "data", firstFile, secondFile,
                                   {"Full", "HLT_A", "HLT_B"}, active);
        require(many.size() == 6, "three requested triggers must produce three decisions per entry");
        require(many[0].name == "Full" && many[0].found && many[0].pass && many[0].lumi == 100.f,
                "Full must be an always-true metadata trigger");
        require(many[2].name == "HLT_B" && many[2].found && !many[2].pass && many[2].lumi == 7.f,
                "HLT_B must read the first tree value");
        require(many[5].name == "HLT_B" && many[5].found && !many[5].pass && many[5].lumi == 7.f,
                "a trigger missing from a later tree must be optional and false");
        require(contains(active, "HLT_A") && contains(active, "HLT_B"),
                "many requested triggers must activate only requested physical HLT branches");
    } catch (...) {
        fs::remove_all(directory);
        throw;
    }
    fs::remove_all(directory);
}

void testOwnedEventBlockLoopAndFallback() {
    const fs::path directory =
        fs::temp_directory_path() / "sknano_owned_event_block";
    fs::remove_all(directory);
    fs::create_directories(directory);
    try {
        writeBlockFile(directory / "events.root");
        writeTriggerMetadata(directory / "data");
        require(setenv("SKNANO_DATA", (directory / "data").c_str(), 1) == 0,
                "cannot set block fixture data path");
        BlockProbeLoader loader;
        loader.SetInputFormat("ttree");
        loader.SetTreeName("Events");
        loader.SetEra("2024");
        loader.EnableEventBlockMode();
        loader.EnableEventArena(4096);
        loader.SetEventBlockLimits(4096, 2);
        require(loader.AddFile((directory / "events.root").c_str()) == 1,
                "cannot add block fixture");
        loader.Init();
        loader.Loop();
        require(loader.observed.size() == 5 && loader.executedBlocks == 2,
                "two owned blocks plus one event-local fallback must execute");
        require(loader.arenaEpochs.size() == 3 &&
                    loader.arenaEpochs[0] < loader.arenaEpochs[1] &&
                    loader.arenaEpochs[1] < loader.arenaEpochs[2],
                "EventArena must reset between block/fallback executions");
        for (Long64_t entry = 0; entry < 5; ++entry)
            require(loader.observed[static_cast<std::size_t>(entry)] ==
                        std::make_pair(entry, entry % 2 == 0),
                    "EventBlock loop must preserve event ordering and values");
    } catch (...) {
        fs::remove_all(directory);
        throw;
    }
    fs::remove_all(directory);
}

} // namespace

int main() {
    try {
        testZeroOneAndManyRequestedTriggers();
        testOwnedEventBlockLoopAndFallback();
    } catch (const std::exception &error) {
        std::cerr << "test_lazy_trigger_provider: " << error.what() << '\n';
        return 1;
    }
    return 0;
}

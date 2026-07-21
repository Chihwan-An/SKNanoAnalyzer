#include "AnalyzerCore.h"

#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {

void Require(bool condition, const char *message) {
  if (!condition)
    throw std::runtime_error(message);
}

class TaskHarness final : public AnalyzerCore {
public:
  void AddTask(std::string flag, bool enabledByDefault,
               TaskSystematicPolicy systematicPolicy) {
    const std::string key = flag;
    RegisterTask(
        std::move(flag), TaskOptions{enabledByDefault, systematicPolicy},
        [this, key] { ++validated[key]; },
        [this, key] { ++booked[key]; }, [this, key] { ++ran[key]; });
  }

  void Start(std::string_view enableAllFlag = {}) {
    InitializeTasks(enableAllFlag);
  }

  bool HasForSystematic(bool isCentral) const {
    return HasTasksForSystematic(isCentral);
  }

  void Run(bool isCentral) { RunTasks(isCentral); }

  std::map<std::string, int> validated;
  std::map<std::string, int> booked;
  std::map<std::string, int> ran;
};

void TestDefaultTask() {
  TaskHarness harness;
  harness.AddTask("Default", true,
                  AnalyzerCore::TaskSystematicPolicy::CentralOnly);
  harness.AddTask("Optional", false,
                  AnalyzerCore::TaskSystematicPolicy::AllVariations);
  harness.Start("AllTasks");

  Require(harness.validated["Default"] == 1,
          "default task validation did not run");
  Require(harness.booked["Default"] == 1,
          "default task booking did not run");
  Require(harness.validated["Optional"] == 0,
          "optional task was enabled without a flag");
  Require(harness.HasForSystematic(true),
          "central systematic did not find the default task");
  Require(!harness.HasForSystematic(false),
          "central-only default task accepted a variation");

  harness.Run(true);
  harness.Run(false);
  Require(harness.ran["Default"] == 1,
          "central-only task ran for a non-central variation");
}

void TestExplicitTask() {
  TaskHarness harness;
  harness.Userflags = {"Optional"};
  harness.AddTask("Default", true,
                  AnalyzerCore::TaskSystematicPolicy::CentralOnly);
  harness.AddTask("Optional", false,
                  AnalyzerCore::TaskSystematicPolicy::AllVariations);
  harness.Start("AllTasks");

  Require(harness.validated["Default"] == 0,
          "default task remained enabled after explicit selection");
  Require(harness.validated["Optional"] == 1,
          "explicit task validation did not run");
  Require(harness.booked["Optional"] == 1,
          "explicit task booking did not run");
  Require(harness.HasForSystematic(false),
          "all-variations task rejected a variation");

  harness.Run(false);
  harness.Run(true);
  Require(harness.ran["Optional"] == 2,
          "all-variations task did not run in both lanes");
}

void TestEnableAllAndDuplicateGuard() {
  TaskHarness harness;
  harness.Userflags = {"AllTasks"};
  harness.AddTask("First", true,
                  AnalyzerCore::TaskSystematicPolicy::CentralOnly);
  harness.AddTask("Second", false,
                  AnalyzerCore::TaskSystematicPolicy::AllVariations);
  harness.Start("AllTasks");
  Require(harness.validated["First"] == 1 &&
              harness.validated["Second"] == 1,
          "enable-all flag did not select every task");

  TaskHarness duplicate;
  duplicate.AddTask("Repeated", true,
                    AnalyzerCore::TaskSystematicPolicy::CentralOnly);
  bool threw = false;
  try {
    duplicate.AddTask("Repeated", false,
                      AnalyzerCore::TaskSystematicPolicy::AllVariations);
  } catch (const SKNano::ConfigError &) {
    threw = true;
  }
  Require(threw, "duplicate task flag was accepted");
}

} // namespace

int main() {
  try {
    TestDefaultTask();
    TestExplicitTask();
    TestEnableAllAndDuplicateGuard();
  } catch (const std::exception &error) {
    std::cerr << "test_analyzer_task_api: " << error.what() << '\n';
    return 1;
  }
  std::cout << "ANALYZER_TASK_API_OK\n";
  return 0;
}

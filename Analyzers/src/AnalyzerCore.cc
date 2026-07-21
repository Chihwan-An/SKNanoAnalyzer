#include "AnalyzerCore.h"
#include "JetView.h"
#include "TObjArray.h"
#include "TObjString.h"
#include <Compression.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <execution>
#include <fstream>
#include <functional>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <utility>

class AnalyzerTaskRegistryState {
public:
  struct Task {
    std::string flag;
    AnalyzerCore::TaskOptions options;
    AnalyzerCore::TaskHook validate;
    AnalyzerCore::TaskHook book;
    AnalyzerCore::TaskHook run;
  };

  std::vector<Task> tasks;
  std::vector<std::size_t> enabledTaskIndices;
  bool initialized = false;
};

AnalyzerCore::AnalyzerCore() {
  taskRegistryState_ = std::make_unique<AnalyzerTaskRegistryState>();
  myCorr = nullptr;
  outfile = nullptr;
  if (HasFlag("useTH1F")) {
    cout << "[AnalyzerCore::AnalyzerCore] Using TH1F" << endl;
    useTH1F = true;
  } else {
    cout << "[AnalyzerCore::AnalyzerCore] Using TH1D" << endl;
    useTH1F = false;
  }
  histmap1d.reserve(512);
  histmap2d.reserve(256);
  histmap3d.reserve(128);
  // pdfReweight = new PDFReweight();
}
AnalyzerCore::~AnalyzerCore() {
  // RNTuple writers must be destroyed while their backing TFile is alive.
  rntupleOutputs_.clear();
  for (const auto &pair : histmap1d)
    delete pair.second;
  histmap1d.clear();
  for (const auto &pair : histmap2d)
    delete pair.second;
  histmap2d.clear();
  for (const auto &pair : histmap3d)
    delete pair.second;
  histmap3d.clear();
  if (outfile)
    delete outfile;
  if (myCorr)
    delete myCorr;
  // if (pdfReweight) delete pdfReweight;
}

void AnalyzerCore::RegisterTask(std::string flag, TaskOptions options,
                                TaskHook validate, TaskHook book,
                                TaskHook run) {
  if (taskRegistryState_->initialized)
    throw SKNano::LogicError(
        "[AnalyzerCore::RegisterTask] tasks are already initialized");
  if (flag.empty())
    throw SKNano::ConfigError(
        "[AnalyzerCore::RegisterTask] task flag must not be empty");
  if (!run)
    throw SKNano::ConfigError(
        "[AnalyzerCore::RegisterTask] task '" + flag +
        "' does not have a run hook");

  const auto duplicate = std::find_if(
      taskRegistryState_->tasks.begin(), taskRegistryState_->tasks.end(),
      [&flag](const AnalyzerTaskRegistryState::Task &task) {
        return task.flag == flag;
      });
  if (duplicate != taskRegistryState_->tasks.end())
    throw SKNano::ConfigError(
        "[AnalyzerCore::RegisterTask] duplicate task flag '" + flag + "'");

  taskRegistryState_->tasks.push_back(
      {std::move(flag), options, std::move(validate), std::move(book),
       std::move(run)});
}

void AnalyzerCore::InitializeTasks(std::string_view enableAllFlag) {
  auto &state = *taskRegistryState_;
  if (state.initialized)
    throw SKNano::LogicError(
        "[AnalyzerCore::InitializeTasks] tasks are already initialized");
  if (state.tasks.empty())
    throw SKNano::ConfigError(
        "[AnalyzerCore::InitializeTasks] no tasks were registered");
  state.enabledTaskIndices.clear();

  const bool enableAll =
      !enableAllFlag.empty() && HasFlag(std::string(enableAllFlag).c_str());
  bool hasExplicitTask = enableAll;
  for (const auto &task : state.tasks)
    hasExplicitTask = hasExplicitTask || HasFlag(task.flag.c_str());

  for (std::size_t index = 0; index < state.tasks.size(); ++index) {
    const auto &task = state.tasks[index];
    if (enableAll || HasFlag(task.flag.c_str()) ||
        (!hasExplicitTask && task.options.enabledByDefault))
      state.enabledTaskIndices.push_back(index);
  }
  if (state.enabledTaskIndices.empty())
    throw SKNano::ConfigError(
        "[AnalyzerCore::InitializeTasks] no task was selected");

  for (const auto index : state.enabledTaskIndices) {
    auto &task = state.tasks[index];
    if (task.validate)
      task.validate();
  }
  for (const auto index : state.enabledTaskIndices) {
    auto &task = state.tasks[index];
    if (task.book)
      task.book();
  }
  state.initialized = true;
}

bool AnalyzerCore::HasTasksForSystematic(bool isCentralSystematic) const {
  const auto &state = *taskRegistryState_;
  if (!state.initialized)
    throw SKNano::LogicError(
        "[AnalyzerCore::HasTasksForSystematic] tasks are not initialized");
  return std::any_of(
      state.enabledTaskIndices.begin(), state.enabledTaskIndices.end(),
      [&state, isCentralSystematic](std::size_t index) {
        return isCentralSystematic ||
               state.tasks[index].options.systematicPolicy ==
                   TaskSystematicPolicy::AllVariations;
      });
}

void AnalyzerCore::RunTasks(bool isCentralSystematic) {
  auto &state = *taskRegistryState_;
  if (!state.initialized)
    throw SKNano::LogicError(
        "[AnalyzerCore::RunTasks] tasks are not initialized");
  for (const auto index : state.enabledTaskIndices) {
    auto &task = state.tasks[index];
    if (task.options.systematicPolicy ==
            TaskSystematicPolicy::CentralOnly &&
        !isCentralSystematic)
      continue;
    task.run();
  }
}

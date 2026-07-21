# Analyzer task API

[Documentation index](README.md)

`AnalyzerCore` provides a small lifecycle API for analyzers that share one
event selection across several flag-selectable tasks.

## Lifecycle

1. Register every task with `RegisterTask()`.
2. Perform any base-analyzer initialization required by the task hooks.
3. Call `InitializeTasks()` once. It resolves flags, validates inputs, and
   books outputs for enabled tasks.
4. After the shared event selection, use `HasTasksForSystematic()` to skip
   unused systematic lanes and `RunTasks()` to execute eligible tasks.

Task flags are additive. If no registered task flag is present, tasks with
`enabledByDefault=true` are selected. An optional all-tasks flag is passed to
`InitializeTasks()` by each analyzer.

```cpp
void MyAnalyzer::RegisterTasks() {
  RegisterTask(
      "ControlPlots",
      TaskOptions{true, TaskSystematicPolicy::CentralOnly},
      [this] { ValidateControlInputs(); },
      [this] { BookControlPlots(); },
      [this] { FillControlPlots(); });

  RegisterTask(
      "SystematicTrees",
      TaskOptions{false, TaskSystematicPolicy::AllVariations},
      [this] { ValidateTreeInputs(); },
      [this] { BookSystematicTrees(); },
      [this] { FillSystematicTrees(); });
}

void MyAnalyzer::initializeAnalyzer() {
  RegisterTasks();
  BaseAnalyzer::initializeAnalyzer();
  InitializeTasks("MyAnalyzerAll");
}
```

The event loop retains ownership of object loading and the shared selection:

```cpp
const bool isCentral = CurrentSystematicIsCentral();
if (!HasTasksForSystematic(isCentral))
  continue;
if (!PassSharedSelection())
  continue;
RunTasks(isCentral);
```

Validation and booking hooks may be empty, but every task must provide a run
hook. Empty flags, duplicate flags, registration after initialization, and
initialization without a selected task are configuration errors.

The callback registry is held in an opaque, non-persistent `AnalyzerCore`
state so task implementation types do not enter the ROOT dictionary.

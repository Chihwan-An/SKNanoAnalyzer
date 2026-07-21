#include <algorithm>
// Note that this is a template for MC, for data, it will automatically change uncessary lines in SKNano.py

void [jobname]() {
    [analyzer] module;
    module.SetInputFormat("[input_format]");
    module.SetTreeName("Events");
    module.SetAnalyzerName("[analyzer]");
    module.SetOutputThreads([ncpu]);
    module.LogEvery = 5000;
    module.IsDATA = false;
    module.MCSample = "[sample]";
    module.xsec = [xsec];
    module.sumW = [sumW];
    module.sumSign = [sumSign];
    module.SetEra("[era]");
    module.SetPeriod("[period]");
[USERFLAGS]
[SAMPLEPATHS]
[MAXEVENT]
    module.SetOutfilePath("[output]");
    module.SetFailurePolicy("[failure_policy]");
    module.SetMaxEventErrors([max_event_errors]);
    module.Init();
    module.initializeAnalyzer();
    module.Loop();
    module.WriteHist();
}

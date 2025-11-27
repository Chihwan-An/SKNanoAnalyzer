#include <algorithm>
// Note that this is a template for MC, for data, it will automatically change uncessary lines in SKNano.py

void test() {
    Vcb_SL module;
    module.SetTreeName("Events");
    module.LogEvery = 5000;
    module.IsDATA = false;
    module.MCSample = "TTLJ_powheg";
    module.xsec = 405.685352016;
    module.sumW = 51082709405.36572;
    module.sumSign = 151559604.0;
    module.SetEra("2024");
    module.SetPeriod("");
module.Userflags = {
        "Mu",
    };
        module.AddFile("/gv0/Users/yeonjoon/SKNano_Vcb_2FileBased/TTtoLNu2Q_TuneCP5_13p6TeV_powheg-pythia8/crab_MC_2024_TTtoLNu2Q_TuneCP5_13p6TeV_powheg-pythia8/250917_030904/0001/NANOAOD_1000.root");
        module.MaxEvent = std::max(1, static_cast<int>(module.fChain->GetEntries()/1));
    module.SetOutfilePath("test.root");
    module.Init();
    module.initializeAnalyzer();
    module.Loop();
    module.WriteHist();
}

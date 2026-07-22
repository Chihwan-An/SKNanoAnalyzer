#include <MyAnalysis/MyAnalyzer.h>

void MyAnalyzer::initializeAnalyzer() {
    Hists().Book1D("EventCount", 1, 0., 1.);
}

void MyAnalyzer::executeEvent() {
    Hists().Fill("EventCount", 0.5f, IsDATA ? 1.f : MCweight());
}

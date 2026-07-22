#include <AnalyzerFramework/GeneratorBase.h>

GeneratorBase::GeneratorBase() {}
GeneratorBase::~GeneratorBase() {}

void GeneratorBase::initializeAnalyzer() {}

void GeneratorBase::executeEvent() {
    LHEObjects = GetAllLHEViews();
    GenObjects = GetAllGenViews();
    GenJetObjects = GetAllGenJetViews();
    GenDressedLeptonObjects = GetAllGenDressedLeptonViews();
    GenIsolatedPhotonObjects = GetAllGenIsolatedPhotonViews();
    GenVisTauObjects = GetAllGenVisTauViews();

}

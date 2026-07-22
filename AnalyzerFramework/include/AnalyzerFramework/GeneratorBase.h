#ifndef GeneratorBase_h
#define GeneratorBase_h

#include <AnalyzerFramework/AnalyzerCore.h>

class GeneratorBase : public AnalyzerCore {
public:
    void initializeAnalyzer();
    void executeEvent();

    LHEViewCollection LHEObjects;
    GenViewCollection GenObjects;
    GenJetViewCollection GenJetObjects;
    GenDressedLeptonViewCollection GenDressedLeptonObjects;
    GenIsolatedPhotonViewCollection GenIsolatedPhotonObjects;
    GenVisTauViewCollection GenVisTauObjects;

    GeneratorBase();
    ~GeneratorBase();
};

#endif

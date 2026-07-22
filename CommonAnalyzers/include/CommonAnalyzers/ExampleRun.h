#ifndef ExampleRun_h
#define ExampleRun_h

#include <AnalyzerFramework/AnalyzerCore.h>

class ExampleRun : public AnalyzerCore {
public:
    ExampleRun();
    ~ExampleRun();

    void initializeAnalyzer() override;
    void executeEvent() override;

private:
    TString IsoMuTriggerName;
    float TriggerSafePtCut = 0.f;
};

#endif

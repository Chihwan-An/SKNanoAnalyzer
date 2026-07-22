#ifndef MyAnalysis_MyAnalyzer_h
#define MyAnalysis_MyAnalyzer_h

#include <AnalyzerFramework/AnalyzerCore.h>

class MyAnalyzer final : public AnalyzerCore {
public:
    void initializeAnalyzer() override;
    void executeEvent() override;
};

#endif

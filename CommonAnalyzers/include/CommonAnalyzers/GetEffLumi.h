#ifndef GetEffLumi_h 
#define GetEffLumi_h

#include <AnalyzerFramework/AnalyzerCore.h>



class GetEffLumi : public AnalyzerCore {
public:
    void executeEvent();
    GetEffLumi();
    ~GetEffLumi();
    ;
    void initializeAnalyzer();
};

#endif


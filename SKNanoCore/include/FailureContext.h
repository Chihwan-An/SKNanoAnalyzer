#ifndef SKNanoCore_FailureContext_h
#define SKNanoCore_FailureContext_h

#include <string>

namespace SKNano {

struct FailureContext {
    long long entry = -1;
    long long localEntry = -1;
    int treeNumber = -1;
    unsigned long long run = 0;
    unsigned long long lumi = 0;
    unsigned long long event = 0;
    std::string inputFile;
    std::string analyzer;
    std::string sample;
    std::string dataStream;
    std::string era;
    std::string period;
    std::string campaign;
    std::string systematic;
};

} // namespace SKNano

#endif

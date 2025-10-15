#ifndef Triggerinfo_h
#define Triggerinfo_h

#include <memory>
#include <map>

#include "BranchManager.h"
#include "TString.h"

struct TriggerInfo {
    std::unique_ptr<BranchScalar<Bool_t>> hlt;
    float lumi = 0.f;
    bool alwaysTrue = false;
};

using TriggerMap_t =
    std::map<TString, std::unique_ptr<TriggerInfo>>;

#endif // Triggerinfo_h

#ifndef Triggerinfo_h
#define Triggerinfo_h

#include <memory>
#include <map>

#include "BranchManager.h"
#include "TString.h"

struct TriggerInfo {
    // Owned by BranchManager.  This may point at a generated HLT wrapper or
    // a dynamic wrapper, but never creates a second SetAddress for a branch.
    BranchScalar<Bool_t> *hlt = nullptr;
    float lumi = 0.f;
    bool alwaysTrue = false;
};

using TriggerMap_t =
    std::map<TString, std::unique_ptr<TriggerInfo>>;

#endif // Triggerinfo_h

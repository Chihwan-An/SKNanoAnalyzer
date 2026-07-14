#ifndef SKNanoCore_TriggerDecision_h
#define SKNanoCore_TriggerDecision_h

#include <cstdint>
#include <map>
#include <string>

namespace SKNano {

struct TriggerDecision {
    bool pass = false;
    float lumi = 0.f;
};

using TriggerDecisionMap = std::map<std::string, TriggerDecision>;

// Event owns no HLT branch buffers.  The loader provides decisions lazily for
// the current entry; callers that need a persistent snapshot can request one.
class TriggerDecisionProvider {
public:
    virtual ~TriggerDecisionProvider() = default;
    virtual std::uint64_t triggerEpoch() const noexcept = 0;
    virtual bool lookupTrigger(const std::string &name,
                               TriggerDecision &decision) const = 0;
};

} // namespace SKNano

#endif

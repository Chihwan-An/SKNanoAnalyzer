#ifndef SKNanoCore_FailurePolicy_h
#define SKNanoCore_FailurePolicy_h

#include <string>

#include "AnalysisException.h"

namespace SKNano {

enum class FailurePolicy {
    FailFast,
    SkipEvent
};

inline const char *FailurePolicyName(FailurePolicy policy) {
    switch (policy) {
    case FailurePolicy::FailFast:
        return "fail-fast";
    case FailurePolicy::SkipEvent:
        return "skip-event";
    default:
        return "unknown";
    }
}

inline FailurePolicy ParseFailurePolicy(const std::string &policy) {
    if (policy == "fail-fast" || policy == "fail_fast" || policy == "failfast")
        return FailurePolicy::FailFast;
    if (policy == "skip-event" || policy == "skip_event" || policy == "skip")
        return FailurePolicy::SkipEvent;
    throw ConfigError("Unknown failure policy: " + policy);
}

} // namespace SKNano

#endif

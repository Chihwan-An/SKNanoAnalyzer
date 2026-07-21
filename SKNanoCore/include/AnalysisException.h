#ifndef SKNanoCore_AnalysisException_h
#define SKNanoCore_AnalysisException_h

#include <stdexcept>
#include <string>
#include <utility>

namespace SKNano {

enum class ErrorCategory {
    Config,
    Correction,
    EventData,
    Logic,
    Unknown
};

inline const char *ErrorCategoryName(ErrorCategory category) {
    switch (category) {
    case ErrorCategory::Config:
        return "Config";
    case ErrorCategory::Correction:
        return "Correction";
    case ErrorCategory::EventData:
        return "EventData";
    case ErrorCategory::Logic:
        return "Logic";
    default:
        return "Unknown";
    }
}

class AnalysisException : public std::runtime_error {
public:
    AnalysisException(ErrorCategory category, std::string message,
                      bool eventLocal)
        : std::runtime_error(std::move(message)), category_(category),
          eventLocal_(eventLocal) {}

    ErrorCategory category() const noexcept { return category_; }
    bool eventLocal() const noexcept { return eventLocal_; }

private:
    ErrorCategory category_;
    bool eventLocal_;
};

class ConfigError : public AnalysisException {
public:
    explicit ConfigError(const std::string &message)
        : AnalysisException(ErrorCategory::Config, message, false) {}
};

class CorrectionError : public AnalysisException {
public:
    explicit CorrectionError(const std::string &message,
                             bool eventLocal = true)
        : AnalysisException(ErrorCategory::Correction, message, eventLocal) {}
};

class EventDataError : public AnalysisException {
public:
    explicit EventDataError(const std::string &message)
        : AnalysisException(ErrorCategory::EventData, message, true) {}
};

class LogicError : public AnalysisException {
public:
    explicit LogicError(const std::string &message)
        : AnalysisException(ErrorCategory::Logic, message, false) {}
};

} // namespace SKNano

#endif

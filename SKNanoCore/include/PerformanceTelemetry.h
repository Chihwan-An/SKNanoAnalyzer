#ifndef SKNanoCore_PerformanceTelemetry_h
#define SKNanoCore_PerformanceTelemetry_h

#include <chrono>
#include <cstdint>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>

#if defined(__unix__) || defined(__APPLE__)
#include <sys/resource.h>
#endif

namespace SKNano {

class PerformanceTelemetry {
public:
    struct PhaseStats {
        double seconds = 0.;
        std::uint64_t calls = 0;
    };

    class ScopedPhase {
    public:
        ScopedPhase() = default;
        ScopedPhase(PerformanceTelemetry *owner_, std::string name_)
            : owner(owner_), name(std::move(name_)),
              start(std::chrono::steady_clock::now()) {}
        ScopedPhase(const ScopedPhase &) = delete;
        ScopedPhase &operator=(const ScopedPhase &) = delete;
        ScopedPhase(ScopedPhase &&other) noexcept
            : owner(other.owner), name(std::move(other.name)), start(other.start) {
            other.owner = nullptr;
        }
        ScopedPhase &operator=(ScopedPhase &&other) noexcept {
            if (this != &other) {
                finish();
                owner = other.owner;
                name = std::move(other.name);
                start = other.start;
                other.owner = nullptr;
            }
            return *this;
        }
        ~ScopedPhase() { finish(); }

    private:
        void finish() {
            if (!owner)
                return;
            const auto elapsed = std::chrono::duration<double>(
                std::chrono::steady_clock::now() - start).count();
            owner->recordPhase(name, elapsed);
            owner = nullptr;
        }
        PerformanceTelemetry *owner = nullptr;
        std::string name;
        std::chrono::steady_clock::time_point start{};
    };

    void setEnabled(bool value) noexcept { enabled_ = value; }
    bool enabled() const noexcept { return enabled_; }
    void setOutputPath(std::string path) { outputPath_ = std::move(path); }
    const std::string &outputPath() const noexcept { return outputPath_; }

    void startRun() {
        if (!enabled_)
            return;
        phases_.clear();
        counters_.clear();
        wallStart_ = std::chrono::steady_clock::now();
        cpuStart_ = std::clock();
        running_ = true;
        wallSeconds_ = 0.;
        cpuSeconds_ = 0.;
    }

    void stopRun() {
        if (!enabled_ || !running_)
            return;
        wallSeconds_ = std::chrono::duration<double>(
            std::chrono::steady_clock::now() - wallStart_).count();
        cpuSeconds_ = static_cast<double>(std::clock() - cpuStart_) /
                      static_cast<double>(CLOCKS_PER_SEC);
        running_ = false;
    }

    ScopedPhase measure(std::string name) {
        return enabled_ ? ScopedPhase(this, std::move(name)) : ScopedPhase();
    }

    void setCounter(std::string name, double value) {
        if (enabled_)
            counters_[std::move(name)] = value;
    }
    void addCounter(const std::string &name, double value = 1.) {
        if (enabled_)
            counters_[name] += value;
    }
    void setMetadata(std::string name, std::string value) {
        if (enabled_)
            metadata_[std::move(name)] = std::move(value);
    }

    const std::map<std::string, PhaseStats> &phases() const noexcept {
        return phases_;
    }
    const std::map<std::string, double> &counters() const noexcept {
        return counters_;
    }

    void writeJson() {
        if (!enabled_ || outputPath_.empty())
            return;
        stopRun();
        std::ofstream output(outputPath_);
        if (!output)
            throw std::runtime_error(
                "[PerformanceTelemetry] cannot write " + outputPath_);
        output << std::setprecision(17);
        output << "{\n  \"schema_version\": 1,\n";
        output << "  \"wall_seconds\": " << wallSeconds_ << ",\n";
        output << "  \"cpu_seconds\": " << cpuSeconds_ << ",\n";
        const auto events = counters_.find("events_processed");
        const double eventCount = events == counters_.end() ? 0. : events->second;
        output << "  \"events_per_second\": "
               << (wallSeconds_ > 0. ? eventCount / wallSeconds_ : 0.)
               << ",\n";
        output << "  \"cpu_seconds_per_event\": "
               << (eventCount > 0. ? cpuSeconds_ / eventCount : 0.)
               << ",\n";
        output << "  \"peak_rss_kib\": " << peakRssKiB() << ",\n";
        output << "  \"metadata\": {";
        writeStringMap(output, metadata_);
        output << "\n  },\n  \"counters\": {";
        writeNumberMap(output, counters_);
        output << "\n  },\n  \"phases\": {";
        bool first = true;
        for (const auto &item : phases_) {
            output << (first ? "\n" : ",\n") << "    \""
                   << escape(item.first) << "\": {\"seconds\": "
                   << item.second.seconds << ", \"calls\": "
                   << item.second.calls << "}";
            first = false;
        }
        if (!first)
            output << '\n';
        output << "  }\n}\n";
    }

private:
    void recordPhase(const std::string &name, double seconds) {
        auto &stats = phases_[name];
        stats.seconds += seconds;
        ++stats.calls;
    }

    static long peakRssKiB() noexcept {
#if defined(__unix__) || defined(__APPLE__)
        struct rusage usage {};
        if (getrusage(RUSAGE_SELF, &usage) == 0) {
#if defined(__APPLE__)
            return usage.ru_maxrss / 1024;
#else
            return usage.ru_maxrss;
#endif
        }
#endif
        return -1;
    }

    static std::string escape(const std::string &value) {
        std::string result;
        result.reserve(value.size());
        for (const char character : value) {
            switch (character) {
            case '\\': result += "\\\\"; break;
            case '"': result += "\\\""; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default: result += character; break;
            }
        }
        return result;
    }

    static void writeStringMap(
        std::ostream &output,
        const std::map<std::string, std::string> &values) {
        bool first = true;
        for (const auto &item : values) {
            output << (first ? "\n" : ",\n") << "    \""
                   << escape(item.first) << "\": \"" << escape(item.second)
                   << '"';
            first = false;
        }
        if (!first)
            output << '\n';
    }

    static void writeNumberMap(
        std::ostream &output,
        const std::map<std::string, double> &values) {
        bool first = true;
        for (const auto &item : values) {
            output << (first ? "\n" : ",\n") << "    \""
                   << escape(item.first) << "\": " << item.second;
            first = false;
        }
        if (!first)
            output << '\n';
    }

    bool enabled_ = false;
    bool running_ = false;
    std::string outputPath_;
    std::chrono::steady_clock::time_point wallStart_{};
    std::clock_t cpuStart_ = 0;
    double wallSeconds_ = 0.;
    double cpuSeconds_ = 0.;
    std::map<std::string, PhaseStats> phases_;
    std::map<std::string, double> counters_;
    std::map<std::string, std::string> metadata_;
};

} // namespace SKNano

#endif

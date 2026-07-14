#include <chrono>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include "PerformanceTelemetry.h"

namespace {

void require(bool condition, const char *message) {
    if (!condition)
        throw std::runtime_error(message);
}

} // namespace

int main() {
    const auto output = std::filesystem::temp_directory_path() /
                        "sknano_performance_telemetry.json";
    try {
        SKNano::PerformanceTelemetry telemetry;
        telemetry.setEnabled(true);
        telemetry.setOutputPath(output.string());
        telemetry.setMetadata("workload", "unit");
        telemetry.startRun();
        {
            auto phase = telemetry.measure("selection");
            volatile std::uint64_t sum = 0;
            for (std::uint64_t index = 0; index < 10000; ++index)
                sum += index;
            static_cast<void>(sum);
        }
        telemetry.addCounter("events", 2.);
        telemetry.addCounter("events", 3.);
        telemetry.writeJson();

        require(telemetry.phases().at("selection").calls == 1,
                "phase call count must be recorded");
        require(telemetry.phases().at("selection").seconds >= 0.,
                "phase duration must be non-negative");
        require(telemetry.counters().at("events") == 5.,
                "numeric counters must accumulate");
        std::ifstream input(output);
        const std::string json((std::istreambuf_iterator<char>(input)),
                               std::istreambuf_iterator<char>());
        require(json.find("\"schema_version\": 1") != std::string::npos &&
                    json.find("\"selection\"") != std::string::npos &&
                    json.find("\"peak_rss_kib\"") != std::string::npos,
                "telemetry JSON must contain stable schema fields");
        std::filesystem::remove(output);
    } catch (const std::exception &error) {
        std::filesystem::remove(output);
        std::cerr << "test_performance_telemetry: " << error.what() << '\n';
        return 1;
    }
    return 0;
}

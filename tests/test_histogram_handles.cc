#include <cmath>
#include <exception>
#include <iostream>
#include <stdexcept>

#include "AnalyzerCore.h"

namespace {

template <typename Exception, typename Function>
bool throws(Function &&function) {
    try {
        function();
    } catch (const Exception &) {
        return true;
    } catch (...) {
    }
    return false;
}

void require(bool condition, const char *message) {
    if (!condition)
        throw std::runtime_error(message);
}

} // namespace

int main() {
    try {
        AnalyzerCore analyzer;
        const auto histogram = analyzer.BookHist1D("typed", 4, 0., 4.);
        const auto same = analyzer.BookHist1D("typed", 4, 0., 4.);
        require(histogram.get() == same.get(),
                "same name and schema must return one canonical histogram");

        histogram.Fill(0.5, 2.0);
        histogram.Fill(1.5, -0.5);
        require(histogram.get()->GetEntries() == 2.,
                "typed fills must preserve ROOT fEntries semantics");
        require(std::abs(histogram.get()->GetBinContent(1) - 2.) < 1e-12 &&
                    std::abs(histogram.get()->GetBinContent(2) + 0.5) < 1e-12,
                "typed fills must preserve weighted bin contents");
        require(histogram.get()->GetSumw2N() > 0 &&
                    std::abs(histogram.get()->GetBinError(1) - 2.) < 1e-12,
                "typed fills must preserve sumw2");

        require(throws<SKNano::ConfigError>([&] {
                    static_cast<void>(analyzer.BookHist1D("typed", 5, 0., 5.));
                }),
                "duplicate uniform binning mismatch must fail");
        const float variableEdges[] = {0.f, 1.f, 3.f};
        analyzer.BookHist1D("variable", 2, variableEdges);
        const float wrongEdges[] = {0.f, 2.f, 3.f};
        require(throws<SKNano::ConfigError>([&] {
                    static_cast<void>(
                        analyzer.BookHist1D("variable", 2, wrongEdges));
                }),
                "duplicate variable binning mismatch must fail");
        require(throws<SKNano::ConfigError>([&] {
                    static_cast<void>(analyzer.BookHist2D(
                        "typed", 4, 0., 4., 4, 0., 4.));
                }),
                "same name with another dimension must fail");

        analyzer.FillHist("legacy", 0.5f, 1.f, 2, 0.f, 2.f);
        require(analyzer.GetHist1D("legacy")->GetEntries() == 1.,
                "legacy FillHist must route through canonical booking");
    } catch (const std::exception &error) {
        std::cerr << "test_histogram_handles: " << error.what() << '\n';
        return 1;
    }
    return 0;
}

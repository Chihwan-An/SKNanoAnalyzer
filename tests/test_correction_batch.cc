#include <array>
#include <cmath>
#include <exception>
#include <iostream>
#include <stdexcept>

#include "CorrectionBatch.h"

namespace {
struct Evaluator {
    mutable int jecCalls = 0;
    mutable int resolutionCalls = 0;
    mutable int scaleFactorCalls = 0;
    float jec(float, float, float, float, float, std::uint32_t) const {
        ++jecCalls; return 2.f;
    }
    std::array<float, 3> jerScaleFactors(float, float) const {
        ++scaleFactorCalls; return {1.f, 2.f, 0.5f};
    }
    float jerResolution(float, float, float) const {
        ++resolutionCalls; return 0.1f;
    }
    float jesUncertainty(float, float) const { return 0.05f; }
    float sqrt(float value) const { return std::sqrt(value); }
};
void require(bool condition, const char *message) {
    if (!condition) throw std::runtime_error(message);
}
}

int main() {
    try {
        const float pt[] = {10.f, 20.f};
        const float eta[] = {0.f, 1.f};
        const float phi[] = {0.f, 2.f};
        const float mass[] = {2.f, 4.f};
        const float raw[] = {0.1f, 0.2f};
        const float area[] = {0.5f, 0.6f};
        const int matches[] = {-1, 0};
        const float genPt[] = {0.f, 20.f};
        const float gaussian[] = {1.f, 99.f};
        SKNano::CorrectionLaneMask jer;
        jer.add(SKNano::CorrectionLane::Up).add(SKNano::CorrectionLane::Down);
        SKNano::CorrectionLaneMask jes;
        jes.add(SKNano::CorrectionLane::Up);
        SKNano::JetCorrectionBatch batch(jer, jes);
        SKNano::JetCorrectionBatchOutput output;
        output.reserve(2);
        const auto capacity = output.correctedPt.capacity();
        Evaluator evaluator;
        batch.evaluate({pt, eta, phi, mass, raw, area, matches, genPt,
                        gaussian, 2, 20.f, 1}, output, evaluator);
        require(output.correctedPt[0] == 18.f && output.correctedPt[1] == 32.f,
                "JEC must preserve raw-to-corrected multiplication order");
        require(evaluator.jecCalls == 2 && evaluator.scaleFactorCalls == 2 &&
                    evaluator.resolutionCalls == 1,
                "resolution and scale-factor sets must be reused across lanes");
        require(output.upPt.size() == 2 && output.downPt.size() == 2 &&
                    output.jesUpPt.size() == 2 &&
                    output.jesUpMass.size() == 2 &&
                    output.jesDownPt.empty() && output.jesDownMass.empty(),
                "only demanded correction lanes must be materialized");
        require(output.jesUpPt[1] == output.nominalPt[1] * 1.05f,
                "JES must be applied after nominal JER in the batch plan");
        batch.evaluate({pt, eta, phi, mass, raw, area, matches, genPt,
                        gaussian, 2, 20.f, 1}, output, evaluator);
        require(output.correctedPt.capacity() == capacity,
                "warm batch evaluation must reuse output allocation");
    } catch (const std::exception &error) {
        std::cerr << "test_correction_batch: " << error.what() << '\n';
        return 1;
    }
    return 0;
}

#ifndef SKNanoCore_CorrectionBatch_h
#define SKNanoCore_CorrectionBatch_h

#include "AnalysisException.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace SKNano {

enum class CorrectionLane : std::uint32_t {
    Nominal = 1u << 0,
    Up = 1u << 1,
    Down = 1u << 2,
};

class CorrectionLaneMask {
public:
    CorrectionLaneMask() = default;
    explicit CorrectionLaneMask(CorrectionLane lane)
        : bits_(static_cast<std::uint32_t>(lane)) {}
    CorrectionLaneMask &add(CorrectionLane lane) {
        bits_ |= static_cast<std::uint32_t>(lane);
        return *this;
    }
    bool contains(CorrectionLane lane) const {
        return (bits_ & static_cast<std::uint32_t>(lane)) != 0;
    }
private:
    std::uint32_t bits_ = static_cast<std::uint32_t>(CorrectionLane::Nominal);
};

struct JetCorrectionBatchInput {
    const float *pt = nullptr;
    const float *eta = nullptr;
    const float *phi = nullptr;
    const float *mass = nullptr;
    const float *rawFactor = nullptr;
    const float *area = nullptr;
    const int *matchedGenIndex = nullptr;
    const float *matchedGenPt = nullptr;
    const float *gaussianSample = nullptr;
    std::size_t size = 0;
    float rho = 0.f;
    std::uint32_t run = 0;
};

struct JetCorrectionBatchOutput {
    std::vector<float> correctedPt;
    std::vector<float> correctedMass;
    std::vector<float> nominalPt;
    std::vector<float> nominalMass;
    std::vector<float> upPt;
    std::vector<float> upMass;
    std::vector<float> downPt;
    std::vector<float> downMass;
    std::vector<float> jesUpPt;
    std::vector<float> jesUpMass;
    std::vector<float> jesDownPt;
    std::vector<float> jesDownMass;

    void reserve(std::size_t size) {
        correctedPt.reserve(size); correctedMass.reserve(size);
        nominalPt.reserve(size); nominalMass.reserve(size);
        upPt.reserve(size); upMass.reserve(size);
        downPt.reserve(size); downMass.reserve(size);
        jesUpPt.reserve(size); jesUpMass.reserve(size);
        jesDownPt.reserve(size); jesDownMass.reserve(size);
    }
};

class JetCorrectionBatch {
public:
    explicit JetCorrectionBatch(CorrectionLaneMask jerLanes = {},
                                CorrectionLaneMask jesLanes = {})
        : jerLanes_(jerLanes), jesLanes_(jesLanes) {}

    template <typename Evaluator>
    void evaluate(const JetCorrectionBatchInput &input,
                  JetCorrectionBatchOutput &output,
                  const Evaluator &evaluator) const {
        validate(input);
        resize(output.correctedPt, input.size);
        resize(output.correctedMass, input.size);
        resize(output.nominalPt, input.size);
        resize(output.nominalMass, input.size);
        resizeIfRequested(output.upPt, input.size,
                          jerLanes_.contains(CorrectionLane::Up));
        resizeIfRequested(output.upMass, input.size,
                          jerLanes_.contains(CorrectionLane::Up));
        resizeIfRequested(output.downPt, input.size,
                          jerLanes_.contains(CorrectionLane::Down));
        resizeIfRequested(output.downMass, input.size,
                          jerLanes_.contains(CorrectionLane::Down));
        resizeIfRequested(output.jesUpPt, input.size,
                          jesLanes_.contains(CorrectionLane::Up));
        resizeIfRequested(output.jesUpMass, input.size,
                          jesLanes_.contains(CorrectionLane::Up));
        resizeIfRequested(output.jesDownPt, input.size,
                          jesLanes_.contains(CorrectionLane::Down));
        resizeIfRequested(output.jesDownMass, input.size,
                          jesLanes_.contains(CorrectionLane::Down));

        for (std::size_t index = 0; index < input.size; ++index) {
            const float rawScale = 1.f - input.rawFactor[index];
            const float rawPt = input.pt[index] * rawScale;
            const float rawMass = input.mass[index] * rawScale;
            const float jec = evaluator.jec(
                input.area[index], input.eta[index], rawPt, input.phi[index],
                input.rho, input.run);
            const float correctedPt = rawPt * jec;
            const float correctedMass = rawMass * jec;
            output.correctedPt[index] = correctedPt;
            output.correctedMass[index] = correctedMass;

            const auto scaleFactors = evaluator.jerScaleFactors(
                input.eta[index], correctedPt);
            const int matched = input.matchedGenIndex
                                    ? input.matchedGenIndex[index] : -1;
            const float genPt = input.matchedGenPt
                                  ? input.matchedGenPt[index] : 0.f;
            const float resolution = matched < 0
                ? evaluator.jerResolution(input.eta[index], correctedPt,
                                          input.rho)
                : 0.f;
            const float gaussian = input.gaussianSample
                                       ? input.gaussianSample[index] : 0.f;
            const auto smear = [&](float scaleFactor) {
                if (correctedPt <= 0.f)
                    return 1.f;
                if (matched >= 0) {
                    const float scale =
                        1.f - genPt / (correctedPt > 1e-6f ? correctedPt : 1e-6f);
                    const float value = 1.f + (scaleFactor - 1.f) * scale;
                    return value > 0.f ? value : 0.f;
                }
                const float variance = scaleFactor * scaleFactor - 1.f;
                const float width = variance > 0.f ? evaluator.sqrt(variance) : 0.f;
                const float value = 1.f + gaussian * resolution * width;
                return value > 0.f ? value : 0.f;
            };
            const float nominalSmear = smear(scaleFactors[0]);
            output.nominalPt[index] = correctedPt * nominalSmear;
            output.nominalMass[index] = correctedMass * nominalSmear;
            if (jerLanes_.contains(CorrectionLane::Up)) {
                const float value = smear(scaleFactors[1]);
                output.upPt[index] = correctedPt * value;
                output.upMass[index] = correctedMass * value;
            }
            if (jerLanes_.contains(CorrectionLane::Down)) {
                const float value = smear(scaleFactors[2]);
                output.downPt[index] = correctedPt * value;
                output.downMass[index] = correctedMass * value;
            }
            if (jesLanes_.contains(CorrectionLane::Up) ||
                jesLanes_.contains(CorrectionLane::Down)) {
                const float uncertainty =
                    evaluator.jesUncertainty(input.eta[index], correctedPt);
                if (jesLanes_.contains(CorrectionLane::Up))
                {
                    output.jesUpPt[index] =
                        output.nominalPt[index] * (1.f + uncertainty);
                    output.jesUpMass[index] =
                        output.nominalMass[index] * (1.f + uncertainty);
                }
                if (jesLanes_.contains(CorrectionLane::Down)) {
                    output.jesDownPt[index] =
                        output.nominalPt[index] * (1.f - uncertainty);
                    output.jesDownMass[index] =
                        output.nominalMass[index] * (1.f - uncertainty);
                }
            }
        }
    }

private:
    static void validate(const JetCorrectionBatchInput &input) {
        if (input.size && (!input.pt || !input.eta || !input.phi ||
                           !input.mass || !input.rawFactor || !input.area))
            throw EventDataError("[JetCorrectionBatch] missing required input span");
    }
    static void resize(std::vector<float> &values, std::size_t size) {
        values.resize(size);
    }
    static void resizeIfRequested(std::vector<float> &values, std::size_t size,
                                  bool requested) {
        if (requested)
            values.resize(size);
        else
            values.clear();
    }
    CorrectionLaneMask jerLanes_;
    CorrectionLaneMask jesLanes_;
};

} // namespace SKNano

#endif

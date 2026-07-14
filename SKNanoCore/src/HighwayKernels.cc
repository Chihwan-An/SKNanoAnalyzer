#include "HighwayKernels.h"

#include "AnalysisException.h"

#include <cmath>

#undef HWY_TARGET_INCLUDE
#define HWY_TARGET_INCLUDE "HighwayKernels.cc"
#include "hwy/foreach_target.h"
#include "hwy/highway.h"

HWY_BEFORE_NAMESPACE();
namespace SKNano {
namespace HWY_NAMESPACE {

namespace hn = hwy::HWY_NAMESPACE;

void ptEtaMaskHighway(const float *pt, const float *eta, std::size_t size,
                      float minimumPt, float maximumAbsEta,
                      std::uint8_t *mask) {
    const hn::ScalableTag<float> d;
    const std::size_t lanes = hn::Lanes(d);
    const auto minimum = hn::Set(d, minimumPt);
    const auto maximumEta = hn::Set(d, maximumAbsEta);
    HWY_ALIGN float passLanes[HWY_MAX_LANES_D(decltype(d))];
    std::size_t index = 0;
    for (; index + lanes <= size; index += lanes) {
        const auto pass = hn::And(
            hn::Ge(hn::LoadU(d, pt + index), minimum),
            hn::Le(hn::Abs(hn::LoadU(d, eta + index)), maximumEta));
        hn::StoreU(hn::IfThenElse(pass, hn::Set(d, 1.f), hn::Zero(d)),
                   d, passLanes);
        for (std::size_t lane = 0; lane < lanes; ++lane)
            mask[index + lane] = static_cast<std::uint8_t>(passLanes[lane]);
    }
    for (; index < size; ++index)
        mask[index] = static_cast<std::uint8_t>(
            pt[index] >= minimumPt && std::fabs(eta[index]) <= maximumAbsEta);
}

void deltaR2Highway(const float *eta, const float *phi, std::size_t size,
                    float referenceEta, float referencePhi, float *output) {
    constexpr float piValue = 3.14159265358979323846f;
    constexpr float twoPiValue = 2.f * piValue;
    const hn::ScalableTag<float> d;
    const std::size_t lanes = hn::Lanes(d);
    const auto referenceEtas = hn::Set(d, referenceEta);
    const auto referencePhis = hn::Set(d, referencePhi);
    const auto pi = hn::Set(d, piValue);
    const auto negativePi = hn::Set(d, -piValue);
    const auto twoPi = hn::Set(d, twoPiValue);
    std::size_t index = 0;
    for (; index + lanes <= size; index += lanes) {
        const auto dEta = hn::Sub(hn::LoadU(d, eta + index), referenceEtas);
        const auto rawDPhi = hn::Sub(hn::LoadU(d, phi + index), referencePhis);
        auto dPhi = hn::IfThenElse(hn::Gt(rawDPhi, pi),
                                   hn::Sub(rawDPhi, twoPi), rawDPhi);
        dPhi = hn::IfThenElse(hn::Le(rawDPhi, negativePi),
                              hn::Add(rawDPhi, twoPi), dPhi);
        hn::StoreU(hn::Add(hn::Mul(dEta, dEta), hn::Mul(dPhi, dPhi)),
                   d, output + index);
    }
    for (; index < size; ++index) {
        const volatile float dEta = eta[index] - referenceEta;
        volatile float dPhi = phi[index] - referencePhi;
        if (dPhi > piValue)
            dPhi = dPhi - twoPiValue;
        else if (dPhi <= -piValue)
            dPhi = dPhi + twoPiValue;
        const volatile float dEta2 = dEta * dEta;
        const volatile float dPhi2 = dPhi * dPhi;
        output[index] = dEta2 + dPhi2;
    }
}

const char *highwayTargetNameImpl() { return hwy::TargetName(HWY_TARGET); }

} // namespace HWY_NAMESPACE
} // namespace SKNano
HWY_AFTER_NAMESPACE();

#if HWY_ONCE
namespace SKNano {

HWY_EXPORT(ptEtaMaskHighway);
HWY_EXPORT(deltaR2Highway);
HWY_EXPORT(highwayTargetNameImpl);

void ptEtaMask(const float *pt, const float *eta, std::size_t size,
               float minimumPt, float maximumAbsEta, std::uint8_t *mask) {
    if (size && (!pt || !eta || !mask))
        throw EventDataError("[ptEtaMask] null kernel input");
    HWY_DYNAMIC_DISPATCH(ptEtaMaskHighway)(
        pt, eta, size, minimumPt, maximumAbsEta, mask);
}

void deltaR2(const float *eta, const float *phi, std::size_t size,
             float referenceEta, float referencePhi, float *output) {
    if (size && (!eta || !phi || !output))
        throw EventDataError("[deltaR2] null kernel input");
    HWY_DYNAMIC_DISPATCH(deltaR2Highway)(
        eta, phi, size, referenceEta, referencePhi, output);
}

const char *highwayTargetName() {
    return HWY_DYNAMIC_DISPATCH(highwayTargetNameImpl)();
}

} // namespace SKNano
#endif

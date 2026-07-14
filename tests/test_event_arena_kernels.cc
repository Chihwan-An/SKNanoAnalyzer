#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "EventArena.h"
#include "HighwayKernels.h"

namespace {
template <typename Exception, typename Function>
bool throws(Function &&function) {
    try { function(); } catch (const Exception &) { return true; } catch (...) {}
    return false;
}
void require(bool condition, const char *message) {
    if (!condition) throw std::runtime_error(message);
}
void scalarMask(const float *pt, const float *eta, std::size_t size,
                float minimumPt, float maximumAbsEta, std::uint8_t *mask) {
    for (std::size_t index = 0; index < size; ++index)
        mask[index] = static_cast<std::uint8_t>(
            pt[index] >= minimumPt && std::fabs(eta[index]) <= maximumAbsEta);
}
void scalarDeltaR2(const float *eta, const float *phi, std::size_t size,
                   float referenceEta, float referencePhi, float *output) {
    constexpr float pi = 3.14159265358979323846f;
    constexpr float twoPi = 2.f * pi;
    for (std::size_t index = 0; index < size; ++index) {
        const volatile float dEta = eta[index] - referenceEta;
        volatile float dPhi = phi[index] - referencePhi;
        if (dPhi > pi) dPhi = dPhi - twoPi;
        else if (dPhi <= -pi) dPhi = dPhi + twoPi;
        const volatile float dEta2 = dEta * dEta;
        const volatile float dPhi2 = dPhi * dPhi;
        output[index] = dEta2 + dPhi2;
    }
}
}

int main() {
    try {
        SKNano::EventArena arena(4096);
        auto pt = arena.allocateArray<float>(37);
        auto eta = arena.allocateArray<float>(37);
        auto mask = arena.allocateArray<std::uint8_t>(37);
        auto dr2 = arena.allocateArray<float>(37);
        require(reinterpret_cast<std::uintptr_t>(pt.data()) % 64 == 0,
                "arena array must honor requested alignment");

        std::vector<float> phi(pt.size());
        for (std::size_t index = 0; index < pt.size(); ++index) {
            pt[index] = static_cast<float>(index * 3) - 20.f;
            eta[index] = static_cast<float>(static_cast<int>(index % 9) - 4) * .7f;
            phi[index] = static_cast<float>(static_cast<int>(index % 13) - 6) * .8f;
        }
        std::vector<std::uint8_t> referenceMask(pt.size());
        std::vector<float> referenceDr2(pt.size());
        scalarMask(pt.data(), eta.data(), pt.size(), 25.f, 2.5f,
                   referenceMask.data());
        scalarDeltaR2(eta.data(), phi.data(), pt.size(), .3f, -2.9f,
                      referenceDr2.data());
        SKNano::ptEtaMask(pt.data(), eta.data(), pt.size(), 25.f, 2.5f,
                          mask.data());
        SKNano::deltaR2(eta.data(), phi.data(), pt.size(), .3f, -2.9f,
                        dr2.data());
        require(std::equal(mask.data(), mask.data() + mask.size(),
                           referenceMask.begin()),
                "Highway mask body and scalar tail must match reference");
        for (std::size_t index = 0; index < dr2.size(); ++index) {
            std::uint32_t actualBits = 0, referenceBits = 0;
            std::memcpy(&actualBits, &dr2[index], sizeof(float));
            std::memcpy(&referenceBits, &referenceDr2[index], sizeof(float));
            const auto distance = actualBits > referenceBits
                ? actualBits - referenceBits : referenceBits - actualBits;
            if (distance > 1)
                throw std::runtime_error(
                    "Highway deltaR exceeds one ULP at index " +
                    std::to_string(index) + ": actual=" +
                    std::to_string(dr2[index]) + ", reference=" +
                    std::to_string(referenceDr2[index]) + ", ulp=" +
                    std::to_string(distance) + ", actual_bits=" +
                    std::to_string(actualBits) + ", reference_bits=" +
                    std::to_string(referenceBits));
        }
        require(SKNano::highwayTargetName() && *SKNano::highwayTargetName(),
                "Highway must report the selected runtime target");

        arena.reset();
        require(throws<SKNano::LogicError>([&] { static_cast<void>(pt.size()); }),
                "arena allocation must fail its epoch check after reset");
    } catch (const std::exception &error) {
        std::cerr << "test_event_arena_kernels: " << error.what() << '\n';
        return 1;
    }
    return 0;
}

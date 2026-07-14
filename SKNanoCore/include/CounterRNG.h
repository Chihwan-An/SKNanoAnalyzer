#ifndef SKNanoCore_CounterRNG_h
#define SKNanoCore_CounterRNG_h

#include <cmath>
#include <cstdint>
#include <limits>

namespace SKNano {

enum class RngMode { StrictLegacy, CounterBased };

class CounterRNG {
public:
    static std::uint64_t bits(std::uint64_t run, std::uint64_t lumi,
                              std::uint64_t event, std::uint64_t object,
                              std::uint64_t stream, std::uint64_t counter = 0) {
        std::uint64_t state = mix(run + 0x9e3779b97f4a7c15ULL);
        state = mix(state ^ lumi);
        state = mix(state ^ event);
        state = mix(state ^ object);
        state = mix(state ^ stream);
        return mix(state ^ counter);
    }

    static double uniform(std::uint64_t run, std::uint64_t lumi,
                          std::uint64_t event, std::uint64_t object,
                          std::uint64_t stream, std::uint64_t counter = 0) {
        const std::uint64_t mantissa =
            bits(run, lumi, event, object, stream, counter) >> 11;
        return (static_cast<double>(mantissa) + 0.5) *
               (1.0 / 9007199254740992.0);
    }

    static double normal(std::uint64_t run, std::uint64_t lumi,
                         std::uint64_t event, std::uint64_t object,
                         std::uint64_t stream) {
        const double u1 = uniform(run, lumi, event, object, stream, 0);
        const double u2 = uniform(run, lumi, event, object, stream, 1);
        constexpr double twoPi = 6.283185307179586476925286766559;
        return std::sqrt(-2.0 * std::log(u1)) * std::cos(twoPi * u2);
    }

private:
    static std::uint64_t mix(std::uint64_t value) {
        value += 0x9e3779b97f4a7c15ULL;
        value = (value ^ (value >> 30)) * 0xbf58476d1ce4e5b9ULL;
        value = (value ^ (value >> 27)) * 0x94d049bb133111ebULL;
        return value ^ (value >> 31);
    }
};

} // namespace SKNano

#endif

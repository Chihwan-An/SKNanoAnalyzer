#ifndef SKNanoCore_HighwayKernels_h
#define SKNanoCore_HighwayKernels_h

#include <cstddef>
#include <cstdint>

namespace SKNano {

void ptEtaMask(const float *pt, const float *eta, std::size_t size,
               float minimumPt, float maximumAbsEta, std::uint8_t *mask);

void deltaR2(const float *eta, const float *phi, std::size_t size,
             float referenceEta, float referencePhi, float *output);

const char *highwayTargetName();

} // namespace SKNano

#endif

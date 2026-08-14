#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <cmath>

// Eta/phi geometry shared by every dR computation in the framework.  This used
// to be re-derived at each call site -- overlap removal, trigger matching, jet
// smearing -- with the phi wrap written out by hand each time.  One copy, so a
// boundary convention cannot drift between them.
namespace SKNano::Geometry {

inline constexpr float kPi = 3.14159265358979323846f;
inline constexpr float kTwoPi = 2.f * kPi;

// Phi difference folded into (-pi, pi].  Both inputs are assumed to live in
// (-pi, pi] already, which is what NanoAOD stores, so a single fold suffices.
inline float DeltaPhi(const float phi1, const float phi2) noexcept {
    float dPhi = phi1 - phi2;
    if (dPhi > kPi)
        dPhi -= kTwoPi;
    else if (dPhi <= -kPi)
        dPhi += kTwoPi;
    return dPhi;
}

// Squared separation.  Prefer this over DeltaR when comparing against a cut:
// it orders identically and skips the sqrt.
inline float DeltaR2(const float eta1, const float phi1, const float eta2,
                     const float phi2) noexcept {
    const float dEta = eta1 - eta2;
    const float dPhi = DeltaPhi(phi1, phi2);
    return dEta * dEta + dPhi * dPhi;
}

inline float DeltaR(const float eta1, const float phi1, const float eta2,
                    const float phi2) noexcept {
    return std::sqrt(DeltaR2(eta1, phi1, eta2, phi2));
}

// View overloads.  Anything exposing Eta() and Phi() qualifies, which is every
// *View plus SelectedJetView, so callers never build a TLorentzVector just to
// ask how far apart two objects are.
template <typename A, typename B>
inline float DeltaR2(const A &a, const B &b) noexcept {
    return DeltaR2(a.Eta(), a.Phi(), b.Eta(), b.Phi());
}

template <typename A, typename B>
inline float DeltaR(const A &a, const B &b) noexcept {
    return std::sqrt(DeltaR2(a, b));
}

} // namespace SKNano::Geometry

#endif

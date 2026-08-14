#ifndef LEPTONIDENUMS_H
#define LEPTONIDENUMS_H

// Shared ID enums for lepton view/object pairs.
namespace LeptonID {

enum class ElectronID {
    NOCUT,
    POG_VETO,
    POG_LOOSE,
    POG_MEDIUM,
    POG_TIGHT,
    POG_HEEP,
    POG_MVAISO_WP80,
    POG_MVAISO_WP90,
    POG_MVAISO_WPL,
    POG_MVANOISO_WP80,
    POG_MVANOISO_WP90,
    POG_MVANOISO_WPL,
    POG_PROMPTMVA_TIGHT,
    POG_PROMPTMVA_MEDIUM
};

enum class ElectronCutBasedWP : unsigned char {
    NONE = 0,
    VETO = 1,
    LOOSE = 2,
    MEDIUM = 3,
    TIGHT = 4
};

enum class ElectronEtaRegion {
    IB,
    OB,
    GAP,
    EC
};

enum class MuonWorkingPoint : unsigned char {
    NONE = 0,
    VLOOSE = 1,
    LOOSE = 2,
    MEDIUM = 3,
    TIGHT = 4,
    VTIGHT = 5,
    VVTIGHT = 6
};

enum class MuonID {
    NOCUT,
    POG_TIGHT,
    POG_MEDIUM,
    POG_MEDIUM_PROMPT,
    POG_LOOSE,
    POG_SOFT,
    POG_SOFT_MVA,
    POG_TRIGGER_LOOSE,
    POG_TRACKER_HIGH_PT,
    POG_GLOBAL_HIGH_PT,
    POG_MINISO_LOOSE,
    POG_MINISO_MEDIUM,
    POG_MINISO_TIGHT,
    POG_MINISO_VTIGHT,
    POG_MULTISO_LOOSE,
    POG_MULTISO_MEDIUM,
    POG_MVA_MU_MEDIUM,
    POG_MVA_MU_TIGHT,
    POG_PFISO_VLOOSE,
    POG_PFISO_LOOSE,
    POG_PFISO_MEDIUM,
    POG_PFISO_TIGHT,
    POG_PFISO_VTIGHT,
    POG_PFISO_VVTIGHT,
    POG_PUPPIISO_LOOSE,
    POG_PUPPIISO_MEDIUM,
    POG_PUPPIISO_TIGHT,
    POG_TKISO_LOOSE,
    POG_TKISO_TIGHT,
    POG_PROMPTMVA_WP0p64,
    POG_SOFTMVA_RUN3_TIGHT,
    POG_SOFTMVA_RUN3_MEDIUM,
    POG_SOFTMVA_RUN3_LOOSE,
    POG_SOFTMVA_RUN3_VLOOSE
};

// DeepTau 2018v2p5 working points.  The stored NanoAOD column is the index of
// the tightest point the tau passes, so the enumerator values are the column
// values and ordering them means ordering the working points.
enum class TauWorkingPoint : unsigned char {
    NONE = 0,
    VVVLOOSE = 1,
    VVLOOSE = 2,
    VLOOSE = 3,
    LOOSE = 4,
    MEDIUM = 5,
    TIGHT = 6,
    VTIGHT = 7,
    VVTIGHT = 8
};

// The vsMu discriminator is published with four points, not eight.
enum class TauWorkingPointVsMu : unsigned char {
    NONE = 0,
    VLOOSE = 1,
    LOOSE = 2,
    MEDIUM = 3,
    TIGHT = 4
};

// Spelling used by the TAU POG correctionlib files.  Found by ADL, so callers
// holding a LeptonID enum can write ToCorrectionString(wp) unqualified.
inline const char *ToCorrectionString(const TauWorkingPoint wp) {
    switch (wp) {
    case TauWorkingPoint::VVVLOOSE: return "VVVLoose";
    case TauWorkingPoint::VVLOOSE:  return "VVLoose";
    case TauWorkingPoint::VLOOSE:   return "VLoose";
    case TauWorkingPoint::LOOSE:    return "Loose";
    case TauWorkingPoint::MEDIUM:   return "Medium";
    case TauWorkingPoint::TIGHT:    return "Tight";
    case TauWorkingPoint::VTIGHT:   return "VTight";
    case TauWorkingPoint::VVTIGHT:  return "VVTight";
    case TauWorkingPoint::NONE:     return "None";
    }
    return "None";
}

inline const char *ToCorrectionString(const TauWorkingPointVsMu wp) {
    switch (wp) {
    case TauWorkingPointVsMu::VLOOSE: return "VLoose";
    case TauWorkingPointVsMu::LOOSE:  return "Loose";
    case TauWorkingPointVsMu::MEDIUM: return "Medium";
    case TauWorkingPointVsMu::TIGHT:  return "Tight";
    case TauWorkingPointVsMu::NONE:   return "None";
    }
    return "None";
}

} // namespace LeptonID

#endif // LEPTONIDENUMS_H

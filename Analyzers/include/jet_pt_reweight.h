#ifndef jet_pt_reweight_h
#define jet_pt_reweight_h

#include "Reproduce20_002_copy.h"

// DY control-region jet-pT spectra, with the gen Z-pT correction applied, for
// deriving the data-driven jet-pT reweighting
//
//     R(pT) = [ N_data - N_nonDY ] / N_DY        in the DY CR
//
// downstream (fig/05_backgrounds/scripts/make_dy_jetpt_ratio.py).
//
// Why this is a separate analyzer rather than a change to Reproduce20_002_copy:
// R has to be re-derived every time C changes, and C is not final (it currently
// exists for 2022 + 2022EE only; 2023 needs its samples sorted out first).
// Re-running the full production for that would mean the SR, every CR and 19
// object systematics, when the derivation only ever reads four histograms out
// of the DY CR. This analyzer fills those four and stops.
//
// Only DY has to be re-run. C multiplies DY MC alone, so the data and non-DY
// histograms are bit-for-bit what the existing Reproduce20_002_copy output
// already holds -- 14 DYMLL samples per era instead of 291.
//
// That reuse is only valid if the selection here is identical to the parent's.
// The class therefore inherits Reproduce20_002_copy so the cut values
// (el_set / mu_set / jet_set / fatjet_set), the cleaning helpers and the
// trigger configuration are literally the same objects rather than copies that
// can drift. The event flow below is still a transcription of the parent's DY
// CR path, so it must be validated before use: run once with the correction
// disabled and check the DY CR histograms against the existing production bin
// by bin. `--no-zpt` exists for exactly that.
class jet_pt_reweight : public Reproduce20_002_copy {
public:
    jet_pt_reweight();
    ~jet_pt_reweight();

    void initializeAnalyzer() override;
    void executeEvent() override;

private:
    // --- gen Z pT correction -------------------------------------------------
    // Loaded from the ROOT file make_dy_zpt_nlo_lo.py writes. Path comes from
    // $DY_ZPT_CORRECTION if set, else kDefaultZptFile. The era-specific curve is
    // used; there is no combined fallback, because silently reweighting 2023
    // with a 2022 curve is worse than not running.
    static constexpr const char *kDefaultZptFile =
        "/data9/Users/achihwan/25-020/AN-25-020/chihwan/fig/05_backgrounds/"
        "corrections/zpt/dy_zpt_nlo_lo.root";
    // The variant the analysis delivers: isHardProcess lepton pair, LHE_HT > 40.
    // Not the native/ copy -- that one still has the unmerged tail where the NLO
    // numerator is empty and C would come out as 0.
    static constexpr const char *kZptVariant = "Zpt_hardproc_HT40";

    std::vector<double> fZptEdges;   // size N+1
    std::vector<double> fZptValues;  // size N
    bool fApplyZpt = false;          // false for data, non-DY, or --no-zpt

    // C(genZpT). Clamps to the last bin above the top edge, and returns 1 for a
    // non-finite or non-positive entry -- a 0 here would delete the event
    // instead of leaving it alone.
    float GetZptWeight(float gen_zpt) const;

    // Sum of the two isHardProcess charged leptons.
    //
    // Must stay identical to DYGenZpT.cc, which is what C was derived with: a
    // correction looked up with a different definition than it was built from is
    // not the same correction. Returns -1 if the pair is not found, which for a
    // DY sample means the assumption behind C has failed and the job stops.
    float GetGenZpT() const;

    // DY samples the correction applies to. Deliberately the HT-binned LO set
    // only: DYJets is the amcatnloFXFX sample used as the *numerator* when C was
    // built, so reweighting it would be circular, and it is not part of the DY
    // background estimate either.
    bool IsDYSample() const;

    bool fWarnedNoCurve = false;
};

#endif

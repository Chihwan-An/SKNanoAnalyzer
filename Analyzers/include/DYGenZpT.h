#ifndef DYGenZpT_h
#define DYGenZpT_h

#include <algorithm>

#include "AnalyzerCore.h"
#include "SystematicHelper.h"
#include "Gen.h"
#include "GenDressedLepton.h"

// Generator-level DY Z-pT spectrum, for deriving the NLO/LO reweighting.
//
// No object selection and no detector-level quantity is touched: the analyzer
// reads the gen record, finds the Z, and fills its pT. The correction itself,
//     C(pT) = N_NLO(pT) / N_LO(pT),
// is formed downstream (fig/05_backgrounds/scripts/make_dy_zpt_nlo_lo.py) by
// dividing the amcatnloFXFX sample by the sum of the HT-binned madgraphMLM
// samples. LO is held fixed; only the NLO numerator carries the theory
// variations, which is why docs/DYGenZpT.yaml lists theory nuisances only.
class DYGenZpT : public AnalyzerCore {
public:
    DYGenZpT();
    ~DYGenZpT();

    void initializeAnalyzer();
    void executeEvent();

    unique_ptr<SystematicHelper> systHelper;

    // Variable binning: fine where the spectrum is, coarse in the tail, and
    // running to 8000 GeV. The previous fixed 1 GeV / 0-1000 binning left C
    // undefined above 1000 GeV and, worse, undefined in most 1 GeV bins from
    // ~515 GeV up because the LO side was empty there -- the derivation then
    // fell back to C = 1 over that whole region. Empty bins are merged upward
    // downstream (make_dy_zpt_nlo_lo.py), which can only be done once the
    // spectrum is known, so the analyzer just has to provide edges wide enough
    // to be mergeable and a range that no event can fall off.
    static const RVec<float> &ZptBins();

    // Charged leptons of the hard process. statusFlags bit 7.
    //
    // NOT fromHardProcessBeforeFSR (bit 11): in this NanoAOD that bit is set on
    // only ~20% of the charged leptons and is absent from both legs in ~30% of
    // events. NOT LHE_Vpt either: it is the matrix-element boson pT, exactly
    // zero in 56% of the NLO events (0-jet ME), and so is not the showered
    // boson pT the reweighting is about.
    //
    // The bit 7 pair exists in exactly 2 copies in 100.00% of events, and where
    // the old (pdgId 23, status 62) definition also exists the two agree to a
    // median of 0.04 (NLO) / 0.09 (LO) GeV, max 0.9 GeV -- the same object.
    static constexpr int kBitIsHardProcess = 7;

    // The LO side is HT-binned and its lowest bin starts at HT = 40 GeV, so the
    // LO sum has no LHE_HT < 40 events at all. In the NLO inclusive sample that
    // region is 97% of the Z pT < 25 GeV events and 73% of 25-50 GeV, dropping
    // to exactly 0 above 100 GeV. The "_HT40" histograms below apply the same
    // requirement to both sides so the ratio is defined over the whole range;
    // the uncut ones are kept alongside so the choice can be made downstream
    // without paying for the event loop twice.
    static constexpr float kLHEHTCut = 40.;

private:
    // Emit the LHE-weight multiplicity warning at most once per job.
    bool fWarnedWeightMultiplicity = false;
};

#endif

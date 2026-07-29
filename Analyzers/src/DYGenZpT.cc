#include "DYGenZpT.h"

DYGenZpT::DYGenZpT() {}
DYGenZpT::~DYGenZpT() {}

const RVec<float> &DYGenZpT::ZptBins() {
    // 2 GeV to 200, 10 to 500, 50 to 1000, 250 to 2000, then 2000/3000/4000/
    // 6000/8000. The upper edges 500, 1000, 2000 and 8000 are present by
    // construction so the tail can be merged onto them downstream.
    static const RVec<float> bins = [] {
        RVec<float> b;
        for (float x = 0.;    x < 200.;  x += 2.)   b.push_back(x);
        for (float x = 200.;  x < 500.;  x += 10.)  b.push_back(x);
        for (float x = 500.;  x < 1000.; x += 50.)  b.push_back(x);
        for (float x = 1000.; x < 2000.; x += 250.) b.push_back(x);
        for (float x : {2000., 3000., 4000., 6000., 8000.}) b.push_back(x);
        return b;
    }();
    return bins;
}

void DYGenZpT::initializeAnalyzer() {
    string SKNANO_HOME = getenv("SKNANO_HOME");
    systHelper = std::make_unique<SystematicHelper>(SKNANO_HOME + "/docs/DYGenZpT.yaml", MCSample, DataEra);
}

void DYGenZpT::executeEvent() {
    // Gen-level only; there is nothing to fill for data.
    if (IsDATA) return;

    // AnalyzerCore::GetScaleVariation indexes LHEScaleWeight assuming 8 or 9
    // entries, and the PDF/alphaS envelopes need the 103-member LHEPdfWeight
    // block. Both hold for the DY samples, but a silent fallback to the nominal
    // weight is exactly the failure mode that produced the muF_Up == Central
    // histograms in the previous production, so say so loudly if it happens.
    if (!fWarnedWeightMultiplicity && (nLHEScaleWeight != 9 || nLHEPdfWeight != 103)) {
        cout << "[DYGenZpT::executeEvent] WARNING: nLHEScaleWeight = " << nLHEScaleWeight
             << ", nLHEPdfWeight = " << nLHEPdfWeight
             << " (expected 9 and 103). Theory variations may silently equal the nominal." << endl;
        fWarnedWeightMultiplicity = true;
    }

    const RVec<Gen> allgens = GetAllGens();

    // --- the definition the correction is derived and applied with:
    // the two charged leptons of the hard process, summed.
    //
    // This must find exactly two. Coverage is the whole premise of the
    // definition -- the reason for moving off (pdgId 23, status 62) was that it
    // silently dropped events -- so anything else aborts the job rather than
    // filling a wrong number or quietly skipping.
    float zpt_hardproc = -1.;
    float mll_hardproc = -1.;
    {
        RVec<Gen> hard;
        for (const auto &gen : allgens) {
            const int abspid = abs(gen.PID());
            if (abspid != 11 && abspid != 13 && abspid != 15) continue;
            if (!gen.isHardProcess()) continue;
            hard.emplace_back(gen);
        }
        if (hard.size() != 2) {
            cerr << "[DYGenZpT::executeEvent] FATAL: found " << hard.size()
                 << " isHardProcess charged leptons, expected exactly 2 "
                 << "(run " << RunNumber << ", event " << EventNumber << ", "
                 << "sample " << MCSample << ", era " << DataEra << "). "
                 << "The gen-Z definition assumes 100% coverage; it does not "
                 << "hold for this sample." << endl;
            exit(EXIT_FAILURE);
        }
        zpt_hardproc = (hard.at(0) + hard.at(1)).Pt();
        mll_hardproc = (hard.at(0) + hard.at(1)).M();
    }

    // --- the old definition, kept as a cross-check only.
    // (pdgId 23, status 62) is the outgoing last copy and is unique per event
    // when present. It is missing for the pure gamma* events -- about 6% of the
    // NLO sample and, cross-section weighted, 5% of the LO sum, but 83-89% of
    // the MLL > 120 samples on their own, where gamma*/Z interference dominates.
    // Those events are left out of the Zpt histograms and counted in the
    // cutflow instead. This is exactly the loss zpt_hardproc removes.
    float zpt = -1.;
    for (const auto &gen : allgens) {
        if (gen.PID() != 23) continue;
        if (gen.Status() != 62) continue;
        zpt = gen.Pt();
        break;
    }

    // --- dressed-lepton alternative, which also covers the gamma* events.
    // Highest-pT opposite-sign e/mu pair, taus excluded.
    float zpt_dressed = -1.;
    {
        RVec<GenDressedLepton> leps;
        for (const auto &lep : GetAllGenDressedLeptons()) {
            const int abspid = abs(lep.PdgId());
            if (abspid != 11 && abspid != 13) continue;
            if (lep.HasTauAnc()) continue;
            leps.emplace_back(lep);
        }
        std::sort(leps.begin(), leps.end(),
                  [](const GenDressedLepton &a, const GenDressedLepton &b) { return a.Pt() > b.Pt(); });
        for (size_t i = 0; i < leps.size() && zpt_dressed < 0.; i++) {
            for (size_t j = i + 1; j < leps.size(); j++) {
                // opposite sign: pdgId of a lepton and its antiparticle differ in sign
                if (leps.at(i).PdgId() * leps.at(j).PdgId() > 0) continue;
                zpt_dressed = (leps.at(i) + leps.at(j)).Pt();
                break;
            }
        }
    }

    const bool passHT = (LHE_HT > kLHEHTCut);

    // genWeight sign x xsec / sumSign. The NLO sample has a strictly constant
    // |genWeight| so the sign-based normalization is exact there; the LO
    // HT-binned samples vary by at most 0.3%.
    const float weight = MCweight();

    std::unordered_map<std::string, std::variant<std::function<float(MyCorrection::variation, TString)>, std::function<float()>>> weight_function_map;

    // Theory weight targets. Bodies are copied verbatim from
    // Reproduce20_002_copy.cc:188-213 -- the two must not drift apart, since the
    // correction derived here is applied to histograms produced there.
    // Each returns the absolute event weight for the variation (nom == 1).
    // muF: vary factorization scale, keep renormalization scale nominal.
    weight_function_map["ScaleWeight_muF"] = [&](MyCorrection::variation var, TString source) -> float {
        return GetScaleVariation(var, MyCorrection::variation::nom);
    };
    // muR: vary renormalization scale, keep factorization scale nominal.
    weight_function_map["ScaleWeight_muR"] = [&](MyCorrection::variation var, TString source) -> float {
        return GetScaleVariation(MyCorrection::variation::nom, var);
    };
    // PDF envelope: Hessian sum in quadrature over members 1..100 (LHEPdfWeight[0] is central == 1).
    weight_function_map["PDF_Weight"] = [&](MyCorrection::variation var, TString source) -> float {
        if (nLHEPdfWeight < 103) return 1.f;
        const float w0 = LHEPdfWeight[0];
        float sumSq = 0.f;
        for (int i = 1; i <= 100; i++) {
            const float dw = LHEPdfWeight[i] - w0;
            sumSq += dw * dw;
        }
        const float deltaPDF = sqrt(sumSq);
        if (var == MyCorrection::variation::up)   return w0 + deltaPDF;
        if (var == MyCorrection::variation::down) return w0 - deltaPDF;
        return 1.f;
    };
    // alpha_S: dedicated PDF members (101 = down, 102 = up).
    weight_function_map["AlphaS_Weight"] = [&](MyCorrection::variation var, TString source) -> float {
        if (nLHEPdfWeight < 103) return 1.f;
        if (var == MyCorrection::variation::up)   return LHEPdfWeight[102];
        if (var == MyCorrection::variation::down) return LHEPdfWeight[101];
        return 1.f;
    };

    systHelper->assignWeightFunctionMap(weight_function_map);

    // All four nuisances are weight-only, so this iterator has a single element
    // (Central) and calculateWeight() returns Central + 4 x {Up, Down}.
    for (const auto &syst_dummy : *systHelper) {
        for (const auto &[syst_name, sf] : systHelper->calculateWeight()) {
            const float w = weight * sf;
            const TString dir = TString(syst_name) + "/";

            // 0 = all, 1 = has gen Z (old definition), 2 = passes the LO HT
            // phase space, 3 = has a dressed-lepton pair, 4 = has the
            // isHardProcess pair (must equal bin 0 by construction; it is
            // filled anyway so the claim is checkable in the output).
            FillHist(dir + "cutflow", 0., w, 5, 0., 5.);
            if (zpt >= 0.)          FillHist(dir + "cutflow", 1., w, 5, 0., 5.);
            if (passHT)             FillHist(dir + "cutflow", 2., w, 5, 0., 5.);
            if (zpt_dressed >= 0.)  FillHist(dir + "cutflow", 3., w, 5, 0., 5.);
            if (zpt_hardproc >= 0.) FillHist(dir + "cutflow", 4., w, 5, 0., 5.);

            // Unweighted twin of the above, for raw-statistics checks.
            FillHist(dir + "cutflow_raw", 0., 1., 5, 0., 5.);
            if (zpt >= 0.)          FillHist(dir + "cutflow_raw", 1., 1., 5, 0., 5.);
            if (passHT)             FillHist(dir + "cutflow_raw", 2., 1., 5, 0., 5.);
            if (zpt_dressed >= 0.)  FillHist(dir + "cutflow_raw", 3., 1., 5, 0., 5.);
            if (zpt_hardproc >= 0.) FillHist(dir + "cutflow_raw", 4., 1., 5, 0., 5.);

            // The deliverable. The other two variants below are cross-checks:
            // overlaying the three C curves shows what the choice of definition
            // is worth.
            FillHist(dir + "Zpt_hardproc", zpt_hardproc, w, ZptBins());
            if (passHT)
                FillHist(dir + "Zpt_hardproc_HT40", zpt_hardproc, w, ZptBins());

            // Mass slices of the deliverable. C is derived in the Z peak but
            // applied in a search region that lives at high mass, so whether C
            // depends on m(ll) is a question about the extrapolation, not a
            // detail. The LO side splits cleanly: the MLL 50-120 samples feed
            // the first slice and the MLL > 120 samples the other two.
            if (passHT) {
                const char *slice = nullptr;
                if (mll_hardproc >= 50. && mll_hardproc < 120.)  slice = "M50to120";
                else if (mll_hardproc >= 120. && mll_hardproc < 200.) slice = "M120to200";
                else if (mll_hardproc >= 200.)                   slice = "M200toInf";
                if (slice)
                    FillHist(dir + "Zpt_hardproc_HT40_" + TString(slice),
                             zpt_hardproc, w, ZptBins());
            }

            if (zpt >= 0.) {
                FillHist(dir + "Zpt", zpt, w, ZptBins());
                if (passHT) FillHist(dir + "Zpt_HT40", zpt, w, ZptBins());
            }
            if (zpt_dressed >= 0.) {
                FillHist(dir + "Zpt_dressed", zpt_dressed, w, ZptBins());
                if (passHT) FillHist(dir + "Zpt_dressed_HT40", zpt_dressed, w, ZptBins());
            }
        }
    }
}

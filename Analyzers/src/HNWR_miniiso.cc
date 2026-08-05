#include "HNWR_miniiso.h"

#include <TFile.h>
#include <TH1.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <fstream>

#include <nlohmann/json.hpp>

HNWR_miniiso::HNWR_miniiso() {}
HNWR_miniiso::~HNWR_miniiso() {}  

void HNWR_miniiso::initializeAnalyzer() {
    // if signal ..  # 26 
    // kfactor  # 51
    el_set.AllElectrons.clear();
    mu_set.AllMuons.clear();
    jet_set.AllJets.clear();
    fatjet_set.AllFatJets.clear();
    gen_set.gens.clear();
    lhe_set.lhe_parts.clear();
    
    mu_set.Muon_Trigger.clear();
    mu_set.Muon_Trigger_Safe_Pt_Cut = 0.;
    el_set.Ele_Trigger.clear();
    el_set.Ele_Trigger_Safe_Pt_Cut = 0.;

    if ( DataEra=="2017")
    {
        mu_set.Muon_Trigger = {"HLT_Mu50", "HLT_OldMu100", "HLT_TkMu100"}; 
        mu_set.Muon_Trigger_Safe_Pt_Cut = 52.;
        el_set.Ele_Trigger = {"HLT_Ele35_WPTight_Gsf","HLT_Photon200","HLT_Ele115_CaloIdVT_GsfTrkIdT"};
        el_set.Ele_Trigger_Safe_Pt_Cut = 38.;  
    }

    if (DataEra == "2022")
    {
        mu_set.Muon_Trigger = {"HLT_Mu50", "HLT_CascadeMu100", "HLT_HighPtTkMu100"};
        mu_set.Muon_Trigger_Safe_Pt_Cut = 52.;
        el_set.Ele_Trigger = {"HLT_Photon200","HLT_Ele115_CaloIdVT_GsfTrkIdT"};
        el_set.Ele_Trigger_Safe_Pt_Cut = 118.;
    }
    if (DataEra == "2022EE")
    {
        mu_set.Muon_Trigger = {"HLT_Mu50", "HLT_CascadeMu100", "HLT_HighPtTkMu100"};
        mu_set.Muon_Trigger_Safe_Pt_Cut = 52.;
        el_set.Ele_Trigger = {"HLT_Photon200","HLT_Ele115_CaloIdVT_GsfTrkIdT"};
        el_set.Ele_Trigger_Safe_Pt_Cut = 118.;
    }
    if (DataEra == "2023")
    {
        mu_set.Muon_Trigger = {"HLT_Mu50", "HLT_CascadeMu100", "HLT_HighPtTkMu100"};
        mu_set.Muon_Trigger_Safe_Pt_Cut = 52.;
        el_set.Ele_Trigger = {"HLT_Photon200","HLT_Ele115_CaloIdVT_GsfTrkIdT"};
        el_set.Ele_Trigger_Safe_Pt_Cut = 118.;
    }
    if (DataEra == "2023BPix")
    {
        mu_set.Muon_Trigger = {"HLT_Mu50", "HLT_CascadeMu100", "HLT_HighPtTkMu100"};
        mu_set.Muon_Trigger_Safe_Pt_Cut = 52.;
        el_set.Ele_Trigger = {"HLT_Photon200","HLT_Ele115_CaloIdVT_GsfTrkIdT"};
        el_set.Ele_Trigger_Safe_Pt_Cut = 118.;
    }

    myCorr = new MyCorrection(DataEra, DataPeriod, IsDATA ? DataStream : MCSample, IsDATA);

    
    // Initialize systematic helper
    string SKNANO_HOME = getenv("SKNANO_HOME");
    if (IsDATA) {
        systHelper = std::make_unique<SystematicHelper>(SKNANO_HOME + "/docs/DataLRSM.yaml", DataStream, DataEra);
    } else {
        systHelper = std::make_unique<SystematicHelper>(SKNANO_HOME + "/docs/MCLRSM_miniiso.yaml", MCSample, DataEra);
    }

    NoDYCorr = HasFlag("NoDYCorr");
    RunXsecSyst = HasFlag("RunXsecSyst");
    ZptOnly = HasFlag("ZptOnly");
    if (NoDYCorr && ZptOnly) {
        cerr << "[HNWR_miniiso] FATAL: NoDYCorr and ZptOnly are mutually "
             << "exclusive. Pass one or neither." << endl;
        exit(EXIT_FAILURE);
    }

    LoadDYCorrections();
    LoadTheoryNormK();
}

// Reads the per-sample inclusive normalisation K_var used to make the four theory
// nuisances acceptance-only. Missing file or missing entry is NOT an error: the table
// only covers signal, and everything else keeps K = 1.
void HNWR_miniiso::LoadTheoryNormK() {
    theoryK_scale.clear();
    theoryK_pdf.clear();
    if (IsDATA) return;

    const char *datadir = getenv("SKNANO_DATA");
    if (!datadir) {
        cerr << "[HNWR_miniiso] WARNING: SKNANO_DATA unset; theory nuisances stay "
             << "un-normalised (inclusive xsec change left in)." << endl;
        return;
    }
    const std::string path =
        std::string(datadir) + "/" + std::string(DataEra.Data()) + "/HNWR/TheoryNormK.json";
    std::ifstream f(path);
    if (!f.is_open()) {
        cout << "[HNWR_miniiso] no theory-K table at " << path
             << "; theory nuisances stay un-normalised." << endl;
        return;
    }

    nlohmann::json j;
    try {
        f >> j;
    } catch (const std::exception &e) {
        cerr << "[HNWR_miniiso] FATAL: cannot parse " << path << ": " << e.what() << endl;
        exit(EXIT_FAILURE);
    }

    const std::string key = std::string(MCSample.Data());
    if (!j.contains("samples") || !j["samples"].contains(key)) {
        cout << "[HNWR_miniiso] " << MCSample << " not in " << path
             << " (background, or a signal alias without the full 103 PDF members); "
             << "theory nuisances stay un-normalised." << endl;
        return;
    }

    const auto &s = j["samples"][key];
    if (s.contains("scale")) theoryK_scale = s["scale"].get<std::vector<float>>();
    if (s.contains("pdf"))   theoryK_pdf   = s["pdf"].get<std::vector<float>>();

    // Sizes must match what the weight targets index into. A short table would silently
    // fall back to K = 1 for the missing entries and mix normalised with un-normalised
    // members inside the same PDF envelope, so refuse it outright.
    if (theoryK_scale.size() != 9 || theoryK_pdf.size() != 103) {
        cerr << "[HNWR_miniiso] FATAL: " << MCSample << " in " << path
             << " has scale[" << theoryK_scale.size() << "] pdf[" << theoryK_pdf.size()
             << "], expected scale[9] pdf[103]." << endl;
        exit(EXIT_FAILURE);
    }

    cout << "[HNWR_miniiso] theory-K loaded for " << MCSample << " (" << DataEra
         << "): muF idx3/idx5 = " << theoryK_scale[3] << "/" << theoryK_scale[5]
         << ", alphaS idx101/102 = " << theoryK_pdf[101] << "/" << theoryK_pdf[102]
         << ". Theory nuisances are acceptance-only." << endl;
}

float HNWR_miniiso::GetTheoryNormK(const std::vector<float> &K, int idx) const {
    if (idx < 0 || idx >= static_cast<int>(K.size())) return 1.f;
    const float k = K[idx];
    return (std::isfinite(k) && k > 0.f) ? k : 1.f;
}

// eta MUST be the supercluster eta (Electron::scEta()), not the track eta: the
// barrel/endcap split is at the SC crack edge |eta| = 1.4442.
// Constant HLT SF from the EGM high-pT electron trigger T&P (fit range pT > 130 GeV):
//   era        barrel          endcap
//   2022       0.995 +- 0.004  0.991 +- 0.009
//   2022EE     0.990 +- 0.007  0.981 +- 0.017
//   2023       0.992 +- 0.006  0.979 +- 0.019
//   2023BPix   0.993 +- 0.001  0.978 +- 0.019
// ---------------------------------------------------------------------------
// DY corrections: C(gen Z pT), EW(gen Z pT) and R(reco jet pT)
// ---------------------------------------------------------------------------
namespace {
// Paths are overridable so a re-derived correction can be tested without a
// rebuild. Defaults point at the staging area the derivation writes to.
const char *kDefaultZptFile =
    "/data9/Users/achihwan/25-020/AN-25-020/chihwan/fig/05_backgrounds/"
    "corrections/zpt/dy_zpt_nlo_lo.root";
const char *kDefaultRFile =
    "/data9/Users/achihwan/25-020/AN-25-020/chihwan/fig/05_backgrounds/"
    "corrections/jetpt/dy_jetpt_ratio.root";
// NLO EW correction EW(genZpT), arXiv:1705.04664, copied verbatim from
// SKFlatAnalyzer data/Run2Legacy_v4/2018/HNWRDYPtReweight/ZPtEWCorr.root
// (the 2016/2017/2018 files are byte-identical: pure theory, era-independent,
// so unlike C and R there is no per-era directory inside).
const char *kDefaultEWFile =
    "/data9/Users/achihwan/25-020/AN-25-020/chihwan/fig/05_backgrounds/"
    "corrections/zpt_ew/ZPtEWCorr.root";
// isHardProcess lepton pair, LHE_HT > 40 -- the variant C is delivered in.
const char *kZptVariant = "Zpt_hardproc_HT40";

// Read a TH1 into (edges, values, errors).
bool ReadTH1(TFile *f, const TString &path, std::vector<double> &edges,
             std::vector<double> &vals, std::vector<double> *errs = nullptr) {
    TH1 *h = dynamic_cast<TH1 *>(f->Get(path));
    if (!h) return false;
    const int n = h->GetNbinsX();
    edges.resize(n + 1);
    vals.resize(n);
    if (errs) errs->resize(n);
    for (int i = 1; i <= n; i++) {
        edges[i - 1] = h->GetBinLowEdge(i);
        vals[i - 1] = h->GetBinContent(i);
        if (errs) (*errs)[i - 1] = h->GetBinError(i);
    }
    edges[n] = h->GetBinLowEdge(n) + h->GetBinWidth(n);
    return true;
}

int FindBin(const std::vector<double> &edges, double x) {
    if (edges.size() < 2) return -1;
    if (x >= edges.back()) return static_cast<int>(edges.size()) - 2;  // clamp
    if (x < edges.front()) return 0;
    int i = 0;
    while (i + 2 < static_cast<int>(edges.size()) && x >= edges[i + 1]) i++;
    return i;
}

using ElMiniIsoSFTable = std::array<std::array<float, 6>, 9>;

// egamma-tnp mini-isolation (miniPFRelIso_all < 0.1) Tag&Probe, SIGNAL+BACKGROUND FIT.
// This is the SF for the subleading loose-lepton miniIso cut that replaced LSF3 > 0.75
// in the boosted SR (fatjet_set.Sublead_MiniIso).
// Probe baseline: HNWR loose electron = (cutBased Loose WP minus isolation cut) OR HEEP,
//   implemented from Electron_vidNestedWPBitmap (cut 7 skipped) | Electron_cutBased_HEEP,
//   i.e. the same object as the analysis loose electron (isPassLooseNoIso || cutBased_HEEP,
//   HNWR_miniiso::Electrons::isPassLooseNoIso).
// Source: /data6/Users/achihwan/tnp/egamma-tnp/Mini_iso/MiniIso_fit/bpoly/<era>/miniiso_fit_results_<era>_bpoly.json
// MC is PU-reweighted at histogram level (puWeight*genWeight); DATA is unweighted.
// Fit: double Crystal Ball signal + Bernstein background, 50 < mll < 130 GeV.
// Errors: fit stat (+) background-model systematic |SF(bpoly) - SF(cms)| in quadrature.
//   Bins where the cms fit broke down (wrong minimum, eps parked at 1, singular
//   Hessian) take the era's median systematic instead.
// Binning: el_eta_bins = [-2.5,-1.566,-1.4442,0.,1.4442,1.566,2.5] (6 bins)
//          el_pt_bins  = [53,60,70,80,100,150,200,300,500,1000] (9 bins)
// Crack columns (|eta| 1.4442-1.566) have no T&P data -> 1.0 +- 0.
// The last two pt rows (300-500, 500-1000) hold the same merged 300-1000 refit:
//   500-1000 has too few probes to fit on its own in every era.
// See Mini_iso/MINIISO_SF_LOG.md for the full derivation and per-era fit quality.

// 2022: median systematic substituted in 5 bin(s): bin00, bin44, bin47, bin50, bin53
const ElMiniIsoSFTable ElMiniIsoSF_2022 = {{
    {1.0011f, 1.0000f, 0.9991f, 0.9979f, 1.0000f, 1.0008f},
    {0.9991f, 1.0000f, 0.9997f, 0.9989f, 1.0000f, 0.9955f},
    {1.0003f, 1.0000f, 1.0026f, 1.0011f, 1.0000f, 1.0000f},
    {1.0025f, 1.0000f, 1.0006f, 1.0040f, 1.0000f, 1.0020f},
    {1.0007f, 1.0000f, 1.0013f, 0.9918f, 1.0000f, 0.9971f},
    {0.9984f, 1.0000f, 0.9869f, 0.9874f, 1.0000f, 0.9987f},
    {1.0007f, 1.0000f, 1.0037f, 0.9841f, 1.0000f, 1.0000f},
    {1.0000f, 1.0000f, 0.9847f, 1.0361f, 1.0000f, 1.0000f},
    {1.0000f, 1.0000f, 0.9847f, 1.0361f, 1.0000f, 1.0000f},
}};
const ElMiniIsoSFTable ElMiniIsoSFErr_2022 = {{
    {0.0014f, 0.0000f, 0.0010f, 0.0011f, 0.0000f, 0.0014f},
    {0.0015f, 0.0000f, 0.0071f, 0.0013f, 0.0000f, 0.0017f},
    {0.0028f, 0.0000f, 0.0022f, 0.0020f, 0.0000f, 0.0024f},
    {0.0038f, 0.0000f, 0.0022f, 0.0023f, 0.0000f, 0.0025f},
    {0.0022f, 0.0000f, 0.0041f, 0.0039f, 0.0000f, 0.0013f},
    {0.0041f, 0.0000f, 0.0090f, 0.0096f, 0.0000f, 0.0021f},
    {0.0036f, 0.0000f, 0.0120f, 0.0070f, 0.0000f, 0.0056f},
    {0.0087f, 0.0000f, 0.0163f, 0.0260f, 0.0000f, 0.0077f},
    {0.0087f, 0.0000f, 0.0163f, 0.0260f, 0.0000f, 0.0077f},
}};

// 2022EE: median systematic substituted in 2 bin(s): bin47, bin53
const ElMiniIsoSFTable ElMiniIsoSF_2022EE = {{
    {1.0007f, 1.0000f, 0.9957f, 0.9944f, 1.0000f, 1.0021f},
    {1.0003f, 1.0000f, 0.9977f, 0.9968f, 1.0000f, 1.0000f},
    {1.0023f, 1.0000f, 0.9995f, 1.0012f, 1.0000f, 0.9997f},
    {1.0014f, 1.0000f, 0.9978f, 1.0000f, 1.0000f, 1.0019f},
    {0.9988f, 1.0000f, 0.9970f, 0.9946f, 1.0000f, 0.9982f},
    {1.0010f, 1.0000f, 0.9996f, 0.9809f, 1.0000f, 0.9994f},
    {1.0000f, 1.0000f, 0.9914f, 0.9867f, 1.0000f, 1.0000f},
    {0.9946f, 1.0000f, 1.0105f, 0.9843f, 1.0000f, 1.0000f},
    {0.9946f, 1.0000f, 1.0105f, 0.9843f, 1.0000f, 1.0000f},
}};
const ElMiniIsoSFTable ElMiniIsoSFErr_2022EE = {{
    {0.0010f, 0.0000f, 0.0007f, 0.0010f, 0.0000f, 0.0008f},
    {0.0009f, 0.0000f, 0.0009f, 0.0010f, 0.0000f, 0.0059f},
    {0.0014f, 0.0000f, 0.0011f, 0.0012f, 0.0000f, 0.0014f},
    {0.0012f, 0.0000f, 0.0012f, 0.0013f, 0.0000f, 0.0015f},
    {0.0011f, 0.0000f, 0.0021f, 0.0023f, 0.0000f, 0.0012f},
    {0.0018f, 0.0000f, 0.0050f, 0.0049f, 0.0000f, 0.0018f},
    {0.0006f, 0.0000f, 0.0063f, 0.0050f, 0.0000f, 0.0025f},
    {0.0076f, 0.0000f, 0.0085f, 0.0052f, 0.0000f, 0.0033f},
    {0.0076f, 0.0000f, 0.0085f, 0.0052f, 0.0000f, 0.0033f},
}};

// 2023: median systematic substituted in 2 bin(s): bin47, bin53
const ElMiniIsoSFTable ElMiniIsoSF_2023 = {{
    {1.0005f, 1.0000f, 1.0001f, 1.0002f, 1.0000f, 1.0011f},
    {1.0021f, 1.0000f, 1.0004f, 0.9999f, 1.0000f, 0.9990f},
    {0.9989f, 1.0000f, 1.0030f, 0.9987f, 1.0000f, 1.0016f},
    {0.9977f, 1.0000f, 0.9988f, 1.0001f, 1.0000f, 0.9987f},
    {0.9984f, 1.0000f, 0.9971f, 0.9883f, 1.0000f, 1.0021f},
    {1.0017f, 1.0000f, 0.9830f, 0.9853f, 1.0000f, 1.0027f},
    {1.0021f, 1.0000f, 0.9936f, 0.9998f, 1.0000f, 0.9983f},
    {1.0000f, 1.0000f, 1.0172f, 1.0108f, 1.0000f, 1.0000f},
    {1.0000f, 1.0000f, 1.0172f, 1.0108f, 1.0000f, 1.0000f},
}};
const ElMiniIsoSFTable ElMiniIsoSFErr_2023 = {{
    {0.0010f, 0.0000f, 0.0035f, 0.0016f, 0.0000f, 0.0013f},
    {0.0070f, 0.0000f, 0.0011f, 0.0098f, 0.0000f, 0.0014f},
    {0.0019f, 0.0000f, 0.0016f, 0.0017f, 0.0000f, 0.0020f},
    {0.0013f, 0.0000f, 0.0017f, 0.0017f, 0.0000f, 0.0015f},
    {0.0015f, 0.0000f, 0.0031f, 0.0028f, 0.0000f, 0.0026f},
    {0.0041f, 0.0000f, 0.0059f, 0.0063f, 0.0000f, 0.0048f},
    {0.0035f, 0.0000f, 0.0077f, 0.0074f, 0.0000f, 0.0076f},
    {0.0006f, 0.0000f, 0.0139f, 0.0125f, 0.0000f, 0.0050f},
    {0.0006f, 0.0000f, 0.0139f, 0.0125f, 0.0000f, 0.0050f},
}};

const ElMiniIsoSFTable ElMiniIsoSF_2023BPix = {{
    {0.9994f, 1.0000f, 1.0024f, 0.9999f, 1.0000f, 1.0015f},
    {1.0034f, 1.0000f, 1.0013f, 1.0007f, 1.0000f, 1.0000f},
    {1.0039f, 1.0000f, 0.9999f, 1.0000f, 1.0000f, 1.0045f},
    {1.0001f, 1.0000f, 0.9980f, 1.0002f, 1.0000f, 1.0025f},
    {1.0040f, 1.0000f, 0.9898f, 0.9968f, 1.0000f, 1.0011f},
    {1.0040f, 1.0000f, 1.0030f, 0.9828f, 1.0000f, 0.9977f},
    {1.0000f, 1.0000f, 1.0006f, 1.0039f, 1.0000f, 0.9936f},
    {1.0000f, 1.0000f, 0.9945f, 1.0279f, 1.0000f, 1.0000f},
    {1.0000f, 1.0000f, 0.9945f, 1.0279f, 1.0000f, 1.0000f},
}};
const ElMiniIsoSFTable ElMiniIsoSFErr_2023BPix = {{
    {0.0015f, 0.0000f, 0.0010f, 0.0037f, 0.0000f, 0.0013f},
    {0.0019f, 0.0000f, 0.0013f, 0.0013f, 0.0000f, 0.0055f},
    {0.0025f, 0.0000f, 0.0020f, 0.0019f, 0.0000f, 0.0026f},
    {0.0024f, 0.0000f, 0.0020f, 0.0022f, 0.0000f, 0.0026f},
    {0.0030f, 0.0000f, 0.0039f, 0.0038f, 0.0000f, 0.0027f},
    {0.0050f, 0.0000f, 0.0092f, 0.0082f, 0.0000f, 0.0039f},
    {0.0043f, 0.0000f, 0.0119f, 0.0100f, 0.0000f, 0.0046f},
    {0.0099f, 0.0000f, 0.0127f, 0.0196f, 0.0000f, 0.0089f},
    {0.0099f, 0.0000f, 0.0127f, 0.0196f, 0.0000f, 0.0089f},
}};
}  // namespace

bool HNWR_miniiso::IsDYSample() const {
    return !IsDATA && MCSample.Contains("DYMLL");
}

void HNWR_miniiso::LoadDYCorrections() {
    dycorr = DYCorrections();
    if (!IsDYSample()) return;
    if (NoDYCorr || getenv("DY_NO_CORRECTION")) {
        cout << "[HNWR_miniiso] DY corrections OFF for " << MCSample
             << " / " << DataEra << " (C=off R=off), requested by "
             << (NoDYCorr ? "userflag NoDYCorr" : "DY_NO_CORRECTION") << "." << endl;
        return;
    }

    // --- C
    const char *zenv = getenv("DY_ZPT_CORRECTION");
    const TString zpath = zenv ? zenv : kDefaultZptFile;
    unique_ptr<TFile> zf(TFile::Open(zpath));
    if (!zf || zf->IsZombie()) {
        cerr << "[HNWR_miniiso] FATAL: cannot open " << zpath << endl;
        exit(EXIT_FAILURE);
    }
    // Era-specific. There is no combined fallback: reweighting one era with
    // another's curve would be silently wrong.
    const TString zdir = TString(kZptVariant) + "/" + DataEra + "/";
    for (const char *key : {"ZPTReweight", "ZPTReweight_Up", "ZPTReweight_Down",
                            "ZPTReweight_QCDScaleUp", "ZPTReweight_QCDScaleDown",
                            "ZPTReweight_QCDPDFErrorUp", "ZPTReweight_QCDPDFErrorDown",
                            "ZPTReweight_QCDPDFAlphaSUp", "ZPTReweight_QCDPDFAlphaSDown"}) {
        std::vector<double> e, v, s;
        if (!ReadTH1(zf.get(), zdir + key, e, v, &s)) {
            cerr << "[HNWR_miniiso] FATAL: " << zdir << key << " missing in "
                 << zpath << ". C is delivered for 2022, 2022EE, 2023 and "
                 << "2023BPix; re-run make_dy_zpt_nlo_lo.py if an era is absent."
                 << endl;
            exit(EXIT_FAILURE);
        }
        if (dycorr.zpt_edges.empty()) dycorr.zpt_edges = e;
        dycorr.zpt[key] = v;
        // The MC statistical error of C is carried by the bin errors of the
        // nominal histogram alone; every other key is written with zero errors
        // (make_dy_zpt_nlo_lo.py:597-603). ZPTReweight_Up/Down is the quadrature
        // sum of the three theory groups and contains no statistical component,
        // which is why the ZPtRw_MC_stat nuisance is built from this instead.
        if (std::string(key) == "ZPTReweight") dycorr.zpt_stat = s;
    }

    // --- R. Rebinned curves only; the bin error is sigma_R, the single band
    // that already contains statistics and the non-DY cross sections.
    // The ZptOnly userflag skips R entirely rather than loading it and
    // multiplying by 1: n_nuis_res/boo then stay 0, so the DYReshape nuisances
    // disappear too, which is what "R is not applied" has to mean.
    const char *renv = getenv("DY_JETPT_CORRECTION");
    const TString rpath = ZptOnly ? "(not loaded)" : (renv ? renv : kDefaultRFile);
    if (!ZptOnly) {
        unique_ptr<TFile> rf(TFile::Open(rpath));
        if (!rf || rf->IsZombie()) {
            cerr << "[HNWR_miniiso] FATAL: cannot open " << rpath << endl;
            exit(EXIT_FAILURE);
        }
        // Era-specific, like C. The "_comb" in the directory name is the flavour
        // sum: ee and mumu agree within one sigma, so no flavour-split correction
        // is used. The era, however, is not summed over -- in the resolved category
        // the four eras scatter around a common R by chi2/ndf = 3.3, too much for
        // their statistics, so a combined curve would be wrong by more than its own
        // band (make_dy_jetpt_ratio.py writes that test into its run log). The file
        // still carries a "combined" key for that comparison; reading it here would
        // silently apply a four-era average.
        for (int cat = 0; cat < 2; cat++) {
            const TString dir =
                TString(cat == 0 ? "resolved_comb/" : "boosted_comb/") + DataEra + "/";
            std::vector<double> &edges = cat == 0 ? dycorr.r_edges_res : dycorr.r_edges_boo;
            std::vector<double> &vals = cat == 0 ? dycorr.r_val_res : dycorr.r_val_boo;
            std::vector<double> &sigs = cat == 0 ? dycorr.r_sig_res : dycorr.r_sig_boo;
            // make_dy_jetpt_ratio.py writes "rebinned" as a directory, and puts
            // sigma_R (= R_total, stat + non-DY xsec) into R_nominal's bin errors.
            if (!ReadTH1(rf.get(), dir + "rebinned/R_nominal", edges, vals, &sigs)) {
                cerr << "[HNWR_miniiso] FATAL: " << dir
                     << "rebinned/R_nominal missing in " << rpath << endl;
                exit(EXIT_FAILURE);
            }
            int n = 0;
            for (double v : vals) if (std::isfinite(v) && v > 0.) n++;
            (cat == 0 ? dycorr.n_nuis_res : dycorr.n_nuis_boo) = n;
        }
    }

    // --- EW. Applied alongside C exactly as in SKFlat HNWRAnalyzer: whenever
    // the Z-pT correction is on, EW is on (ZptOnly turns off R, not EW). The
    // nominal sits in the bin CONTENTS of hist_v; the three uncertainty sources
    // of arXiv:1705.04664 sit in the bin ERRORS of hist_e1/e2/e3 (their
    // contents duplicate hist_v and are not read).
    const char *ewenv = getenv("DY_ZPT_EW_CORRECTION");
    const TString ewpath = ewenv ? ewenv : kDefaultEWFile;
    unique_ptr<TFile> ewf(TFile::Open(ewpath));
    if (!ewf || ewf->IsZombie()) {
        cerr << "[HNWR_miniiso] FATAL: cannot open " << ewpath << endl;
        exit(EXIT_FAILURE);
    }
    {
        std::vector<double> dummy_e, dummy_v;
        if (!ReadTH1(ewf.get(), "hist_v", dycorr.ew_edges, dycorr.ew_val) ||
            !ReadTH1(ewf.get(), "hist_e1", dummy_e, dummy_v, &dycorr.ew_e1) ||
            !ReadTH1(ewf.get(), "hist_e2", dummy_e, dummy_v, &dycorr.ew_e2) ||
            !ReadTH1(ewf.get(), "hist_e3", dummy_e, dummy_v, &dycorr.ew_e3)) {
            cerr << "[HNWR_miniiso] FATAL: hist_v/e1/e2/e3 missing in "
                 << ewpath << endl;
            exit(EXIT_FAILURE);
        }
    }

    dycorr.loaded = true;
    dycorr.apply = true;
    // With ZptOnly the R curves were never read, so n_nuis_res/boo stay 0 and
    // the DYReshape nuisance loops iterate zero times. That is the intent: no R
    // means no R nuisances.
    dycorr.apply_r = !ZptOnly;
    dycorr.apply_ew = true;
    cout << "[HNWR_miniiso] DY corrections on for " << MCSample << " / "
         << DataEra << ": C=on R=" << (dycorr.apply_r ? "on" : "off (userflag ZptOnly)")
         << " EW=on, C from " << zpath << " (" << dycorr.zpt_edges.size() - 1
         << " bins), R from " << rpath << " (" << dycorr.n_nuis_res
         << " resolved + " << dycorr.n_nuis_boo << " boosted nuisance bins)"
         << ", EW from " << ewpath << " (" << dycorr.ew_edges.size() - 1
         << " bins)" << endl;
}

float HNWR_miniiso::GetGenZpT() const {
    RVec<Gen> hard;
    for (const auto &gen : gen_set.gens) {
        const int abspid = abs(gen.PID());
        if (abspid != 11 && abspid != 13 && abspid != 15) continue;
        if (!gen.isHardProcess()) continue;
        hard.emplace_back(gen);
    }
    if (hard.size() != 2) return -1.;
    return (hard.at(0) + hard.at(1)).Pt();
}

float HNWR_miniiso::GetZptWeight(float gen_zpt, const std::string &key) const {
    if (!dycorr.apply || gen_zpt < 0.) return 1.f;
    auto it = dycorr.zpt.find(key);
    if (it == dycorr.zpt.end() || it->second.empty()) return 1.f;
    const int i = FindBin(dycorr.zpt_edges, gen_zpt);
    if (i < 0 || i >= static_cast<int>(it->second.size())) return 1.f;
    const double c = it->second[i];
    if (!std::isfinite(c) || c <= 0.) return 1.f;
    return static_cast<float>(c);
}

float HNWR_miniiso::GetZptStat(float gen_zpt) const {
    if (!dycorr.apply || gen_zpt < 0.) return 0.f;
    if (dycorr.zpt_stat.empty()) return 0.f;
    const int i = FindBin(dycorr.zpt_edges, gen_zpt);
    if (i < 0 || i >= static_cast<int>(dycorr.zpt_stat.size())) return 0.f;
    const double s = dycorr.zpt_stat[i];
    // NaN where C is NaN (empty LO denominator). GetZptWeight falls back to 1
    // there, so the variation has to be 0 to stay on top of that fallback.
    if (!std::isfinite(s) || s < 0.) return 0.f;
    return static_cast<float>(s);
}

float HNWR_miniiso::GetZptEW(float gen_zpt, int which, int dir) const {
    if (!dycorr.apply || !dycorr.apply_ew || gen_zpt < 0.) return 1.f;
    if (dycorr.ew_val.empty()) return 1.f;
    // FindBin clamps both sides, which reproduces the SKFlat clipping
    // (pT<30 -> first bin, pT>=6500 -> last bin) without explicit ifs.
    const int i = FindBin(dycorr.ew_edges, gen_zpt);
    if (i < 0 || i >= static_cast<int>(dycorr.ew_val.size())) return 1.f;
    double v = dycorr.ew_val[i];
    if (!std::isfinite(v) || v <= 0.) return 1.f;
    if (which >= 1 && which <= 3 && dir != 0) {
        const std::vector<double> &errs =
            which == 1 ? dycorr.ew_e1 : which == 2 ? dycorr.ew_e2 : dycorr.ew_e3;
        if (i < static_cast<int>(errs.size()) && std::isfinite(errs[i]))
            v += dir * errs[i];
        // Same guard as the C stat variation: never hand back <= 0, that
        // would delete the event instead of down-weighting it.
        if (v <= 0.) v = 1e-3;
    }
    return static_cast<float>(v);
}

float HNWR_miniiso::GetJetPtR(bool resolved, float pt, int nuis_bin, int dir) const {
    if (!dycorr.apply || !dycorr.apply_r || pt < 0.) return 1.f;
    const std::vector<double> &edges = resolved ? dycorr.r_edges_res : dycorr.r_edges_boo;
    const std::vector<double> &vals = resolved ? dycorr.r_val_res : dycorr.r_val_boo;
    const std::vector<double> &sigs = resolved ? dycorr.r_sig_res : dycorr.r_sig_boo;
    if (vals.empty()) return 1.f;
    const int i = FindBin(edges, pt);
    if (i < 0 || i >= static_cast<int>(vals.size())) return 1.f;

    double r = vals[i];
    // Undefined bins are NaN in the file. 1.0 means "no correction"; 0 would
    // delete the DY prediction instead.
    if (!std::isfinite(r) || r <= 0.) return 1.f;

    // Per-bin nuisance: only the requested bin moves.
    if (nuis_bin >= 0 && nuis_bin == i && dir != 0) {
        const double s = (i < static_cast<int>(sigs.size()) && std::isfinite(sigs[i]))
                             ? sigs[i] : 0.;
        r += (dir > 0 ? s : -s);
        if (!std::isfinite(r) || r <= 0.) return 1.f;
    }
    return static_cast<float>(r);
}

float HNWR_miniiso::GetElectronTriggerSF_TnP(double eta, double pt, MyCorrection::variation var) const {
    const bool isBarrel = std::fabs(eta) < 1.4442;

    float sf, err;
    if      (DataEra == "2022")     { sf = isBarrel ? 0.995f : 0.991f; err = isBarrel ? 0.004f : 0.009f; }
    else if (DataEra == "2022EE")   { sf = isBarrel ? 0.990f : 0.981f; err = isBarrel ? 0.007f : 0.017f; }
    else if (DataEra == "2023")     { sf = isBarrel ? 0.992f : 0.979f; err = isBarrel ? 0.006f : 0.019f; }
    else if (DataEra == "2023BPix") { sf = isBarrel ? 0.993f : 0.978f; err = isBarrel ? 0.001f : 0.019f; }
    else return 1.0; // no measurement for this era (e.g. 2017)

    if (var == MyCorrection::variation::up)   return sf + err;
    if (var == MyCorrection::variation::down) return sf - err;
    return sf;
}

// eta MUST be the supercluster eta (Electron::scEta()): the EGM SF is binned in signed
// SC eta with crack bins at |eta| 1.4442-1.566.
// High-pT electron ID SF from POG/EGM/<era>/electronID_highPt.json.gz
// ("Electron-ID-SF", WP "Tight"). pt bins start at 100 GeV with clamp flow,
// so pt < 100 uses the first bin.
float HNWR_miniiso::GetElectronHEEPIDSF_TnP(double eta, double pt, MyCorrection::variation var) const {
    if (DataEra != "2022" && DataEra != "2022EE" && DataEra != "2023" && DataEra != "2023BPix")
        return 1.0; // no EGM high-pT ID SF for this era (e.g. 2017)
    return myCorr->GetElectronHighPtIDSF(eta, pt, var);
}

// Mini-isolation SF for the subleading loose lepton in the fatjet, the cut that
// replaces LSF3 > 0.75 here. ELECTRONS ONLY: egamma-tnp measured this for electrons,
// there is no muon equivalent, so only the regions whose in-fatjet loose lepton is an
// electron (boosted SR EE, flavour CR mu-ejet) get a SF. It is applied through the
// MiniIso_Weight target -- see executeEventFromParameter().
float HNWR_miniiso::GetElectronMiniIsoSF_TnP(double eta, double pt, MyCorrection::variation var) const {
    static const double eta_edges[7] = {-2.5, -1.566, -1.4442, 0.0, 1.4442, 1.566, 2.5};
    static const double pt_edges[10] = {53, 60, 70, 80, 100, 150, 200, 300, 500, 1000};

    const ElMiniIsoSFTable *sf_table, *err_table;
    if      (DataEra == "2022")     { sf_table = &ElMiniIsoSF_2022;     err_table = &ElMiniIsoSFErr_2022; }
    else if (DataEra == "2022EE")   { sf_table = &ElMiniIsoSF_2022EE;   err_table = &ElMiniIsoSFErr_2022EE; }
    else if (DataEra == "2023")     { sf_table = &ElMiniIsoSF_2023;     err_table = &ElMiniIsoSFErr_2023; }
    else if (DataEra == "2023BPix") { sf_table = &ElMiniIsoSF_2023BPix; err_table = &ElMiniIsoSFErr_2023BPix; }
    else return 1.0; // no egamma-tnp miniIso measurement for this era (e.g. 2017)

    int eta_idx = 5;
    for (int i = 0; i < 6; i++) { if (eta < eta_edges[i+1]) { eta_idx = i; break; } }

    // pt below the first measured bin (T&P probe cut is 50, first bin 53-60) uses the
    // first bin; the analysis loose-lepton cut keeps leptons above 53 anyway.
    int pt_idx = 8;
    for (int i = 0; i < 9; i++) { if (pt < pt_edges[i+1]) { pt_idx = i; break; } }

    const float sf  = (*sf_table)[pt_idx][eta_idx];
    const float err = (*err_table)[pt_idx][eta_idx];

    if (var == MyCorrection::variation::up)   return sf + err;
    if (var == MyCorrection::variation::down) return sf - err;
    return sf;
}

void HNWR_miniiso::executeEvent() {

    el_set.AllElectrons =  GetAllElectrons();
    mu_set.AllMuons = GetAllMuons();
    jet_set.AllJets = GetAllJets();
    fatjet_set.AllFatJets = GetAllFatJets();
    gen_set.gens = GetAllGens();
    lhe_set.lhe_parts = GetAllLHEs();

    SetSignalFlags();

    for (const auto &syst_dummy : *systHelper) {
        executeEventFromParameter();
    }

}

void HNWR_miniiso::executeEventFromParameter() {
    const TString this_syst = systHelper->getCurrentSysName();

    // RunXsecSyst produces one thing only: the per-PDF-member fit observable, which is
    // filled in the Central pass. Every object-variation pass (JES/JER/... , evtLoopAgain:
    // true) would re-run the whole event to fill histograms this job does not write, so
    // leave immediately. Cuts the job to a single pass over the events.
    if (RunXsecSyst && this_syst != "Central") return;
    
    Event ev = GetEvent();
    Particle METv = ev.GetMETVector(Event::MET_Type::PUPPI,Event::MET_Syst::CENTRAL);
    
    
    std::unordered_map<std::string, std::variant<std::function<float(MyCorrection::variation, TString)>, std::function<float()>>> weight_function_map;
    weight_function_map.reserve(30);  // Pre-allocate to avoid rehashing
    static auto dummy_sf = [](MyCorrection::variation var, TString source) -> float { return 1.0f; };

    weight_function_map["PU_Weight"]     = dummy_sf;
    // Electron Targets
    weight_function_map["E_Id_Weight"]   = dummy_sf;
    weight_function_map["E_Reco_Weight"] = dummy_sf;
    weight_function_map["E_Trig_Weight"] = dummy_sf;

    // Muon Targets
    weight_function_map["M_Id_Weight"]   = dummy_sf;
    weight_function_map["M_Reco_Weight"] = dummy_sf;
    weight_function_map["M_Trig_Weight"] = dummy_sf;

    // Jet Targets (object variations, not weight variations)
    weight_function_map["JER_Variation"] = dummy_sf;
    weight_function_map["JES_Variation"] = dummy_sf;
    // Object-level kinematic variations: the systematic is applied by re-running the event
    // with a varied collection, so the weight target is a dummy (see docs/MCLRSM_miniiso.yaml).
    weight_function_map["MuonScale_Variation"]     = dummy_sf;
    weight_function_map["MuonRes_Variation"]       = dummy_sf;
    weight_function_map["ElectronScale_Variation"] = dummy_sf;
    weight_function_map["ElectronRes_Variation"]   = dummy_sf;
    weight_function_map["FatJetJES_Variation"]     = dummy_sf;
    weight_function_map["FatJetJER_Variation"]     = dummy_sf;
    weight_function_map["M_Iso_Weight"]  = dummy_sf;
    // Mini-isolation cut efficiency SF. These analyzers drop the LSF3 > 0.75 cut in
    // favour of a miniIso cut on the subleading loose lepton, so this target replaces
    // Reproduce20_002_copy's "LSF_Weight" entirely -- hence the dedicated systematic
    // list docs/MCLRSM_miniiso.yaml ("MiniIso" -> "MiniIso_Weight"); MCLRSM.yaml still
    // declares LSF and is used by Reproduce20_002_copy only. Assigned only where the
    // loose lepton inside the fatjet is an ELECTRON (boosted SR EE, flavour CR
    // mu-ejet): the egamma-tnp measurement has no muon counterpart, so the muon-in-
    // fatjet regions (SR MM, flavour CR e-mujet) keep this dummy 1.
    weight_function_map["MiniIso_Weight"] = dummy_sf;

    // XSec(theory) weight targets
    weight_function_map["ScaleWeight_muF"] = dummy_sf;
    weight_function_map["ScaleWeight_muR"] = dummy_sf;
    weight_function_map["PDF_Weight"]      = dummy_sf;
    weight_function_map["AlphaS_Weight"]   = dummy_sf;

    // DY gen Z-pT correction targets: assigned below for DY MC, left at 1 for
    // everything else.
    weight_function_map["ZPt_Weight"]       = dummy_sf;
    weight_function_map["ZPt_QCDScale"]     = dummy_sf;
    weight_function_map["ZPt_QCDPDFError"]  = dummy_sf;
    weight_function_map["ZPt_QCDPDFAlphaS"] = dummy_sf;
    weight_function_map["ZPt_EW1"]          = dummy_sf;
    weight_function_map["ZPt_EW2"]          = dummy_sf;
    weight_function_map["ZPt_EW3"]          = dummy_sf;




    weight_function_map["PU_Weight"] = [&](MyCorrection::variation var, TString source) {
        return myCorr->GetPUWeight(ev.nTrueInt(), var);
    };

    // --- C(gen Z pT), DY MC only.
    //
    // Four targets, but only ZPt_Weight returns C at nominal; the other three
    // return 1 there and the ratio C_var/C_nom for their variation. That is
    // deliberate: calculateWeight() multiplies the nominal of *every* target
    // into the central weight, so having all four return C would apply it four
    // times over. This way Central carries exactly one factor of C, and each
    // variation swaps in its own curve.
    //
    // The four are also disjoint in what they vary: ZPt_Weight is the MC
    // statistical error of C, the other three are the theory variations of its
    // NLO numerator. Nothing here uses ZPTReweight_Up/Down -- that pair is the
    // quadrature sum of the same three theory groups, so it overlaps them
    // completely and is kept in the file for plotting only.
    if (dycorr.apply) {
        const float gen_zpt = GetGenZpT();
        if (gen_zpt < 0.) {
            cerr << "[HNWR_miniiso] FATAL: no isHardProcess lepton pair "
                 << "(run " << RunNumber << ", event " << EventNumber << ", sample "
                 << MCSample << ", era " << DataEra << "). C assumes 100% coverage."
                 << endl;
            exit(EXIT_FAILURE);
        }
        const float c_nom = GetZptWeight(gen_zpt, "ZPTReweight");
        // ZPt_Weight carries C at nominal and its MC STATISTICAL error as the
        // variation (nuisance ZPtRw_MC_stat). It deliberately does NOT use
        // ZPTReweight_Up/Down: those are QCDScale (+) QCDPDFError (+)
        // QCDPDFAlphaS added in quadrature, i.e. exactly the three theory
        // targets below, so using them here would count the theory band twice
        // and leave the statistical error of C out of the fit entirely.
        const float c_stat = GetZptStat(gen_zpt);
        weight_function_map["ZPt_Weight"] = [&, gen_zpt, c_nom, c_stat](MyCorrection::variation var, TString source) -> float {
            if (var == MyCorrection::variation::up)
                return c_nom + c_stat;
            if (var == MyCorrection::variation::down)
                // sigma_stat stays well below C over the delivered range (worst
                // bin ~17% of C), but never let the weight reach 0: that would
                // delete the event rather than down-weight it.
                return std::max(c_nom - c_stat, 1e-3f);
            return c_nom;
        };
        auto zpt_ratio_target = [&, gen_zpt, c_nom](const char *up_key, const char *dn_key) {
            return [this, gen_zpt, c_nom, up_key, dn_key](MyCorrection::variation var, TString source) -> float {
                if (c_nom <= 0.) return 1.f;
                if (var == MyCorrection::variation::up)
                    return GetZptWeight(gen_zpt, up_key) / c_nom;
                if (var == MyCorrection::variation::down)
                    return GetZptWeight(gen_zpt, dn_key) / c_nom;
                return 1.f;
            };
        };
        weight_function_map["ZPt_QCDScale"] =
            zpt_ratio_target("ZPTReweight_QCDScaleUp", "ZPTReweight_QCDScaleDown");
        weight_function_map["ZPt_QCDPDFError"] =
            zpt_ratio_target("ZPTReweight_QCDPDFErrorUp", "ZPTReweight_QCDPDFErrorDown");
        weight_function_map["ZPt_QCDPDFAlphaS"] =
            zpt_ratio_target("ZPTReweight_QCDPDFAlphaSUp", "ZPTReweight_QCDPDFAlphaSDown");

        // --- EW(gen Z pT), same one-factor-of-EW bookkeeping as C above:
        // ZPt_EW1 carries the EW nominal and its e1 variation; EW2/EW3 return
        // 1 at nominal and the ratio EW_var/EW_nom, so Central picks up the
        // EW correction exactly once and each of the three uncertainty
        // sources of arXiv:1705.04664 varies independently.
        const float ew_nom = GetZptEW(gen_zpt);
        weight_function_map["ZPt_EW1"] = [this, gen_zpt, ew_nom](MyCorrection::variation var, TString source) -> float {
            if (var == MyCorrection::variation::up)
                return GetZptEW(gen_zpt, 1, +1);
            if (var == MyCorrection::variation::down)
                return GetZptEW(gen_zpt, 1, -1);
            return ew_nom;
        };
        auto ew_ratio_target = [this, gen_zpt, ew_nom](int which) {
            return [this, gen_zpt, ew_nom, which](MyCorrection::variation var, TString source) -> float {
                if (ew_nom <= 0.) return 1.f;
                if (var == MyCorrection::variation::up)
                    return GetZptEW(gen_zpt, which, +1) / ew_nom;
                if (var == MyCorrection::variation::down)
                    return GetZptEW(gen_zpt, which, -1) / ew_nom;
                return 1.f;
            };
        };
        weight_function_map["ZPt_EW2"] = ew_ratio_target(2);
        weight_function_map["ZPt_EW3"] = ew_ratio_target(3);
    }

    // --- XSec(theory) weight systematics ---
    // Each function returns the absolute event weight for the given variation (nom == 1),
    // so calculateWeight() forms Up/Down by nominal_weight / nom * up(down).
    //
    // ACCEPTANCE ONLY: every LHE weight below is divided by the inclusive normalisation
    // K_var of its own index (theoryK_scale / theoryK_pdf; see LoadTheoryNormK). The raw
    // weights are inclusive-xsec change TIMES acceptance change, and these four nuisances
    // are signal-only in the datacards, so they must carry acceptance alone. Without the
    // division the nuisance is dominated by the normalisation -- WR4000 is ~35% inclusive
    // PDF against a few-% acceptance -- which inflates it by roughly an order of magnitude
    // and makes the limit correspondingly conservative. The inclusive piece is instead
    // drawn as the theory band of the exclusion plot (the Run2 "SignalScale lnN" slot).
    // K = 1 for any sample without a table entry, reproducing the previous behaviour.
    // muF: vary factorization scale, keep renormalization scale nominal.
    weight_function_map["ScaleWeight_muF"] = [&](MyCorrection::variation var, TString source) -> float {
        const float r = GetScaleVariation(var, MyCorrection::variation::nom);
        return r / GetTheoryNormK(theoryK_scale,
                                  GetScaleVariationIndex(var, MyCorrection::variation::nom));
    };
    // muR: vary renormalization scale, keep factorization scale nominal.
    // NOTE: for the WR signal this is identically 1 both before and after normalisation --
    // W_R production is EW at LO, so the matrix element has no alpha_S and no muR
    // dependence (the 9 weights are 3 identical blocks). The nuisance is only meaningful
    // for backgrounds, which the datacards do not attach it to; see the theory_norm_split
    // README section 6 for the card-side decision this still needs.
    weight_function_map["ScaleWeight_muR"] = [&](MyCorrection::variation var, TString source) -> float {
        const float r = GetScaleVariation(MyCorrection::variation::nom, var);
        return r / GetTheoryNormK(theoryK_scale,
                                  GetScaleVariationIndex(MyCorrection::variation::nom, var));
    };
    // PDF envelope: RETIRED as a weight target -- it stays at the dummy 1.
    //
    // A Hessian envelope is a quadrature sum over members OF THE OBSERVABLE,
    // dN(bin) = sqrt(sum_i (N_i(bin) - N_0(bin))^2), and N_i(bin) is itself a sum over
    // events. A single event weight cannot express that: (sum_evt x)^2 != sum_evt x^2.
    // The old implementation took the envelope event by event, which adds |shift| with no
    // cancellation even though every member pushes all events the same way. Measured on
    // WR4000N2100EE / 2022EE: 71.4% in SR_Resolved_EE and 73.6% in SR_Boosted_EE, against
    // 0.45% and 1.92% for the correct envelope -- a factor 157 and 38. Before any
    // selection, where closure demands 0, it gave 0.73 instead of 0.0034.
    //
    // The replacement is the RunXsecSyst userflag: it writes PDFmem<i>/<region>_mlljj, one
    // histogram per member, and the envelope is formed after hadd by
    // tables/06_systematics/theory_norm_split/make_pdf_envelope.py. Per-member histograms
    // have to survive hadd -- they add linearly, an envelope does not -- which is why the
    // members are stored rather than accumulated into a single number in the job.
    //
    // The "PDF" entry is removed from docs/MCLRSM*.yaml, so this target is now unreferenced;
    // it is kept defined so an older yaml still resolves instead of crashing.
    // alpha_S: dedicated PDF members (101 = down, 102 = up).
    weight_function_map["AlphaS_Weight"] = [&](MyCorrection::variation var, TString source) -> float {
        if (nLHEPdfWeight < 103) return 1.f;
        if (var == MyCorrection::variation::up)   return LHEPdfWeight[102] / GetTheoryNormK(theoryK_pdf, 102);
        if (var == MyCorrection::variation::down) return LHEPdfWeight[101] / GetTheoryNormK(theoryK_pdf, 101);
        return 1.f;
    };
    
    bool debug1(false);
    //Event Bool Categorization

    // Resolved
    ///DY CR
    bool is_Resolved_DY_EE(false);
    bool is_Resolved_DY_MM(false);

    // DY CR split in mll: CR1 = 60-100, CR2 = 100-150 (subsets of the 60-150 DY CR)
    bool is_Resolved_DY_EE_CR1(false);
    bool is_Resolved_DY_EE_CR2(false);
    bool is_Resolved_DY_MM_CR1(false);
    bool is_Resolved_DY_MM_CR2(false);

    bool is_Resolved_DY_EE_SS(false);
    bool is_Resolved_DY_EE_SS_2e_tight(false);
    bool is_Resolved_DY_EE_SS_1e_tight(false);
    bool is_Resolved_DY_EE_SS_0e_tight(false);

    bool is_Resolved_DY_EE_OS(false);
    bool is_Resolved_DY_EE_OS_2e_tight(false);
    bool is_Resolved_DY_EE_OS_1e_tight(false);
    bool is_Resolved_DY_EE_OS_0e_tight(false);
    
    bool is_Resolved_DY_MM_SS(false);
    bool is_Resolved_DY_MM_SS_tight(false);
    bool is_Resolved_DY_MM_SS_not_tight(false);

    bool is_Resolved_DY_MM_OS(false);
    bool is_Resolved_DY_MM_OS_tight(false);
    bool is_Resolved_DY_MM_OS_not_tight(false);
    
    ///FLAV CR
    bool is_Resolved_Flav_EM(false);
    
    bool is_Resolved_Flav_EM_SS(false);
    bool is_Resolved_Flav_EM_SS_tight(false);
    bool is_Resolved_Flav_EM_SS_not_tight(false);

    bool is_Resolved_Flav_EM_SS_M_tight_E_tight(false);
    bool is_Resolved_Flav_EM_SS_M_tight_E_not_tight(false);
    bool is_Resolved_Flav_EM_SS_M_not_tight_E_tight(false);
    bool is_Resolved_Flav_EM_SS_M_not_tight_E_not_tight(false);

    bool is_Resolved_Flav_EM_OS(false);
    bool is_Resolved_Flav_EM_OS_tight(false);
    bool is_Resolved_Flav_EM_OS_not_tight(false);

    bool is_Resolved_Flav_EM_OS_M_tight_E_tight(false);
    bool is_Resolved_Flav_EM_OS_M_tight_E_not_tight(false);
    bool is_Resolved_Flav_EM_OS_M_not_tight_E_tight(false);
    bool is_Resolved_Flav_EM_OS_M_not_tight_E_not_tight(false);

    /// SR
    bool is_Resolved_SR_EE(false);

    bool is_Resolved_SR_EE_SS(false);
    bool is_Resolved_SR_EE_SS_2e_tight(false);
    bool is_Resolved_SR_EE_SS_1e_tight(false);
    bool is_Resolved_SR_EE_SS_0e_tight(false);

    bool is_Resolved_SR_EE_OS(false);
    bool is_Resolved_SR_EE_OS_2e_tight(false);
    bool is_Resolved_SR_EE_OS_1e_tight(false);
    bool is_Resolved_SR_EE_OS_0e_tight(false);

    bool is_Resolved_SR_MM(false);
    bool is_Resolved_SR_MM_SS(false);
    bool is_Resolved_SR_MM_SS_tight(false);
    bool is_Resolved_SR_MM_SS_not_tight(false);

    bool is_Resolved_SR_MM_OS(false);
    bool is_Resolved_SR_MM_OS_tight(false);
    bool is_Resolved_SR_MM_OS_not_tight(false);

    // Boosted
    /// DY CR
    bool is_Boosted_DY_EE(false);
    bool is_Boosted_DY_MM(false);

    // DY CR split in mll: CR1 = 60-100, CR2 = 100-150 (subsets of the 60-150 DY CR)
    bool is_Boosted_DY_EE_CR1(false);
    bool is_Boosted_DY_EE_CR2(false);
    bool is_Boosted_DY_MM_CR1(false);
    bool is_Boosted_DY_MM_CR2(false);

    bool is_Boosted_DY_EE_SS(false);
    bool is_Boosted_DY_EE_SS_2e_tight(false);
    bool is_Boosted_DY_EE_SS_1e_tight(false);
    bool is_Boosted_DY_EE_SS_0e_tight(false);

    bool is_Boosted_DY_EE_OS(false);
    bool is_Boosted_DY_EE_OS_2e_tight(false);
    bool is_Boosted_DY_EE_OS_1e_tight(false);
    bool is_Boosted_DY_EE_OS_0e_tight(false);

    bool is_Boosted_DY_MM_SS(false);
    bool is_Boosted_DY_MM_SS_tight(false);
    bool is_Boosted_DY_MM_SS_not_tight(false);

    bool is_Boosted_DY_MM_OS(false);
    bool is_Boosted_DY_MM_OS_tight(false);
    bool is_Boosted_DY_MM_OS_not_tight(false);
    
    /// FLAV CR
    bool is_Boosted_Flav_EMJ(false);
    bool is_Boosted_Flav_MEJ(false);

    bool is_Boosted_Flav_EMJ_SS(false);
    
    bool is_Boosted_Flav_EMJ_SS_tight(false);
    bool is_Boosted_Flav_EMJ_SS_not_tight(false);
    bool is_Boosted_Flav_EMJ_SS_M_tight_E_tight(false);
    bool is_Boosted_Flav_EMJ_SS_M_tight_E_not_tight(false);
    bool is_Boosted_Flav_EMJ_SS_M_not_tight_E_tight(false);
    bool is_Boosted_Flav_EMJ_SS_M_not_tight_E_not_tight(false);

    bool is_Boosted_Flav_EMJ_OS(false);
    bool is_Boosted_Flav_EMJ_OS_tight(false);
    bool is_Boosted_Flav_EMJ_OS_not_tight(false);
    bool is_Boosted_Flav_EMJ_OS_M_tight_E_tight(false);
    bool is_Boosted_Flav_EMJ_OS_M_tight_E_not_tight(false);
    bool is_Boosted_Flav_EMJ_OS_M_not_tight_E_tight(false);
    bool is_Boosted_Flav_EMJ_OS_M_not_tight_E_not_tight(false);

    bool is_Boosted_Flav_MEJ_SS(false);

    bool is_Boosted_Flav_MEJ_SS_tight(false);
    bool is_Boosted_Flav_MEJ_SS_not_tight(false);
    bool is_Boosted_Flav_MEJ_SS_M_tight_E_tight(false);
    bool is_Boosted_Flav_MEJ_SS_M_tight_E_not_tight(false);
    bool is_Boosted_Flav_MEJ_SS_M_not_tight_E_tight(false);
    bool is_Boosted_Flav_MEJ_SS_M_not_tight_E_not_tight(false);

    bool is_Boosted_Flav_MEJ_OS(false);

    bool is_Boosted_Flav_MEJ_OS_tight(false);
    bool is_Boosted_Flav_MEJ_OS_not_tight(false);
    bool is_Boosted_Flav_MEJ_OS_M_tight_E_tight(false);
    bool is_Boosted_Flav_MEJ_OS_M_tight_E_not_tight(false);
    bool is_Boosted_Flav_MEJ_OS_M_not_tight_E_tight(false);
    bool is_Boosted_Flav_MEJ_OS_M_not_tight_E_not_tight(false);

    /// SR
    bool is_Boosted_SR_EE(false);

    bool is_Boosted_SR_EE_SS(false);
    bool is_Boosted_SR_EE_SS_2e_tight(false);
    bool is_Boosted_SR_EE_SS_1e_tight(false);
    bool is_Boosted_SR_EE_SS_0e_tight(false);
    
    bool is_Boosted_SR_EE_OS(false);   
    
    bool is_Boosted_SR_EE_OS_2e_tight(false);
    bool is_Boosted_SR_EE_OS_1e_tight(false);
    bool is_Boosted_SR_EE_OS_0e_tight(false);

    bool is_Boosted_SR_MM(false);

    bool is_Boosted_SR_MM_SS(false);
    bool is_Boosted_SR_MM_SS_tight(false);
    bool is_Boosted_SR_MM_SS_not_tight(false);

    bool is_Boosted_SR_MM_OS(false);
    bool is_Boosted_SR_MM_OS_tight(false);
    bool is_Boosted_SR_MM_OS_not_tight(false);
    
    //Event values


    //Resolved 
    float Resolve_DYCREEpt = 0.;
    float Resolve_DYCREEmass = 0.;
    float Resolve_DYCREEeta = 0.;
    float Resolve_DYCREEphi = 0.;
    float Resolve_DYCREEleadjetpt = 0.;
    float Resolve_DYCREEleadjeteta = 0.;
    float Resolve_DYCREEleadjetphi = 0.;
    float Resolve_DYCREEsubleadjetpt = 0.;
    float Resolve_DYCREEsubleadjeteta = 0.;
    float Resolve_DYCREEsubleadjetphi = 0.;
    float Resolve_DYCREEj1j2mass = 0.;
    float Resolve_DYCREEj1j2pt = 0.;
    float Resolve_DYCREEj1j2eta = 0.;
    float Resolve_DYCREEj1j2phi = 0.;
    float Resolve_DYCREEl1j1j2mass = 0.;
    float Resolve_DYCREEl1j1j2pt = 0.;
    float Resolve_DYCREEl1j1j2eta = 0.;
    float Resolve_DYCREEl1j1j2phi = 0.;
    float Resolve_DYCREEl2j1j2mass = 0.;
    float Resolve_DYCREEl2j1j2pt = 0.;
    float Resolve_DYCREEl2j1j2eta = 0.;
    float Resolve_DYCREEl2j1j2phi = 0.;
    float Resolve_DYCREEmlljj = 0.;
    float Resolve_DYCREEmlljjpt = 0.;
    float Resolve_DYCREEmlljjeta = 0.;
    float Resolve_DYCREEmlljjphi = 0.;
    float Resolve_DYCREEleadleppt = 0.;
    float Resolve_DYCREEleadlepeta = 0.;
    float Resolve_DYCREEleadlepphi = 0.;
    float Resolve_DYCREEsubleadleppt = 0.;
    float Resolve_DYCREEsubleadlepeta = 0.;
    float Resolve_DYCREEsubleadlepphi = 0.;
    float Resolve_DYCREEjetnum = 0.;
    float Resolve_DYCREEpunum = 0.;
    float Resolve_DYCREEpvgood = 0.;
    float Resolve_DYCREEpv = 0.;

    float Resolve_DYCRMMpt = 0.;
    float Resolve_DYCRMMmass = 0.;
    float Resolve_DYCRMMeta = 0.;
    float Resolve_DYCRMMphi = 0.;
    float Resolve_DYCRMMleadjetpt = 0.;
    float Resolve_DYCRMMleadjeteta = 0.;
    float Resolve_DYCRMMleadjetphi = 0.;
    float Resolve_DYCRMMsubleadjetpt = 0.;
    float Resolve_DYCRMMsubleadjeteta = 0.;
    float Resolve_DYCRMMsubleadjetphi = 0.;
    float Resolve_DYCRMMj1j2mass = 0.;
    float Resolve_DYCRMMj1j2pt = 0.;
    float Resolve_DYCRMMj1j2eta = 0.;
    float Resolve_DYCRMMj1j2phi = 0.;
    float Resolve_DYCRMMl1j1j2mass = 0.;
    float Resolve_DYCRMMl1j1j2pt = 0.;
    float Resolve_DYCRMMl1j1j2eta = 0.;
    float Resolve_DYCRMMl1j1j2phi = 0.;
    float Resolve_DYCRMMl2j1j2mass = 0.;
    float Resolve_DYCRMMl2j1j2pt = 0.;
    float Resolve_DYCRMMl2j1j2eta = 0.;
    float Resolve_DYCRMMl2j1j2phi = 0.;
    float Resolve_DYCRMMmlljj = 0.;
    float Resolve_DYCRMMmlljjpt = 0.;
    float Resolve_DYCRMMmlljjeta = 0.;
    float Resolve_DYCRMMmlljjphi = 0.;
    float Resolve_DYCRMMleadleppt = 0.;
    float Resolve_DYCRMMleadlepeta = 0.;
    float Resolve_DYCRMMleadlepphi = 0.;
    float Resolve_DYCRMMsubleadleppt = 0.;
    float Resolve_DYCRMMsubleadlepeta = 0.;
    float Resolve_DYCRMMsubleadlepphi = 0.;
    float Resolve_DYCRMMjetnum = 0.;
    float Resolve_DYCRMMpunum = 0.;
    float Resolve_DYCRMMpvgood = 0.;
    float Resolve_DYCRMMpv = 0.;

    float Resolve_FlavCRpt = 0.;
    float Resolve_FlavCRmass = 0.;
    float Resolve_FlavCReta = 0.;
    float Resolve_FlavCRphi = 0.;
    float Resolve_FlavCRleadjetpt = 0.;
    float Resolve_FlavCRleadjeteta = 0.;
    float Resolve_FlavCRleadjetphi = 0.;
    float Resolve_FlavCRsubleadjetpt = 0.;
    float Resolve_FlavCRsubleadjeteta = 0.;
    float Resolve_FlavCRsubleadjetphi = 0.;
    float Resolve_FlavCRj1j2mass = 0.;
    float Resolve_FlavCRj1j2pt = 0.;
    float Resolve_FlavCRj1j2eta = 0.;
    float Resolve_FlavCRj1j2phi = 0.;
    float Resolve_FlavCRl1j1j2mass = 0.;
    float Resolve_FlavCRl1j1j2pt = 0.;
    float Resolve_FlavCRl1j1j2eta = 0.;
    float Resolve_FlavCRl1j1j2phi = 0.;
    float Resolve_FlavCRl2j1j2mass = 0.;
    float Resolve_FlavCRl2j1j2pt = 0.;
    float Resolve_FlavCRl2j1j2eta = 0.;
    float Resolve_FlavCRl2j1j2phi = 0.;
    float Resolve_FlavCRmlljj = 0.;
    float Resolve_FlavCRmlljjpt = 0.;
    float Resolve_FlavCRmlljjeta = 0.;
    float Resolve_FlavCRmlljjphi = 0.;
    float Resolve_FlavCRleadleppt = 0.;
    float Resolve_FlavCRleadlepeta = 0.;
    float Resolve_FlavCRleadlepphi = 0.;
    float Resolve_FlavCRsubleadleppt = 0.;
    float Resolve_FlavCRsubleadlepeta = 0.;
    float Resolve_FlavCRsubleadlepphi = 0.;
    float Resolve_FlavCRjetnum = 0.;
    float Resolve_FlavCRpunum = 0.;
    float Resolve_FlavCRpvgood = 0.;
    float Resolve_FlavCRpv = 0.;

    float Resolve_SREEpt = 0.;
    float Resolve_SREEmass = 0.;
    float Resolve_SREEeta = 0.;
    float Resolve_SREEphi = 0.;
    float Resolve_SREEleadjetpt = 0.;
    float Resolve_SREEleadjeteta = 0.;
    float Resolve_SREEleadjetphi = 0.;
    float Resolve_SREEsubleadjetpt = 0.;
    float Resolve_SREEsubleadjeteta = 0.;
    float Resolve_SREEsubleadjetphi = 0.;
    float Resolve_SREEj1j2mass = 0.;
    float Resolve_SREEj1j2pt = 0.;
    float Resolve_SREEj1j2eta = 0.;
    float Resolve_SREEj1j2phi = 0.;
    float Resolve_SREEl1j1j2mass = 0.;
    float Resolve_SREEl1j1j2pt = 0.;
    float Resolve_SREEl1j1j2eta = 0.;
    float Resolve_SREEl1j1j2phi = 0.;
    float Resolve_SREEl2j1j2mass = 0.;
    float Resolve_SREEl2j1j2pt = 0.;
    float Resolve_SREEl2j1j2eta = 0.;
    float Resolve_SREEl2j1j2phi = 0.;
    float Resolve_SREEmlljj = 0.;
    float Resolve_SREEmlljjpt = 0.;
    float Resolve_SREEmlljjeta = 0.;
    float Resolve_SREEmlljjphi = 0.;
    float Resolve_SREEleadleppt = 0.;
    float Resolve_SREEleadlepeta = 0.;
    float Resolve_SREEleadlepphi = 0.;
    float Resolve_SREEsubleadleppt = 0.;
    float Resolve_SREEsubleadlepeta = 0.;
    float Resolve_SREEsubleadlepphi = 0.;
    float Resolve_SREEjetnum = 0.;
    float Resolve_SREEpunum = 0.;
    float Resolve_SREEpvgood = 0.;
    float Resolve_DYSREEpv = 0.;

    float Resolve_SRMMpt = 0.;
    float Resolve_SRMMmass = 0.;
    float Resolve_SRMMeta = 0.;
    float Resolve_SRMMphi = 0.;
    float Resolve_SRMMleadjetpt = 0.;
    float Resolve_SRMMleadjeteta = 0.;
    float Resolve_SRMMleadjetphi = 0.;
    float Resolve_SRMMsubleadjetpt = 0.;
    float Resolve_SRMMsubleadjeteta = 0.;
    float Resolve_SRMMsubleadjetphi = 0.;
    float Resolve_SRMMj1j2mass = 0.;
    float Resolve_SRMMj1j2pt = 0.;
    float Resolve_SRMMj1j2eta = 0.;
    float Resolve_SRMMj1j2phi = 0.;
    float Resolve_SRMMl1j1j2mass = 0.;
    float Resolve_SRMMl1j1j2pt = 0.;
    float Resolve_SRMMl1j1j2eta = 0.;
    float Resolve_SRMMl1j1j2phi = 0.;
    float Resolve_SRMMl2j1j2mass = 0.;
    float Resolve_SRMMl2j1j2pt = 0.;
    float Resolve_SRMMl2j1j2eta = 0.;
    float Resolve_SRMMl2j1j2phi = 0.;
    float Resolve_SRMMmlljj = 0.;
    float Resolve_SRMMmlljjpt = 0.;
    float Resolve_SRMMmlljjeta = 0.;
    float Resolve_SRMMmlljjphi = 0.;
    float Resolve_SRMMleadleppt = 0.;
    float Resolve_SRMMleadlepeta = 0.;
    float Resolve_SRMMleadlepphi = 0.;
    float Resolve_SRMMsubleadleppt = 0.;
    float Resolve_SRMMsubleadlepeta = 0.;
    float Resolve_SRMMsubleadlepphi = 0.;
    float Resolve_SRMMjetnum = 0.;
    float Resolve_SRMMpunum = 0.;
    float Resolve_SRMMpvgood = 0.;
    float Resolve_DYSRMMpv = 0.;


    float Boost_DYCREEpt = 0.;
    float Boost_DYCREEmass = 0.;
    float Boost_DYCREEeta = 0.;
    float Boost_DYCREEphi = 0.;
    float Boost_DYCREEfatjetpt = 0.;
    float Boost_DYCREEfatjeteta = 0.;
    float Boost_DYCREEfatjetphi = 0.;
    float Boost_DYCREEfatjetSDM = 0.;
    float Boost_DYCREEmlljj = 0.;
    float Boost_DYCREEmll = 0.;
    float Boost_DYCREEWRpt = 0.;
    float Boost_DYCREEWReta = 0.;
    float Boost_DYCREEWRphi = 0.;
    float Boost_DYCREEleadleppt = 0.;
    float Boost_DYCREEleadlepeta = 0.;
    float Boost_DYCREEleadlepphi = 0.;
    float Boost_DYCREEsubleadleppt = 0.;
    float Boost_DYCREEsubleadlepeta = 0.;
    float Boost_DYCREEsubleadlepphi = 0.;
    float Boost_DYCREEfatjet_lsf3 = 0.;
    float Boost_DYCREEsubleadlep_miniiso = -999.;
    float Boost_DYCREEdeltaR_leadlep_fatjet = 0.;
    float Boost_DYCREEdphi_leadlep_fatjet = 0.;
    float Boost_DYCREEpileup_num = 0.;
    float Boost_DYCREEjet_num = 0.;
    float Boost_DYCREEpvgood = 0.;
    float Boost_DYCREEpv = 0.;

    float Boost_DYCRMMpt = 0.;
    float Boost_DYCRMMmass = 0.;
    float Boost_DYCRMMeta = 0.;
    float Boost_DYCRMMphi = 0.;
    float Boost_DYCRMMfatjetpt = 0.;
    float Boost_DYCRMMfatjeteta = 0.;
    float Boost_DYCRMMfatjetphi = 0.;
    float Boost_DYCRMMfatjetSDM = 0.;
    float Boost_DYCRMMmlljj = 0.;
    float Boost_DYCRMMmll = 0.;
    float Boost_DYCRMMWRpt = 0.;
    float Boost_DYCRMMWReta = 0.;
    float Boost_DYCRMMWRphi = 0.;
    float Boost_DYCRMMleadleppt = 0.;
    float Boost_DYCRMMleadlepeta = 0.;
    float Boost_DYCRMMleadlepphi = 0.;
    float Boost_DYCRMMsubleadleppt = 0.;
    float Boost_DYCRMMsubleadlepeta = 0.;
    float Boost_DYCRMMsubleadlepphi = 0.;
    float Boost_DYCRMMfatjet_lsf3 = 0.;
    float Boost_DYCRMMsubleadlep_miniiso = -999.;
    float Boost_DYCRMMdeltaR_leadlep_fatjet = 0.;
    float Boost_DYCRMMdphi_leadlep_fatjet = 0.;
    float Boost_DYCRMMpileup_num = 0.;
    float Boost_DYCRMMjet_num = 0.;
    float Boost_DYCRMMpvgood = 0.;
    float Boost_DYCRMMpv = 0.;

    float Boost_SREEpt = 0.;
    float Boost_SREEmass = 0.;
    float Boost_SREEeta = 0.;
    float Boost_SREEphi = 0.;
    float Boost_SREEfatjetpt = 0.;
    float Boost_SREEfatjeteta = 0.;
    float Boost_SREEfatjetphi = 0.;
    float Boost_SREEfatjetSDM = 0.;
    float Boost_SREEmlljj = 0.;
    float Boost_SREEmll = 0.;
    float Boost_SREEWRpt = 0.;
    float Boost_SREEWReta = 0.;
    float Boost_SREEWRphi = 0.;
    float Boost_SREEleadleppt = 0.;
    float Boost_SREEleadlepeta = 0.;
    float Boost_SREEleadlepphi = 0.;
    float Boost_SREEsubleadleppt = 0.;
    float Boost_SREEsubleadlepeta = 0.;
    float Boost_SREEsubleadlepphi = 0.;
    float Boost_SREEfatjet_lsf3 = 0.;
    float Boost_SREEsubleadlep_miniiso = -999.;
    float Boost_SREEdeltaR_leadlep_fatjet = 0.;
    float Boost_SREEdphi_leadlep_fatjet = 0.;
    float Boost_SREEpileup_num = 0.;
    float Boost_SREEjet_num = 0.;
    float Boost_SREEpvgood = 0.;
    float Boost_SREEpv = 0.;

    float Boost_SRMMpt = 0.;
    float Boost_SRMMmass = 0.;
    float Boost_SRMMeta = 0.;
    float Boost_SRMMphi = 0.;
    float Boost_SRMMfatjetpt = 0.;
    float Boost_SRMMfatjeteta = 0.;
    float Boost_SRMMfatjetphi = 0.;
    float Boost_SRMMfatjetSDM = 0.;
    float Boost_SRMMmlljj = 0.;
    float Boost_SRMMmll = 0.;
    float Boost_SRMMWRpt = 0.;
    float Boost_SRMMWReta = 0.;
    float Boost_SRMMWRphi = 0.;
    float Boost_SRMMleadleppt = 0.;
    float Boost_SRMMleadlepeta = 0.;
    float Boost_SRMMleadlepphi = 0.;
    float Boost_SRMMsubleadleppt = 0.;
    float Boost_SRMMsubleadlepeta = 0.;
    float Boost_SRMMsubleadlepphi = 0.;
    float Boost_SRMMfatjet_lsf3 = 0.;
    float Boost_SRMMsubleadlep_miniiso = -999.;
    float Boost_SRMMdeltaR_leadlep_fatjet = 0.;
    float Boost_SRMMdphi_leadlep_fatjet = 0.;
    float Boost_SRMMpileup_num = 0.;
    float Boost_SRMMjet_num = 0.;
    float Boost_SRMMpvgood = 0.;
    float Boost_SRMMpv = 0.;

    float Boost_FlavEMJpt = 0.;
    float Boost_FlavEMJmass = 0.;
    float Boost_FlavEMJeta = 0.;
    float Boost_FlavEMJphi = 0.;
    float Boost_FlavEMJfatjetpt = 0.;
    float Boost_FlavEMJfatjeteta = 0.;
    float Boost_FlavEMJfatjetphi = 0.;
    float Boost_FlavEMJSDM = 0.;
    float Boost_FlavEMJmlljj = 0.;
    float Boost_FlavEMJmll = 0.;
    float Boost_FlavEMJWRpt = 0.;
    float Boost_FlavEMJWReta = 0.;
    float Boost_FlavEMJWRphi = 0.;
    float Boost_FlavEMJleadleppt = 0.;
    float Boost_FlavEMJleadlepeta = 0.;
    float Boost_FlavEMJleadlepphi = 0.;
    float Boost_FlavEMJsubleadleppt = 0.;
    float Boost_FlavEMJsubleadlepeta = 0.;
    float Boost_FlavEMJsubleadlepphi = 0.;
    float Boost_FlavEMJfatjet_lsf3 = 0.;
    float Boost_FlavEMJleadlep_lsf = 0.;
    float Boost_FlavEMJsubleadlep_miniiso = -999.;
    float Boost_FlavEMJleadfatjetpt = 0.;
    float Boost_FlavEMJdeltaR_leadlep_fatjet = 0.;
    float Boost_FlavEMJdphi_leadlep_fatjet = 0.;
    float Boost_FlavEMJpileup_num = 0.;
    float Boost_FlavEMJjet_num = 0.;
    float Boost_FlavEMJpvgood = 0.;
    float Boost_FlavEMJpv = 0.;

    float Boost_FlavMEJpt = 0.;
    float Boost_FlavMEJmass = 0.;
    float Boost_FlavMEJeta = 0.;
    float Boost_FlavMEJphi = 0.;
    float Boost_FlavMEJfatjetpt = 0.;
    float Boost_FlavMEJfatjeteta = 0.;
    float Boost_FlavMEJfatjetphi = 0.;
    float Boost_FlavMEJSDM = 0.;
    float Boost_FlavMEJmlljj = 0.;
    float Boost_FlavMEJmll = 0.;
    float Boost_FlavMEJWRpt = 0.;
    float Boost_FlavMEJWReta = 0.;
    float Boost_FlavMEJWRphi = 0.;
    float Boost_FlavMEJleadleppt = 0.;
    float Boost_FlavMEJleadlepeta = 0.;
    float Boost_FlavMEJleadlepphi = 0.;
    float Boost_FlavMEJsubleadleppt = 0.;
    float Boost_FlavMEJsubleadlepeta = 0.;
    float Boost_FlavMEJsubleadlepphi = 0.;
    float Boost_FlavMEJfatjet_lsf3 = 0.;
    float Boost_FlavMEJleadlep_lsf = 0.;
    float Boost_FlavMEJsubleadlep_miniiso = -999.;
    float Boost_FlavMEJleadfatjetpt = 0.;
    float Boost_FlavMEJdeltaR_leadlep_fatjet = 0.;
    float Boost_FlavMEJdphi_leadlep_fatjet = 0.;
    float Boost_FlavMEJpileup_num = 0.;
    float Boost_FlavMEJjet_num = 0;
    float Boost_FlavMEJpvgood = 0.;
    float Boost_FlavMEJpv = 0.;

    float muon1_tight_charge = 1 ; 
    float muon2_tight_charge  = 1 ; 

    float electron1_tight_charge = -1 ; 
    float electron2_tight_charge = -1;
    
    float weight = 1.0;
    float norm_weight = 1.0;
    RVec<Muon> temp_two_muon;
    //float heep_ID_weight = 1.0;

    if(!IsDATA){
        weight *= MCweight();
        weight *= ev.GetTriggerLumi("HLT_Mu50");
    }
    FillHist(this_syst + "/Cutflow_for_reseolved_SR", 1.0 , weight, 10, 0., 10.);
    FillSignalCutflow(this_syst, true, 1.0, weight);
    FillHist(this_syst + "/Cutflow_for_Boosted_SR", 1.0 , weight, 13, 0., 13.);
    FillSignalCutflow(this_syst, false, 1.0, weight);
    
    FillHist(this_syst + "/Cutflow_for_skim", 2.0 , 1.0, 10, 0., 10.);
    //Event selection
    RVec<Electron> electrons = el_set.AllElectrons;
    RVec<Muon> muons = mu_set.AllMuons;
    RVec<Jet> jets = jet_set.AllJets;
    RVec<FatJet> fatjets = fatjet_set.AllFatJets;
    RVec<LHE> lhe = lhe_set.lhe_parts;
    // Apply JES and JER systematic variations
    if (!IsDATA) {
        // Apply JER systematics (requires GenJets for smearing)
        for (const auto& jet : jets) {
            FillHist(this_syst + "/JESJER_before_jetpt",jet.Pt(), weight, 2000, 0., 2000.);
        }
        // JER smearing is NOT pre-applied in NanoAOD, so the nominal smearing must be
        // applied to every MC event; JER_Up/Down only swap the JER scale factor. Doing it
        // only for the variations would leave Central un-smeared, making both variations
        // shift in the same direction instead of bracketing Central.
        RVec<GenJet> genjets = GetAllGenJets();
        // AK4 JER. "FatJetJER_Up" also Contains("JER_Up"), so the FatJet guard is
        // required: without it the FatJetJER passes would vary the AK4 JER as well,
        // even though the two nuisances are deliberately decorrelated.
        MyCorrection::variation jer_var = MyCorrection::variation::nom;
        if (!this_syst.Contains("FatJetJER")) {
            if (this_syst.Contains("JER_Up"))        jer_var = MyCorrection::variation::up;
            else if (this_syst.Contains("JER_Down")) jer_var = MyCorrection::variation::down;
        }
        jets = SmearJets(jets, genjets, jer_var, "total");

        // AK8 JER: smeared exactly once, with the variation only in the FatJetJER
        // passes. (AK4 JER passes leave the AK8 smearing at nominal.)
        RVec<GenJet> genjetsak8 = GetAllGenJetAK8();
        MyCorrection::variation ak8_jer_var = MyCorrection::variation::nom;
        if (this_syst.Contains("FatJetJER_Up"))        ak8_jer_var = MyCorrection::variation::up;
        else if (this_syst.Contains("FatJetJER_Down")) ak8_jer_var = MyCorrection::variation::down;
        fatjets = SmearFatJets(fatjets, genjetsak8, ak8_jer_var, "total");

        // Apply JES systematics (AK4 only; AK8 has its own FatJetJES nuisance below)
        if (this_syst.Contains("JES_Up") && !this_syst.Contains("FatJetJES")) {
            jets = ScaleJets(jets, MyCorrection::variation::up, "total");
        } else if (this_syst.Contains("JES_Down") && !this_syst.Contains("FatJetJES")) {
            jets = ScaleJets(jets, MyCorrection::variation::down, "total");
        }

        // --- AK8 scale, decorrelated from AK4 ----------------------------------------
        if (this_syst.Contains("FatJetJES_Up")) {
            fatjets = ScaleFatJets(fatjets, MyCorrection::variation::up);
        } else if (this_syst.Contains("FatJetJES_Down")) {
            fatjets = ScaleFatJets(fatjets, MyCorrection::variation::down);
        }

        // --- soft-drop mass: AK4 JERC on the SoftDrop subjets ------------------------
        // JME recommendation (CMS Talk, "AK8 Puppi Jet JERC for Soft Drop Mass", Jan 2026):
        // m_SD is corrected by applying the AK4 JERC to the subjets and recomputing their
        // invariant mass. The AK8 JES/JER above correct FatJet_pt / FatJet_mass and leave
        // FatJet_msoftdrop untouched, so m_SD is driven by the AK4 nuisances, NOT by
        // FatJetJES/FatJetJER - deliberately hung off JES/JER to keep it correlated with the
        // AK4 jets, since it is the same uncertainty source.
        // This is not cosmetic: m_SD > FatJet_SDM (40 GeV) is a selection cut, and roughly
        // 11-16% of the selected yield sits within 10 GeV above it.
        // The trailing else is NOT optional: the call also applies the JER nominal smearing
        // (NanoAOD's msoftdrop has the subjet JEC but no smearing), so every pass has to
        // make it or it would differ from Central for a reason that is not its nuisance.
        if (this_syst.Contains("JES_Up") && !this_syst.Contains("FatJet")) {
            fatjets = VarySoftDropMass(fatjets, MyCorrection::variation::up, "total", false);
        } else if (this_syst.Contains("JES_Down") && !this_syst.Contains("FatJet")) {
            fatjets = VarySoftDropMass(fatjets, MyCorrection::variation::down, "total", false);
        } else if (this_syst.Contains("JER_Up") && !this_syst.Contains("FatJet")) {
            fatjets = VarySoftDropMass(fatjets, MyCorrection::variation::up, "total", true);
        } else if (this_syst.Contains("JER_Down") && !this_syst.Contains("FatJet")) {
            fatjets = VarySoftDropMass(fatjets, MyCorrection::variation::down, "total", true);
        } else {
            fatjets = VarySoftDropMass(fatjets, MyCorrection::variation::nom, "total", false);
        }

        // --- lepton energy/momentum variations --------------------------------------
        // Muon scale: MomentumScaleUp/Down were filled by GetAllMuons (Rochester error
        // below 200 GeV, GE kappa +- sigma above), so one nuisance covers both regimes.
        if (this_syst.Contains("MuonScale_Up")) {
            muons = ScaleMuons(muons, "up");
        } else if (this_syst.Contains("MuonScale_Down")) {
            muons = ScaleMuons(muons, "down");
        }
        // Muon resolution: flat 10% additional smearing (POG resolution deck 2024/06/03).
        // MC-only and high-pT-only by construction; one-sided in the barrel for 2022 and
        // 2023BPix, where the nominal smearing is 0 and a Gaussian cannot sharpen MC.
        if (this_syst.Contains("MuonRes_Up")) {
            muons = SmearMuons(muons, "up");
        } else if (this_syst.Contains("MuonRes_Down")) {
            muons = SmearMuons(muons, "down");
        }
        if (this_syst.Contains("ElectronScale_Up")) {
            electrons = ScaleElectrons(ev, electrons, "up");
        } else if (this_syst.Contains("ElectronScale_Down")) {
            electrons = ScaleElectrons(ev, electrons, "down");
        } else if (this_syst.Contains("ElectronRes_Up")) {
            electrons = SmearElectrons(electrons, "up");
        } else if (this_syst.Contains("ElectronRes_Down")) {
            electrons = SmearElectrons(electrons, "down");
        }
        // AK4-only monitoring; FatJetJES/JER passes leave the AK4 jets at nominal.
        if (!this_syst.Contains("FatJet") &&
            (this_syst.Contains("JER_Up") || this_syst.Contains("JER_Down") || this_syst.Contains("JES_Up") || this_syst.Contains("JES_Down"))) {
            for (const auto& jet : jets) {
                FillHist(this_syst + "/JESJER_after_jetpt",jet.Pt(), weight, 2000, 0., 2000.);
            }
        }
    }

    if (!PassNoiseFilter(jets,ev,Event::MET_Type::PUPPI)) return;
    FillHist(this_syst + "/Cutflow_for_reseolved_SR", 2.0 , weight, 10, 0., 10.);
    FillSignalCutflow(this_syst, true, 2.0, weight);
    FillHist(this_syst + "/Cutflow_for_Boosted_SR", 2.0 , weight, 13, 0., 13.);
    FillSignalCutflow(this_syst, false, 2.0, weight);
    FillHist(this_syst + "/Cutflow_for_skim", 3.0 , 1.0, 10, 0., 10.);
    bool pass_trig_muon = ev.PassTrigger(mu_set.Muon_Trigger);
    bool pass_trig_elec = ev.PassTrigger(el_set.Ele_Trigger);


    std::string nom = "nom";
    
    ///         Leptons       ///
    
    RVec<Electron> my_electrons = SelectElectrons(electrons, "NOCUT" , el_set.Electron_MinPt, 2.5); 
    RVec<Muon> my_muons = SelectMuons(muons, "NOCUT" , mu_set.Muon_MinPt, 2.4); 
    
    sort (my_electrons.begin(), my_electrons.end(), PtComparing);
    sort (my_muons.begin(), my_muons.end(), PtComparing);

    RVec<Electron *> Loose_electrons , Tight_electrons;
    RVec<Muon *> Loose_muons , Tight_muons;
    RVec<Lepton *> Tight_leps_el , Tight_leps_mu , Tight_leps;
    RVec<Lepton *> Loose_leps_el , Loose_leps_mu , Loose_leps;
    
    float el_tight_and_loose = 0.;
    float el_tight_but_fail_loose = 0.;
    float el_loose_and_fail_tight = 0.;
    float el_loose_but_tight = 0.;
    

    for (unsigned int i=0 ; i< my_electrons.size(); i ++) {
        Electron & el = my_electrons.at(i);
        //if (el_set.isPassCustomTightID(el, el_set)) {
        if (el.PassID(el_set.Electron_Tight_ID[0])) {
            Tight_electrons.push_back(&el);
            Tight_leps_el.push_back( &el);
            Tight_leps.push_back(&el);
            if (el_set.isPassCustomLooseID(el)) {
                el_tight_and_loose += 1.;
            } else {
                el_tight_but_fail_loose += 10.;
            }
        }

        // Loose ID: Match Python selectLooseElectrons logic
        // Pass if: (Loose WP without isolation) OR (HEEP)
        bool passLooseNoIso = el_set.isPassLooseNoIso(el);
        bool passHEEP = el.PassID(Electron::ElectronID::POG_HEEP);

        if (passLooseNoIso || passHEEP) {
        //if (el_set.isPassCustomLooseID(el)){  // OLD
        //if (el.PassID(el_set.Electron_Loose_ID[0])) {  // OLD
            Loose_electrons.push_back(&el);
            Loose_leps_el.push_back(&el);
            Loose_leps.push_back(&el);
            if (!el_set.isPassCustomTightID(el, el_set)) {
                el_loose_and_fail_tight += 100.;
            } else {
                el_loose_but_tight += 1000.;
            }
        }
    }
    
    FillHist(this_syst + "/Electron_Tight_and_Loose_ID_Check", el_tight_and_loose + el_tight_but_fail_loose + el_loose_and_fail_tight + el_loose_but_tight , weight, 1111, 0., 1111.);
    
    for (unsigned int i=0 ; i< my_muons.size(); i ++) {
        Muon & mu = my_muons.at(i);

        float tkRelIso = mu.TkRelIso();
        
        if ((mu.PassID(mu_set.Muon_Tight_ID[0]))&&( tkRelIso < 0.1) ){ //global high pt id 
            Tight_muons.push_back(&mu);
            Tight_leps_mu.push_back( &mu);
            Tight_leps.push_back(&mu);
        }
        if (mu.PassID(mu_set.Muon_Loose_ID[0])) {
            Loose_muons.push_back(&mu);
            Loose_leps_mu.push_back(&mu);
            Loose_leps.push_back(&mu);
        }
    }
    

    /// FatJets ///


    RVec<FatJet> fatjet_list ;
    RVec<FatJet> lsf ;

    FillHist(this_syst + "/Fatjet_num_total", fatjets.size() , weight, 10, 0., 10.);
    
    FillHist(this_syst + "/Jet_num_total_before_clean_looselep", jets.size() , weight, 10, 0., 10.);
    FillHist(this_syst + "/Fatjet_num_total_before_clean_tightlep", fatjets.size() , weight, 10, 0., 10.);
    // Leptons cleaned with fatjet(tight lepton) and jets(loose lepton)
    // HNWRAnalyzer does NOT clean fatjets with tight leptons → off by default
    // (CleanFatjetWithTightLeptons in the header; set to true to restore).
    // Rationale: the boosted selection already requires |dPhi(LeadLep, J)| > 2.0, so the
    // leading tight lepton can never be inside the away fatjet (dR < 0.4 is impossible);
    // the cleaning only ever removed fatjets overlapping a *subleading* tight lepton,
    // which is exactly the merged-lepton topology LSF3 is meant to select. Overlap is
    // instead handled at 4-vector level (dR(J,l) < 0.8 → Ncand = J only).
    if (CleanFatjetWithTightLeptons) fatjets = Clean_Fatjet_with_tight_leptons(fatjets, Tight_leps);
    jets = Clean_jet_with_loose_leptons(jets, Loose_leps);
    FillHist(this_syst + "/Jet_num_total_after_clean_looselep", jets.size() , weight, 10, 0., 10.);
    FillHist(this_syst + "/Fatjet_num_total_after_clean_tightlep", fatjets.size() , weight, 10, 0., 10.);
    
    FillHist(this_syst + "/Fatjet_pt_beforecut" , fatjets.size() > 0 ? fatjets[0].Pt() : 0. , weight, 100, 0., 500.);
    FillHist(this_syst + "/Fatjet_eta_beforecut" , fatjets.size() > 0 ? fatjets[0].Eta() : 0. , weight, 100, -5., 5.);
    FillHist(this_syst + "/Fatjet_SDM_beforecut" , fatjets.size() > 0 ? fatjets[0].SDMass() : 0. , weight, 100, 0., 500.);
    FillHist(this_syst + "/Fatjet_LSF3_beforecut" , fatjets.size() > 0 ? fatjets[0].LSF3() : 0. , weight, 100, 0., 1.);
    

    for (unsigned int i=0 ; i< fatjets.size(); i ++) {
        
        FatJet & fj = fatjets.at(i);
        if ((fj.Pt() > fatjet_set.FatJet_MinPt) && (abs(fj.Eta())<fatjet_set.FatJet_MaxEta) && (fj.SDMass() > fatjet_set.FatJet_SDM) ) {
            
            if (fj.PassID(fatjet_set.FatJet_ID)) {
                fatjet_list.push_back(fj);
                // LSF3 > 0.75 cut removed: miniIso < 0.1 on the subleading loose lepton
                // inside the fat jet is required at the region level instead
                lsf.push_back(fj);
            }
        }
    }
    FillHist(this_syst + "/Cutflow_for_skim", 4.0 , 1.0, 10, 0., 10.);
    fatjets = fatjet_list;
    RVec<FatJet> fatjets_LSF = lsf;

    sort (fatjets.begin(), fatjets.end(), PtComparing);
    sort (fatjets_LSF.begin(), fatjets_LSF.end(), PtComparing);
    
    FillHist(this_syst + "/Fatjet_num_aftercut", fatjets.size() , weight, 10, 0., 10.);
    FillHist(this_syst + "/Fatjet_LSF_num_aftercut", fatjets_LSF.size() , weight, 10, 0., 10.);
    
    

    sort (Tight_leps.begin(), Tight_leps.end(), PtComparingPtr);

    int n_Loose_leptons  = Loose_electrons.size() + Loose_muons.size();
    int n_Tight_leptons  = Tight_electrons.size() + Tight_muons.size();
    
    FillHist(this_syst + "/N_Loose_Lepton", n_Loose_leptons , weight, 10, 0., 10.);
    FillHist(this_syst + "/N_Tight_Lepton", n_Tight_leptons , weight, 10, 0., 10.);
    FillHist(this_syst + "/Tightleps_pt_lead", Tight_leps.size() >0 ? Tight_leps[0]->Pt() : 0. , weight, 100, 0., 500.);
    FillHist(this_syst + "/Tightleps_pt_sublead", Tight_leps.size() >1 ? Tight_leps[1]->Pt() : 0. , weight, 100, 0., 500.);


    ///         Jets       ///


    
    //jet veto 
    bool is_jet_veto = AnalyzerCore::PassVetoMap(jets, mu_set.AllMuons, "jetvetomap");
    if (!(is_jet_veto) ) return;
    
    
    FillHist(this_syst + "/Cutflow_for_skim", 6.0 , 1.0, 10, 0., 10.);
    FillHist(this_syst + "/Non_Selected_Jetnum", jets.size(), 1.0, 20, 0., 20.);
    RVec<Jet> selected_jets = SelectJets(jets, jet_set.Jet_ID[0] , jet_set.Jet_MinPt, jet_set.Jet_MaxEta);
    sort (selected_jets.begin(), selected_jets.end(), PtComparing);
    FillHist(this_syst + "/Selected_Jetnum", selected_jets.size(), 1.0, 20,-10,10.);

    

    // clean lsf fatjet with jet 
    //selected_jets = Clean_LSF_FatJet_with_jets(fatjets_LSF, selected_jets); /// 일단 젯 중에서 lsf fatjet이랑 겹치는거 제거  
    // clean fatjet with jet 
    //fatjets = Clean_Jets_with_fatjets(selected_jets,fatjets); // lsf fatjet 이랑 겹치는 젯 을 제외한 젯 중에서 팻젯이랑 겹치는경우 팻젯 제거 
    //FillHist(this_syst + "/num_of_jet_after_cleaning_lsffatjet", selected_jets.size(), 1.0, 10, 0., 10.);
    //FillHist(this_syst + "/num_of_fatjet_after_cleaningjet", fatjets.size(), 1.0, 10, 0., 10.);






    ////cout << ev.nTrueInt()<< "pilepu num"<<endl;
    // Requires 2 tight leptons , l1 > 130 (electron-led) / 60 (muon-led)
    // Def of resolved event
    bool IsResolvedEvent = false;
    bool this_trigger_pass(false);
    bool tmp_isEE(false), tmp_isMM(false), tmp_isEM(false);
    FillHist(this_syst + "/Cutflow_for_skim", 7.0 , 1.0, 10, 0., 10.);
    if ( (n_Tight_leptons == 2 ) && (Tight_leps[0]->Pt() > (Tight_leps[0]->IsElectron() ? 130.0 : 60.0))  && (Tight_leps[1]->Pt() > 53.0)) {
        
        
        if ( (Tight_electrons.size() == 2) && ( Tight_muons.size() == 0 )) {
            if (Tight_electrons[0]->Pt() < el_set.Ele_Trigger_Safe_Pt_Cut) return;
            FillHist(this_syst + "/Cutflow_for_reseolved_SR", 3.0 , weight , 10, 0., 10.);
            FillSignalCutflow(this_syst, true, 3.0, weight);
            FillHist(this_syst + "/CutFlow", 3.0, weight, 20,-10,10.); // 2 tight leptons with pT cut
            this_trigger_pass = pass_trig_elec;
            tmp_isEE = true;
            electron1_tight_charge = Tight_electrons[0]->TightCharge();
            electron2_tight_charge = Tight_electrons[1]->TightCharge();
            weight_function_map["E_Id_Weight"] = [&](MyCorrection::variation var, TString source) -> float {
                // HEEP ID SF from egamma-tnp T&P, (pt, eta) binned; up/down varied
                // coherently for both electrons (single systematic source)
                return GetElectronHEEPIDSF_TnP(Tight_electrons[0]->scEta(), Tight_electrons[0]->Pt(), var)
                     * GetElectronHEEPIDSF_TnP(Tight_electrons[1]->scEta(), Tight_electrons[1]->Pt(), var);
            };
    }
    
        else if ( (Tight_muons.size() == 2) && ( Tight_electrons.size() == 0 )) {
            if (Tight_muons[0]->Pt() < mu_set.Muon_Trigger_Safe_Pt_Cut) return;
            FillHist(this_syst + "/Cutflow_for_reseolved_SR", 3.0 , weight , 10, 0., 10.);
            FillSignalCutflow(this_syst, true, 3.0, weight);
            this_trigger_pass = pass_trig_muon;
            tmp_isMM = true;
            FillHist(this_syst + "/tightmuons", 2 , weight, 5, 0., 5.);
        }
        else if ( (Tight_muons.size() == 1) && ( Tight_electrons.size() == 1 )) {
            if (Tight_muons[0]->Pt() < mu_set.Muon_Trigger_Safe_Pt_Cut) return;
            FillHist(this_syst + "/Cutflow_for_reseolved_SR", 3.0 , weight , 10, 0., 10.);
            FillSignalCutflow(this_syst, true, 3.0, weight);
            this_trigger_pass = pass_trig_muon;
            tmp_isEM = true;
            electron1_tight_charge = Tight_electrons[0]->TightCharge();
            weight_function_map["E_Id_Weight"] = [&](MyCorrection::variation var, TString source) -> float {
                // HEEP ID SF from egamma-tnp T&P, (pt, eta) binned
                return GetElectronHEEPIDSF_TnP(Tight_electrons[0]->scEta(), Tight_electrons[0]->Pt(), var);
            };
            // Muon ID SF for the single tight muon of the resolved EM (flavour) CR.
            // Must stay enabled: rFlvCR shares the R_TT_Resolved rateParam with rEESR /
            // rDYEECR / rDYMuMuCR, so a missing SF here biases the fitted TT
            // normalisation that propagates into the SR.
            weight_function_map["M_Id_Weight"] = [&](MyCorrection::variation var, TString source) -> float   {
                if (DataEra=="2017") return 1.0;
                return (myCorr->GetMuonIDSF("NUM_HighPtID_DEN_GlobalMuonProbes", *Tight_muons[0], var)) ;
            };
            FillHist(this_syst + "/tightmuons", 3 , weight, 5, 0., 5.);
        }

        

        if (this_trigger_pass) {
            FillHist(this_syst + "/Cutflow_for_reseolved_SR", 4.0 , weight, 10, 0., 10.);
            FillSignalCutflow(this_syst, true, 4.0, weight);
            // needs 2 jets 
            Lepton *LeadLep = Tight_leps[0];
            Lepton *SubLeadLep = Tight_leps[1];
            
            float LeadLepCharge = LeadLep->Charge();
            float SubLeadLepCharge = SubLeadLep->Charge();
            bool dRLeadJetLepon(false), dRSubLeadJetLepon(false), dRTwoLetpton(false), dRTwoJets(false);
            if (selected_jets.size() >= 2) {
                FillHist(this_syst + "/Cutflow_for_reseolved_SR", 5.0 , weight, 10, 0., 10.);
                FillSignalCutflow(this_syst, true, 5.0, weight);
            dRLeadJetLepon = (selected_jets[0].DeltaR(*Tight_leps[0]) > 0.4) && (selected_jets[0].DeltaR(*Tight_leps[1]) > 0.4);
            dRSubLeadJetLepon = (selected_jets[1].DeltaR(*Tight_leps[0]) > 0.4) && (selected_jets[1].DeltaR(*Tight_leps[1]) > 0.4);
            dRTwoLetpton = (LeadLep->DeltaR(*SubLeadLep) > 0.4);
            dRTwoJets = (selected_jets[0].DeltaR(selected_jets[1]) > 0.4);
            }
            FillHist(this_syst + "/Jetnumber_before_resolved_selection1", selected_jets.size(), weight, 20,-10,10.);
                FillHist(this_syst + "/Jetnumber_before_resolved_selection2", selected_jets.size(), weight, 20,-10,10.);
                if ((selected_jets.size() >= 2 )&&(dRLeadJetLepon)&&(dRSubLeadJetLepon)&&(dRTwoLetpton)&&(dRTwoJets)) { 
                    FillHist(this_syst + "/Jetnumber_before_resolved_selection3", selected_jets.size(), weight, 20,-10,10.);
                    FillHist(this_syst + "/Cutflow_for_reseolved_SR", 6.0 , weight, 10, 0., 10.);
                    FillSignalCutflow(this_syst, true, 6.0, weight);
                    IsResolvedEvent = true;
                    // Mass calculation 
                    Particle WRCand = *LeadLep + *SubLeadLep + selected_jets[0] + selected_jets[1];
                    
                    double dilepton_mass = ( *LeadLep + *SubLeadLep ).M();
                    double dilepton_pt = ( *LeadLep + *SubLeadLep ).Pt();
                    bool DiLepMassGT200 = ( dilepton_mass > 200.0 );
                    bool DiLepMassGT400 = ( dilepton_mass > 400.0 );
                    bool DiLepMassLT150 = ( dilepton_mass >= 60. ) && ( dilepton_mass < 150. );
                    
                    bool DiLepMass60to100  = (dilepton_mass >= 60.) && (dilepton_mass < 100.);
                    bool DiLepMass100to150 = (dilepton_mass >= 100.) && (dilepton_mass < 150.);
                    bool DiLepMass200to400  = (dilepton_mass >= 200.) && (dilepton_mass < 400.);
                    double trigger_sf_SingleElectron = 1.0;
                    double trigger_sf_SingleMuon = 1.0;


                    
                
                // Trigger SF
                if(!IsDATA){
                    if(tmp_isEE){
                        
                        weight_function_map["E_Reco_Weight"] = [&](MyCorrection::variation var, TString source)  {
                            return (myCorr->GetElectronRECOSF(Tight_electrons[0]->scEta(), Tight_electrons[0]->Pt(), Tight_electrons[0]->Phi(),var)) * myCorr->GetElectronRECOSF(Tight_electrons[1]->scEta(), Tight_electrons[1]->Pt(), Tight_electrons[1]->Phi(),var);
                        };
                        if (DataEra != "2017") {
                        weight_function_map["E_Trig_Weight"] = [&](MyCorrection::variation var, TString source)  {
                            return GetElectronTriggerSF_TnP(Tight_electrons[0]->scEta(), Tight_electrons[0]->Pt(), var);
                        };
                        }

                    }
                    if(tmp_isMM){
                        
                        weight_function_map["M_Id_Weight"] = [&](MyCorrection::variation var, TString source) -> float   {
                            if (DataEra=="2017") return 1.0;
                            return  (myCorr->GetMuonIDSF("NUM_HighPtID_DEN_GlobalMuonProbes", *Tight_muons[0], var))*(myCorr->GetMuonIDSF("NUM_HighPtID_DEN_GlobalMuonProbes", *Tight_muons[1], var));
                        };
                        
                        //float MuonIDSF = (myCorr->GetMuonIDSF("NUM_HighPtID_DEN_GlobalMuonProbes", *Tight_muons[0]))*(myCorr->GetMuonIDSF("NUM_HighPtID_DEN_GlobalMuonProbes", *Tight_muons[1]));
                        weight_function_map["M_Reco_Weight"] = [&](MyCorrection::variation var, TString source)  {
                            return (myCorr->GetMuonHighPtRECOSF(*Tight_muons[0], var) * myCorr->GetMuonHighPtRECOSF(*Tight_muons[1], var));
                        };
                        //float MuonRECOSF = (myCorr->GetMuonRECOSF(*Tight_muons[0]) * myCorr->GetMuonRECOSF(*Tight_muons[1]));
                        // Fix: build trig_muons inside lambda to avoid dangling reference
                        // (local RVec goes out of scope before lambda is evaluated at systHelper->calculateWeight())
                        weight_function_map["M_Trig_Weight"] = [&](MyCorrection::variation var, TString source) -> float {
                            RVec<Muon*> trig_muons;
                            trig_muons.push_back(Tight_muons[0]);
                            trig_muons.push_back(Tight_muons[1]);
                            if (DataEra=="2017") return 1.0;
                            return (myCorr->GetMuonTriggerSF("NUM_HLT_DEN_HighPtLooseRelIsoProbes",trig_muons, var));
                        };
                        
                        weight_function_map["M_Iso_Weight"] = [&](MyCorrection::variation var, TString source) -> float  {
                            if (DataEra=="2017") return 1.0;
                            return  (myCorr->GetMuonIDSF("NUM_probe_LooseRelTkIso_DEN_HighPtProbes",*Tight_muons[0], var))*(myCorr->GetMuonIDSF("NUM_probe_LooseRelTkIso_DEN_HighPtProbes",*Tight_muons[1], var));
                        };
                        
                        //float MuonISOSF = (myCorr->GetMuonIDSF("NUM_probe_LooseRelTkIso_DEN_HighPtProbes",*Tight_muons[0]))*(myCorr->GetMuonIDSF("NUM_probe_LooseRelTkIso_DEN_HighPtProbes",*Tight_muons[1]));
                        if ( DiLepMassLT150 && WRCand.M() > 800.0 ) {
                        
                        }
                    }
                    if(tmp_isEM){
                        
                        weight_function_map["E_Reco_Weight"] = [&](MyCorrection::variation var, TString source)  {
                            return    (myCorr->GetElectronRECOSF(Tight_electrons[0]->scEta(), Tight_electrons[0]->Pt(), Tight_electrons[0]->Phi(),var)) ;
                        };
                        weight_function_map["M_Reco_Weight"] = [&](MyCorrection::variation var, TString source)  {
                            return  myCorr->GetMuonHighPtRECOSF(*Tight_muons[0], var);
                        };
                        
                        weight_function_map["M_Trig_Weight"] = [&](MyCorrection::variation var, TString source) -> float {
                            if (DataEra=="2017") return 1.0;
                            return   (myCorr->GetMuonTriggerSF("NUM_HLT_DEN_HighPtLooseRelIsoProbes",*Tight_muons[0], var));
                        };
                        
                        weight_function_map["M_Iso_Weight"] = [&](MyCorrection::variation var, TString source) -> float  {
                            if (DataEra=="2017") return 1.0;
                            return   (myCorr->GetMuonIDSF("NUM_probe_LooseRelTkIso_DEN_HighPtProbes",*Tight_muons[0], var));
                        };
                    }   
                }

                // DY CR 60 < ll < 150 , lljj > 800 
                if ( DiLepMassLT150 && WRCand.M() > 800.0 ) {
                    if (tmp_isEE) {
                        is_Resolved_DY_EE = true;
                        
                        if ( LeadLepCharge * SubLeadLepCharge > 0 ) {
                            is_Resolved_DY_EE_SS = true;
                        } else {
                            is_Resolved_DY_EE_OS = true;
                        }
                        //PU&object weight 적용
                        Resolve_DYCREEpt = dilepton_pt;
                        Resolve_DYCREEleadjetpt = selected_jets[0].Pt();
                        Resolve_DYCREEsubleadjetpt = selected_jets[1].Pt();
                        Resolve_DYCREEmlljj = WRCand.M();
                        Resolve_DYCREEleadleppt = LeadLep->Pt();
                        Resolve_DYCREEsubleadleppt = SubLeadLep->Pt();
                        // new variables
                        Resolve_DYCREEmass = dilepton_mass;
                        Resolve_DYCREEeta = (*LeadLep + *SubLeadLep).Eta();
                        Resolve_DYCREEphi = (*LeadLep + *SubLeadLep).Phi();
                        Resolve_DYCREEleadjeteta = selected_jets[0].Eta();
                        Resolve_DYCREEleadjetphi = selected_jets[0].Phi();
                        Resolve_DYCREEsubleadjeteta = selected_jets[1].Eta();
                        Resolve_DYCREEsubleadjetphi = selected_jets[1].Phi();
                        {
                            Particle j1j2 = selected_jets[0] + selected_jets[1];
                            Resolve_DYCREEj1j2mass = j1j2.M();
                            Resolve_DYCREEj1j2pt   = j1j2.Pt();
                            Resolve_DYCREEj1j2eta  = j1j2.Eta();
                            Resolve_DYCREEj1j2phi  = j1j2.Phi();
                            Particle l1j1j2 = *LeadLep + selected_jets[0] + selected_jets[1];
                            Resolve_DYCREEl1j1j2mass = l1j1j2.M();
                            Resolve_DYCREEl1j1j2pt   = l1j1j2.Pt();
                            Resolve_DYCREEl1j1j2eta  = l1j1j2.Eta();
                            Resolve_DYCREEl1j1j2phi  = l1j1j2.Phi();
                            Particle l2j1j2 = *SubLeadLep + selected_jets[0] + selected_jets[1];
                            Resolve_DYCREEl2j1j2mass = l2j1j2.M();
                            Resolve_DYCREEl2j1j2pt   = l2j1j2.Pt();
                            Resolve_DYCREEl2j1j2eta  = l2j1j2.Eta();
                            Resolve_DYCREEl2j1j2phi  = l2j1j2.Phi();
                        }
                        Resolve_DYCREEmlljjpt  = WRCand.Pt();
                        Resolve_DYCREEmlljjeta = WRCand.Eta();
                        Resolve_DYCREEmlljjphi = WRCand.Phi();
                        Resolve_DYCREEleadleppt      = LeadLep->Pt();
                        Resolve_DYCREEleadlepeta     = LeadLep->Eta();
                        Resolve_DYCREEleadlepphi     = LeadLep->Phi();
                        Resolve_DYCREEsubleadleppt   = SubLeadLep->Pt();
                        Resolve_DYCREEsubleadlepeta  = SubLeadLep->Eta();
                        Resolve_DYCREEsubleadlepphi  = SubLeadLep->Phi();
                        Resolve_DYCREEjetnum = (float)selected_jets.size();
                        Resolve_DYCREEpunum  = ev.nTrueInt();
                        Resolve_DYCREEpvgood = ev.nPVsGood();
                        Resolve_DYCREEpv = ev.nPV();
                        FillHist(this_syst + "/Resolve_DY_CR_EE_electron1_tight_charge", electron1_tight_charge, 1.0, 5, 0., 5.);
                        FillHist(this_syst + "/Resolve_DY_CR_EE_electron2_tight_charge", electron2_tight_charge, 1.0, 5, 0., 5.);
                        {
                            bool e1_tight = (electron1_tight_charge == 2);
                            bool e2_tight = (electron2_tight_charge == 2);
                            int n_e_tight = (e1_tight ? 1 : 0) + (e2_tight ? 1 : 0);
                            if (is_Resolved_DY_EE_SS) {
                                if (n_e_tight == 2) is_Resolved_DY_EE_SS_2e_tight = true;
                                else if (n_e_tight == 1) is_Resolved_DY_EE_SS_1e_tight = true;
                                else is_Resolved_DY_EE_SS_0e_tight = true;
                            } else if (is_Resolved_DY_EE_OS) {
                                if (n_e_tight == 2) is_Resolved_DY_EE_OS_2e_tight = true;
                                else if (n_e_tight == 1) is_Resolved_DY_EE_OS_1e_tight = true;
                                else is_Resolved_DY_EE_OS_0e_tight = true;
                            }
                        }
                        //cout << Resolve_DYCREEjetnum << "number of jet in DY CR EE" << endl;
                    }
                    else if (tmp_isMM) {
                        muon1_tight_charge = Tight_muons[0]->TightCharge() ;
                        muon2_tight_charge = Tight_muons[1]->TightCharge() ;
                        FillHist(this_syst + "/Resolve_DY_CR_MM_muon_tight_charge", muon1_tight_charge + muon2_tight_charge , 1.0, 5, 0., 5.);
                        bool tight_charge = ((muon1_tight_charge==2) && ( muon2_tight_charge == 2)) ;
                        
                    
                        is_Resolved_DY_MM = true;
                        if ( LeadLepCharge * SubLeadLepCharge > 0 ) {
                            is_Resolved_DY_MM_SS = true;
                            if (tight_charge) {
                                is_Resolved_DY_MM_SS_tight = true;
                            } else {
                                is_Resolved_DY_MM_SS_not_tight = true;
                            }
                        } else {
                            is_Resolved_DY_MM_OS = true;
                            if (tight_charge) {
                                is_Resolved_DY_MM_OS_tight = true;
                            } else {
                                is_Resolved_DY_MM_OS_not_tight = true;
                            }
                        }
                        Resolve_DYCRMMpt = dilepton_pt;
                        Resolve_DYCRMMleadjetpt = selected_jets[0].Pt();
                        Resolve_DYCRMMsubleadjetpt = selected_jets[1].Pt();
                        Resolve_DYCRMMmlljj = WRCand.M();
                        Resolve_DYCRMMleadleppt = LeadLep->Pt();
                        Resolve_DYCRMMsubleadleppt = SubLeadLep->Pt();
                        // new variables
                        Resolve_DYCRMMmass = dilepton_mass;
                        Resolve_DYCRMMeta = (*LeadLep + *SubLeadLep).Eta();
                        Resolve_DYCRMMphi = (*LeadLep + *SubLeadLep).Phi();
                        Resolve_DYCRMMleadjeteta = selected_jets[0].Eta();
                        Resolve_DYCRMMleadjetphi = selected_jets[0].Phi();
                        Resolve_DYCRMMsubleadjeteta = selected_jets[1].Eta();
                        Resolve_DYCRMMsubleadjetphi = selected_jets[1].Phi();
                        {
                            Particle j1j2 = selected_jets[0] + selected_jets[1];
                            Resolve_DYCRMMj1j2mass = j1j2.M();
                            Resolve_DYCRMMj1j2pt   = j1j2.Pt();
                            Resolve_DYCRMMj1j2eta  = j1j2.Eta();
                            Resolve_DYCRMMj1j2phi  = j1j2.Phi();
                            Particle l1j1j2 = *LeadLep + selected_jets[0] + selected_jets[1];
                            Resolve_DYCRMMl1j1j2mass = l1j1j2.M();
                            Resolve_DYCRMMl1j1j2pt   = l1j1j2.Pt();
                            Resolve_DYCRMMl1j1j2eta  = l1j1j2.Eta();
                            Resolve_DYCRMMl1j1j2phi  = l1j1j2.Phi();
                            Particle l2j1j2 = *SubLeadLep + selected_jets[0] + selected_jets[1];
                            Resolve_DYCRMMl2j1j2mass = l2j1j2.M();
                            Resolve_DYCRMMl2j1j2pt   = l2j1j2.Pt();
                            Resolve_DYCRMMl2j1j2eta  = l2j1j2.Eta();
                            Resolve_DYCRMMl2j1j2phi  = l2j1j2.Phi();
                        }
                        Resolve_DYCRMMmlljjpt  = WRCand.Pt();
                        Resolve_DYCRMMmlljjeta = WRCand.Eta();
                        Resolve_DYCRMMmlljjphi = WRCand.Phi();
                        Resolve_DYCRMMleadleppt      = LeadLep->Pt();
                        Resolve_DYCRMMleadlepeta     = LeadLep->Eta();
                        Resolve_DYCRMMleadlepphi     = LeadLep->Phi();
                        Resolve_DYCRMMsubleadleppt   = SubLeadLep->Pt();
                        Resolve_DYCRMMsubleadlepeta  = SubLeadLep->Eta();
                        Resolve_DYCRMMsubleadlepphi  = SubLeadLep->Phi();
                        Resolve_DYCRMMjetnum = (float)selected_jets.size();
                        Resolve_DYCRMMpunum  = ev.nTrueInt();
                        Resolve_DYCRMMpvgood = ev.nPVsGood();
                        Resolve_DYCRMMpv = ev.nPV();
                        
                        float tightmuon1_org_pt = Tight_muons[0]->OriginalPt();
                        float tightmuon1_pt = Tight_muons[0]->Pt();                                        
                                            if (tightmuon1_org_pt != tightmuon1_pt) {
                                                float rel = tightmuon1_pt / tightmuon1_org_pt;
                                                FillHist(this_syst + "/Muon_TuneP_RelPt", rel, weight, 100, -1., 1.);
                                            }
                    }
                    else if (tmp_isEM) {

                    }
                }
            // DY CR1 ( 60 < mll < 100)
            // Subset of the 60-150 DY CR above, so the Resolve_DYCR{EE,MM}* kinematics
            // are already cached there; only the region flag is needed here.
                if ( DiLepMass60to100 && WRCand.M() > 800.0 ){
                    if (tmp_isEE)
                    {
                        is_Resolved_DY_EE_CR1 = true;
                    }
                    if (tmp_isMM)
                    {
                        is_Resolved_DY_MM_CR1 = true;
                    }
                }
            // DY CR2 ( 100 < mll < 150)
                if ( DiLepMass100to150 && WRCand.M() > 800.0 ){
                    if (tmp_isEE)
                    {
                        is_Resolved_DY_EE_CR2 = true;
                    }
                    if (tmp_isMM)
                    {
                        is_Resolved_DY_MM_CR2 = true;
                    }
                }
                // Flavor CR
                if ( DiLepMassGT400 && WRCand.M() > 800.0 ){

                    if (tmp_isEM)
                    {
                    is_Resolved_Flav_EM = true;
                    muon1_tight_charge = Tight_muons[0]->TightCharge() ;
                    FillHist(this_syst + "/Flav_CR_EM_muon_tight_charge", muon1_tight_charge , 1.0, 5, 0., 5.);
                    bool tight_charge  = (muon1_tight_charge == 2);
                    bool e_tight_flav = (electron1_tight_charge == 2);

                    if ( LeadLepCharge * SubLeadLepCharge > 0 ) {
                        is_Resolved_Flav_EM_SS = true;
                        if (tight_charge ){
                            is_Resolved_Flav_EM_SS_tight = true;
                        } else {
                            is_Resolved_Flav_EM_SS_not_tight = true;
                        }
                        if (tight_charge && e_tight_flav) is_Resolved_Flav_EM_SS_M_tight_E_tight = true;
                        else if (tight_charge && !e_tight_flav) is_Resolved_Flav_EM_SS_M_tight_E_not_tight = true;
                        else if (!tight_charge && e_tight_flav) is_Resolved_Flav_EM_SS_M_not_tight_E_tight = true;
                        else is_Resolved_Flav_EM_SS_M_not_tight_E_not_tight = true;
                    } else {
                        is_Resolved_Flav_EM_OS = true;
                        if (tight_charge ){
                            is_Resolved_Flav_EM_OS_tight = true;
                        } else {
                            is_Resolved_Flav_EM_OS_not_tight = true;
                        }
                        if (tight_charge && e_tight_flav) is_Resolved_Flav_EM_OS_M_tight_E_tight = true;
                        else if (tight_charge && !e_tight_flav) is_Resolved_Flav_EM_OS_M_tight_E_not_tight = true;
                        else if (!tight_charge && e_tight_flav) is_Resolved_Flav_EM_OS_M_not_tight_E_tight = true;
                        else is_Resolved_Flav_EM_OS_M_not_tight_E_not_tight = true;
                    }
                    Resolve_FlavCRpt = dilepton_pt;
                    Resolve_FlavCRleadjetpt = selected_jets[0].Pt();
                    Resolve_FlavCRsubleadjetpt = selected_jets[1].Pt();
                    Resolve_FlavCRmlljj = WRCand.M();
                    Resolve_FlavCRleadleppt = LeadLep->Pt();
                    Resolve_FlavCRsubleadleppt = SubLeadLep->Pt();
                    // new variables
                    Resolve_FlavCRmass = dilepton_mass;
                    Resolve_FlavCReta  = (*LeadLep + *SubLeadLep).Eta();
                    Resolve_FlavCRphi  = (*LeadLep + *SubLeadLep).Phi();
                    Resolve_FlavCRleadjeteta = selected_jets[0].Eta();
                    Resolve_FlavCRleadjetphi = selected_jets[0].Phi();
                    Resolve_FlavCRsubleadjeteta = selected_jets[1].Eta();
                    Resolve_FlavCRsubleadjetphi = selected_jets[1].Phi();
                    {
                        Particle j1j2 = selected_jets[0] + selected_jets[1];
                        Resolve_FlavCRj1j2mass = j1j2.M();
                        Resolve_FlavCRj1j2pt   = j1j2.Pt();
                        Resolve_FlavCRj1j2eta  = j1j2.Eta();
                        Resolve_FlavCRj1j2phi  = j1j2.Phi();
                        Particle l1j1j2 = *LeadLep + selected_jets[0] + selected_jets[1];
                        Resolve_FlavCRl1j1j2mass = l1j1j2.M();
                        Resolve_FlavCRl1j1j2pt   = l1j1j2.Pt();
                        Resolve_FlavCRl1j1j2eta  = l1j1j2.Eta();
                        Resolve_FlavCRl1j1j2phi  = l1j1j2.Phi();
                        Particle l2j1j2 = *SubLeadLep + selected_jets[0] + selected_jets[1];
                        Resolve_FlavCRl2j1j2mass = l2j1j2.M();
                        Resolve_FlavCRl2j1j2pt   = l2j1j2.Pt();
                        Resolve_FlavCRl2j1j2eta  = l2j1j2.Eta();
                        Resolve_FlavCRl2j1j2phi  = l2j1j2.Phi();
                    }
                    Resolve_FlavCRmlljjpt  = WRCand.Pt();
                    Resolve_FlavCRmlljjeta = WRCand.Eta();
                    Resolve_FlavCRmlljjphi = WRCand.Phi();
                    Resolve_FlavCRleadlepeta    = LeadLep->Eta();
                    Resolve_FlavCRleadlepphi    = LeadLep->Phi();
                    Resolve_FlavCRsubleadlepeta = SubLeadLep->Eta();
                    Resolve_FlavCRsubleadlepphi = SubLeadLep->Phi();
                    Resolve_FlavCRjetnum = (float)selected_jets.size();
                    Resolve_FlavCRpunum  = ev.nTrueInt();
                    Resolve_FlavCRpvgood = ev.nPVsGood();
                    Resolve_FlavCRpv = ev.nPV();
                    FillHist(this_syst + "/Resolve_Flav_CR_electron1_tight_charge", electron1_tight_charge, 1.0, 5, 0., 5.);
                    }
                }
                // low mass CR 
                if (DiLepMassGT200 && WRCand.M() < 800.0 ){
                    if (tmp_isEE)
                    {
                    }
                    if (tmp_isMM)
                    {
                    }
                    }
                FillHist(this_syst + "/Cutflow_for_reseolved_SR", 7.0 , weight, 10, 0., 10.);
                FillSignalCutflow(this_syst, true, 7.0, weight);
                FillHist(this_syst + "/Cutflow_for_reseolved_SR", 8.0 , weight, 10, 0., 10.);
                FillSignalCutflow(this_syst, true, 8.0, weight);
                //Resovled SR
                if (DiLepMassGT400 && WRCand.M() > 800.0) {
                    if (!IsDATA){
                        if (tmp_isEE){
                            is_Resolved_SR_EE = true;
                            FillHist(this_syst + "/Cutflow_for_reseolved_SR", 9.0 , weight, 10, 0., 10.);
                            FillSignalCutflow(this_syst, true, 9.0, weight);
                            Resolve_SREEpt = dilepton_pt;
                            Resolve_SREEleadjetpt = selected_jets[0].Pt();
                            Resolve_SREEsubleadjetpt = selected_jets[1].Pt();
                            Resolve_SREEmlljj = WRCand.M();
                            Resolve_SREEleadleppt = LeadLep->Pt();
                            Resolve_SREEsubleadleppt = SubLeadLep->Pt();
                            // new variables
                            Resolve_SREEmass = dilepton_mass;
                            Resolve_SREEeta  = (*LeadLep + *SubLeadLep).Eta();
                            Resolve_SREEphi  = (*LeadLep + *SubLeadLep).Phi();
                            Resolve_SREEleadjeteta = selected_jets[0].Eta();
                            Resolve_SREEleadjetphi = selected_jets[0].Phi();
                            Resolve_SREEsubleadjeteta = selected_jets[1].Eta();
                            Resolve_SREEsubleadjetphi = selected_jets[1].Phi();
                            {
                                Particle j1j2 = selected_jets[0] + selected_jets[1];
                                Resolve_SREEj1j2mass = j1j2.M();
                                Resolve_SREEj1j2pt   = j1j2.Pt();
                                Resolve_SREEj1j2eta  = j1j2.Eta();
                                Resolve_SREEj1j2phi  = j1j2.Phi();
                                Particle l1j1j2 = *LeadLep + selected_jets[0] + selected_jets[1];
                                Resolve_SREEl1j1j2mass = l1j1j2.M();
                                Resolve_SREEl1j1j2pt   = l1j1j2.Pt();
                                Resolve_SREEl1j1j2eta  = l1j1j2.Eta();
                                Resolve_SREEl1j1j2phi  = l1j1j2.Phi();
                                Particle l2j1j2 = *SubLeadLep + selected_jets[0] + selected_jets[1];
                                Resolve_SREEl2j1j2mass = l2j1j2.M();
                                Resolve_SREEl2j1j2pt   = l2j1j2.Pt();
                                Resolve_SREEl2j1j2eta  = l2j1j2.Eta();
                                Resolve_SREEl2j1j2phi  = l2j1j2.Phi();
                            }
                            Resolve_SREEmlljjpt  = WRCand.Pt();
                            Resolve_SREEmlljjeta = WRCand.Eta();
                            Resolve_SREEmlljjphi = WRCand.Phi();
                            Resolve_SREEleadlepeta    = LeadLep->Eta();
                            Resolve_SREEleadlepphi    = LeadLep->Phi();
                            Resolve_SREEsubleadlepeta = SubLeadLep->Eta();
                            Resolve_SREEsubleadlepphi = SubLeadLep->Phi();
                            Resolve_SREEjetnum = (float)selected_jets.size();
                            Resolve_SREEpunum  = ev.nTrueInt();
                            Resolve_SREEpvgood = ev.nPVsGood();
                            Resolve_DYSREEpv = ev.nPV();
                            for (int i = 0; i < lhe.size(); i++) {
                                float q1 = lhe[i].PdgId();
                                float q2 = lhe[i].PdgId();
                                if (i > 2) {
                                    if (abs(q1) < 7 && abs(q2) < 7) {
                                        FillHist( this_syst + "/Resolved_SR_EE_q", q1, weight, 14, -7., 7.);
                                        FillHist( this_syst + "/Resolved_SR_EE_q", q2, weight, 14, -7., 7.);
                                    }
                                    }
                                }
                            // charge
                            if ( LeadLepCharge * SubLeadLepCharge > 0 ) {
                                is_Resolved_SR_EE_SS = true;
                            }
                            else {
                                is_Resolved_SR_EE_OS = true;
                            }
                            FillHist(this_syst + "/Resolve_SR_EE_electron1_tight_charge", electron1_tight_charge, 1.0, 5, 0., 5.);
                            FillHist(this_syst + "/Resolve_SR_EE_electron2_tight_charge", electron2_tight_charge, 1.0, 5, 0., 5.);
                            {
                                bool e1_tight = (electron1_tight_charge == 2);
                                bool e2_tight = (electron2_tight_charge == 2);
                                int n_e_tight = (e1_tight ? 1 : 0) + (e2_tight ? 1 : 0);
                                if (is_Resolved_SR_EE_SS) {
                                    if (n_e_tight == 2) is_Resolved_SR_EE_SS_2e_tight = true;
                                    else if (n_e_tight == 1) is_Resolved_SR_EE_SS_1e_tight = true;
                                    else is_Resolved_SR_EE_SS_0e_tight = true;
                                } else if (is_Resolved_SR_EE_OS) {
                                    if (n_e_tight == 2) is_Resolved_SR_EE_OS_2e_tight = true;
                                    else if (n_e_tight == 1) is_Resolved_SR_EE_OS_1e_tight = true;
                                    else is_Resolved_SR_EE_OS_0e_tight = true;
                                }
                            }
                        }
                        if (tmp_isMM){
                            FillHist(this_syst + "/Cutflow_for_reseolved_SR", 9.0 , weight, 10, 0., 10.);
                            FillSignalCutflow(this_syst, true, 9.0, weight);
                            is_Resolved_SR_MM = true;
                            muon1_tight_charge = Tight_muons[0]->TightCharge() ;
                            muon2_tight_charge = Tight_muons[1]->TightCharge() ;
                            FillHist(this_syst + "/Resolve_SR_MM_muon_tight_charge", muon1_tight_charge + muon2_tight_charge , 1.0, 5, 0., 5.);
                            bool tight_charge  = ((muon1_tight_charge==2) && ( muon2_tight_charge == 2)) ;
                            Resolve_SRMMpt = dilepton_pt;
                            Resolve_SRMMleadjetpt = selected_jets[0].Pt();
                            Resolve_SRMMsubleadjetpt = selected_jets[1].Pt();
                            Resolve_SRMMmlljj = WRCand.M();
                            Resolve_SRMMleadleppt = LeadLep->Pt();
                            Resolve_SRMMsubleadleppt = SubLeadLep->Pt();
                            // new variables
                            Resolve_SRMMmass = dilepton_mass;
                            Resolve_SRMMeta  = (*LeadLep + *SubLeadLep).Eta();
                            Resolve_SRMMphi  = (*LeadLep + *SubLeadLep).Phi();
                            Resolve_SRMMleadjeteta = selected_jets[0].Eta();
                            Resolve_SRMMleadjetphi = selected_jets[0].Phi();
                            Resolve_SRMMsubleadjeteta = selected_jets[1].Eta();
                            Resolve_SRMMsubleadjetphi = selected_jets[1].Phi();
                            {
                                Particle j1j2 = selected_jets[0] + selected_jets[1];
                                Resolve_SRMMj1j2mass = j1j2.M();
                                Resolve_SRMMj1j2pt   = j1j2.Pt();
                                Resolve_SRMMj1j2eta  = j1j2.Eta();
                                Resolve_SRMMj1j2phi  = j1j2.Phi();
                                Particle l1j1j2 = *LeadLep + selected_jets[0] + selected_jets[1];
                                Resolve_SRMMl1j1j2mass = l1j1j2.M();
                                Resolve_SRMMl1j1j2pt   = l1j1j2.Pt();
                                Resolve_SRMMl1j1j2eta  = l1j1j2.Eta();
                                Resolve_SRMMl1j1j2phi  = l1j1j2.Phi(); 
                                Particle l2j1j2 = *SubLeadLep + selected_jets[0] + selected_jets[1];
                                Resolve_SRMMl2j1j2mass = l2j1j2.M();
                                Resolve_SRMMl2j1j2pt   = l2j1j2.Pt();
                                Resolve_SRMMl2j1j2eta  = l2j1j2.Eta();
                                Resolve_SRMMl2j1j2phi  = l2j1j2.Phi();
                            }
                            Resolve_SRMMmlljjpt  = WRCand.Pt();
                            Resolve_SRMMmlljjeta = WRCand.Eta();
                            Resolve_SRMMmlljjphi = WRCand.Phi();
                            Resolve_SRMMleadlepeta    = LeadLep->Eta();
                            Resolve_SRMMleadlepphi    = LeadLep->Phi();
                            Resolve_SRMMsubleadlepeta = SubLeadLep->Eta();
                            Resolve_SRMMsubleadlepphi = SubLeadLep->Phi();
                            Resolve_SRMMjetnum = (float)selected_jets.size();
                            Resolve_SRMMpunum  = ev.nTrueInt();
                            Resolve_SRMMpvgood = ev.nPVsGood();
                            Resolve_DYSRMMpv = ev.nPV();
                            
                            float tightmuon1_org_pt = Tight_muons[0]->OriginalPt();
                            float tightmuon1_pt = Tight_muons[0]->Pt();                                        
                                            if (tightmuon1_org_pt != tightmuon1_pt) {
                                                float rel = tightmuon1_pt / tightmuon1_org_pt;
                                                FillHist(this_syst + "/Muon_TuneP_RelPt", rel, weight, 100, -1., 1.);
                                            }
                            for (int i = 0; i < lhe.size(); i++) {
                                float q1 = lhe[i].PdgId();
                                float q2 = lhe[i].PdgId();
                                if (i > 2) {
                                    if (abs(q1) < 7 && abs(q2) < 7) {
                                        FillHist( this_syst + "/Resolved_SR_MM_q", q1, weight, 14, -7., 7.);
                                        FillHist( this_syst + "/Resolved_SR_MM_q", q2, weight, 14, -7., 7.);
                                    }
                                    }
                                }
                        // charge
                            if ( LeadLepCharge * SubLeadLepCharge > 0 ) {
                                is_Resolved_SR_MM_SS = true;
                                if(tight_charge){   
                                    is_Resolved_SR_MM_SS_tight = true;
                                } else {
                                    is_Resolved_SR_MM_SS_not_tight = true;
                                }
                            }
                            else {
                                is_Resolved_SR_MM_OS = true;
                                if(tight_charge){
                                    is_Resolved_SR_MM_OS_tight = true;
                                } else {
                                    is_Resolved_SR_MM_OS_not_tight = true;
                                }
                            }
                    }
                }
            }// # 1192 -> Resolved ends 
            // dr < 0.4
        //} // 2jets 
    }
    }
}
    
    if (!IsResolvedEvent){
        FillHist(this_syst + "/Cutflow_for_Boosted_SR", 3.0 ,weight, 13, 0., 13.);
        FillSignalCutflow(this_syst, false, 3.0, weight);
        FillHist(this_syst + "/Boost_tightlepnum", n_Tight_leptons , weight, 10, 0., 10.);

        FillHist(this_syst + "/Cutflow_for_e_mujet", 1.0 , 1.0, 20, 0., 20.);
        FillHist(this_syst + "/Cutflow_for_mu_ejet", 1.0 , 1.0, 20, 0., 20.);

        FillHist(this_syst + "/Boost_cutflow_DY", 1 , weight, 20,-10,10.);
        FillHist(this_syst + "/Boost_cutflow_FLV", 1 , weight, 20,-10,10.);
        if ((n_Tight_leptons >0 ) && (Tight_leps[0]->Pt() > (Tight_leps[0]->IsElectron() ? 130.0 : 60.0))) {
            FillHist(this_syst + "/Boost_cutflow_DY", 2 , weight, 20,-10,10.);
            FillHist(this_syst + "/Boost_cutflow_FLV", 2 , weight, 20,-10,10.);
            bool this_trigger_pass_boost(false);
            bool is_tmp_lead_el(false), is_tmp_lead_mu(false);
            Lepton * LeadLep = Tight_leps[0];
            FillHist(this_syst + "/Cutflow_for_e_mujet", 2.0 , 1.0, 20, 0., 20.);
            FillHist(this_syst + "/Cutflow_for_mu_ejet", 2.0 , 1.0, 20, 0., 20.);
            if ( LeadLep->IsElectron() ) {
                FillHist(this_syst + "/Cutflow_for_e_mujet", 3.0 , 1.0, 20, 0., 20.);
                if (LeadLep->Pt() < el_set.Ele_Trigger_Safe_Pt_Cut) return;
                FillHist(this_syst + "/Cutflow_for_e_mujet", 4.0 , 1.0,20, 0., 20.);
                FillHist(this_syst + "/Cutflow_for_Boosted_SR", 4.0 , weight, 13, 0., 13.);
                FillSignalCutflow(this_syst, false, 4.0, weight);
                is_tmp_lead_el = true;
                electron1_tight_charge = Tight_electrons[0]->TightCharge();
                this_trigger_pass_boost = pass_trig_elec;
                weight_function_map["E_Id_Weight"] = [&](MyCorrection::variation var, TString source) -> float {
                    // HEEP ID SF from egamma-tnp T&P, (pt, eta) binned
                    return GetElectronHEEPIDSF_TnP(Tight_electrons[0]->scEta(), Tight_electrons[0]->Pt(), var);
                };
            }
            else if ( LeadLep->IsMuon()){
                FillHist(this_syst + "/Cutflow_for_mu_ejet", 3.0 , 1.0, 20, 0., 20.);
                if (LeadLep->Pt() < mu_set.Muon_Trigger_Safe_Pt_Cut) return;
                FillHist(this_syst + "/Cutflow_for_mu_ejet", 4.0 , 1.0, 20, 0., 20.);
                is_tmp_lead_mu = true;
                this_trigger_pass_boost = pass_trig_muon;
                FillHist(this_syst + "/Check_is_tmp_lead_muon_ok", 1 , weight, 5, 0., 5.);
                FillHist(this_syst + "/Cutflow_for_Boosted_SR", 4.0 , weight, 13, 0., 13.);
                FillSignalCutflow(this_syst, false, 4.0, weight);
            }
            //checked ok
            if (this_trigger_pass_boost){
                if(is_tmp_lead_el){
                FillHist(this_syst + "/Cutflow_for_e_mujet", 5.0 , 1.0, 20, 0., 20.);
                }
                if(is_tmp_lead_mu){
                FillHist(this_syst + "/Cutflow_for_mu_ejet", 5.0 , 1.0, 20, 0., 20.);
                }
                FillHist(this_syst + "/Cutflow_for_Boosted_SR", 5.0 , weight, 13, 0., 13.);
                FillSignalCutflow(this_syst, false, 5.0, weight);
                FillHist(this_syst + "/Boost_cutflow_DY", 3 , weight,  20,-10,10.);
                FillHist(this_syst + "/Boost_cutflow_FLV", 3 , weight, 20,-10,10.);
                RVec<Lepton *> Loose_SF_leps = is_tmp_lead_el ? Loose_leps_el : Loose_leps_mu;
                RVec<Lepton *> Loose_OF_leps = is_tmp_lead_el ? Loose_leps_mu : Loose_leps_el;
                Lepton * LowMllLooseLepton ; 
                bool has_lowmll(false);
                float lowmllmass ;
                //check num of SF loose lepton for low mll num
                
                Muon looseMuon ;
                for (unsigned int i=0 ; i< Loose_SF_leps.size(); i ++) {
                    if (Loose_SF_leps[i] == LeadLep) continue;
                    double dilep_mass = (*LeadLep + *Loose_SF_leps[i]).M();
                    
                    if ((dilep_mass > 60) && (dilep_mass < 150 )) {
                        FillHist(this_syst + "/Dilep_mass_cutflow3to4test", dilep_mass, weight, 200, 0., 2000.);
                        has_lowmll = true;
                        lowmllmass = dilep_mass;
                        LowMllLooseLepton = Loose_SF_leps[i];
                        if (is_tmp_lead_mu){
                            Muon * looseMuon = (Muon *)LowMllLooseLepton;
                        }
                        break;
                    }
                }
                
                // Boosted CR selection with low mll ( DY CR)
                if (has_lowmll){
                    
                    FillHist(this_syst + "/Boost_cutflow_DY", 4 , weight, 20,-10,10.);
                    float fatjet_DYBoost_num = 0;
                    for (unsigned int i=0 ; i< fatjets.size(); i++) {
                        FatJet this_fatjet = fatjets.at(i);
                        FillHist(this_syst + "/deltaPhi_LeadLep_Fatjet", abs( LeadLep->DeltaPhi(this_fatjet)) , weight, 100, 0., 3.5);
                        if (abs( LeadLep->DeltaPhi(this_fatjet))>2.0) {
                            FillHist(this_syst + "/Boost_cutflow_DY", 5 , weight, 20,-10,10.);
                            FatJet HNFatJet = this_fatjet;
                            Particle Ncand;
                            bool looselepton_infatjet = false;
                            fatjet_DYBoost_num++;
                        // if loose lepton is inside of fatjet
                            if (this_fatjet.DeltaR( *LowMllLooseLepton)<0.8) {
                                Ncand = this_fatjet;
                                FillHist(this_syst + "/has_looselepton_insidefatjet", 1 , weight, 5, 0., 5.);
                                looselepton_infatjet = true;
                            }
                            else { // if loose lepton is outside of fatjet
                                Ncand = HNFatJet + *LowMllLooseLepton;
                                FillHist(this_syst + "/has_looselepton_outsidefatjet", 1 , weight, 5, 0., 5.);
                                FillHist(this_syst + "/mass_looselepton_fatjet_outside", Ncand.M() , weight, 8000, 0., 8000.);
                                FillHist(this_syst +"/numofhnfatjet_DY", HNFatJet.SDMass() , weight, 10000, 0., 10000.);
                            }
                            Particle WRCand;
                            WRCand = *LeadLep + Ncand;
                            double Ncandmasss = (Ncand).M();
                            double leadlepmass = LeadLep->M();
                            double leadleppt = LeadLep->Pt();
                            FillHist(this_syst + "/LeadLep_pt_DY", leadleppt , weight, 1000, 0., 1000.);
                            FillHist(this_syst + "/LeadLep_mass_DY", leadlepmass , weight, 5000, 0., 5000.);
                            FillHist(this_syst + "/WRCandmass", Ncandmasss , weight, 8000, 0., 8000.);
                            FillHist(this_syst + "/WRCand_mass_boosted_DY", WRCand.M() , weight, 8000, 0., 8000.);
                            if ( WRCand.M() > 800.0 )  {
                                FillHist(this_syst + "/Boost_cutflow_DY", 6 , weight, 20,-10,10.);
                                // DY CR
                                FillHist(this_syst + "/DY_CR_Fatjet_SDMass", HNFatJet.SDMass() , weight, 10000, 0., 10000.);
                                if (!IsDATA){
                                    //SF
                                    if(is_tmp_lead_el){
                                        
                                            weight_function_map["E_Reco_Weight"] = [&](MyCorrection::variation var, TString source) {
                                            return (myCorr->GetElectronRECOSF(((Electron *)LeadLep)->scEta(), LeadLep->Pt(), LeadLep->Phi(),var)) ;};
                                            if (DataEra != "2017") {
                                            weight_function_map["E_Trig_Weight"] = [&](MyCorrection::variation var, TString source)  {
                                                return GetElectronTriggerSF_TnP(Tight_electrons[0]->scEta(), Tight_electrons[0]->Pt(), var);
                                            };
                                        }

                                    }
                                    if(is_tmp_lead_mu){
                                        
                                        weight_function_map["M_Id_Weight"] = [&](MyCorrection::variation var, TString source) -> float  {
                                            if (DataEra=="2017") return 1.0;
                                            return (myCorr->GetMuonIDSF("NUM_HighPtID_DEN_GlobalMuonProbes", *Tight_muons[0],var));};
                                        weight_function_map["M_Reco_Weight"] = [&](MyCorrection::variation var, TString source) {
                                            return (myCorr->GetMuonHighPtRECOSF(*Tight_muons[0],var)) ;};
                                        
                                        // Fix: capture LowMllLooseLepton by value to avoid dangling reference
                                        // (LowMllLooseLepton goes out of scope before lambda is evaluated at systHelper->calculateWeight())
                                        
                                        weight_function_map["M_Trig_Weight"] = [&, LowMllLooseLepton](MyCorrection::variation var, TString source) -> float {
                                            RVec<Muon*> trig_muons;
                                            trig_muons.push_back(Tight_muons[0]);
                                            if (LowMllLooseLepton) {
                                                trig_muons.push_back( (Muon*)LowMllLooseLepton );
                                            }
                                            if (DataEra=="2017") return 1.0;
                                        return myCorr->GetMuonTriggerSF("NUM_HLT_DEN_HighPtLooseRelIsoProbes", trig_muons, var);};
                                        
                                        weight_function_map["M_Iso_Weight"] = [&](MyCorrection::variation var, TString source) {
                                            return (myCorr->GetMuonIDSF("NUM_probe_LooseRelTkIso_DEN_HighPtProbes",*Tight_muons[0],var));};
                                        
                                    }
                                }
                                
                                if (is_tmp_lead_el){ // ee 
                                        
                                        is_Boosted_DY_EE = true;
                                        float leadlep_charge = LeadLep->Charge();
                                        float looselep_charge = LowMllLooseLepton->Charge();

                                        if ( leadlep_charge * looselep_charge > 0 ) {
                                            is_Boosted_DY_EE_SS = true;
                                        } else {
                                            is_Boosted_DY_EE_OS = true;
                                        }

                                        Boost_DYCREEpt = (*LeadLep + *LowMllLooseLepton).Pt();
                                        Boost_DYCREEfatjetpt = HNFatJet.Pt();
                                        Boost_DYCREEmlljj = WRCand.M();
                                        Boost_DYCREEleadleppt = LeadLep->Pt();
                                        Boost_DYCREEsubleadleppt = LowMllLooseLepton->Pt();
                                        // new variables
                                        Boost_DYCREEmass    = (*LeadLep + *LowMllLooseLepton).M();
                                        Boost_DYCREEeta     = (*LeadLep + *LowMllLooseLepton).Eta();
                                        Boost_DYCREEphi     = (*LeadLep + *LowMllLooseLepton).Phi();
                                        Boost_DYCREEfatjeteta  = HNFatJet.Eta();
                                        Boost_DYCREEfatjetphi  = HNFatJet.Phi();
                                        Boost_DYCREEfatjetSDM  = HNFatJet.SDMass();
                                        Boost_DYCREEmll        = lowmllmass;
                                        Boost_DYCREEWRpt       = WRCand.Pt();
                                        Boost_DYCREEWReta      = WRCand.Eta();
                                        Boost_DYCREEWRphi      = WRCand.Phi();
                                        Boost_DYCREEleadlepeta    = LeadLep->Eta();
                                        Boost_DYCREEleadlepphi    = LeadLep->Phi();
                                        Boost_DYCREEsubleadlepeta = LowMllLooseLepton->Eta();
                                        Boost_DYCREEsubleadlepphi = LowMllLooseLepton->Phi();
                                        Boost_DYCREEfatjet_lsf3               = HNFatJet.LSF3();
                                        Boost_DYCREEsubleadlep_miniiso        = LowMllLooseLepton->MiniPFRelIso();
                                        Boost_DYCREEdeltaR_leadlep_fatjet     = LeadLep->DeltaR(HNFatJet);
                                        Boost_DYCREEdphi_leadlep_fatjet       = (float)abs(LeadLep->DeltaPhi(HNFatJet));
                                        Boost_DYCREEpileup_num = ev.nTrueInt();
                                        Boost_DYCREEpvgood = ev.nPVsGood();
                                        Boost_DYCREEpv = ev.nPV();
                                        Boost_DYCREEjet_num    = (float)selected_jets.size();
                                        if (looselepton_infatjet){
                                            FillHist(this_syst + "/Boosted_DY_CR_EE_looselepton_infatjet_Fatjet_SDMass", HNFatJet.SDMass() , weight, 10000, 0., 10000.);
                                        }
                                        else{
                                            FillHist(this_syst + "/Boosted_DY_CR_EE_looselepton_outsidefatjet_Fatjet_SDMass", HNFatJet.SDMass() , weight, 10000, 0., 10000.);
                                        }
                                        electron2_tight_charge = ((Electron*)LowMllLooseLepton)->TightCharge();
                                        FillHist(this_syst + "/Boosted_DY_CR_EE_electron1_tight_charge", electron1_tight_charge , 1.0, 5, 0., 5.);
                                        FillHist(this_syst + "/Boosted_DY_CR_EE_electron2_tight_charge", electron2_tight_charge , 1.0, 5, 0., 5.);
                                        {
                                            bool e1_tight = (electron1_tight_charge == 2);
                                            bool e2_tight = (electron2_tight_charge == 2);
                                            int n_e_tight = (e1_tight ? 1 : 0) + (e2_tight ? 1 : 0);
                                            if (is_Boosted_DY_EE_SS) {
                                                if (n_e_tight == 2) is_Boosted_DY_EE_SS_2e_tight = true;
                                                else if (n_e_tight == 1) is_Boosted_DY_EE_SS_1e_tight = true;
                                                else is_Boosted_DY_EE_SS_0e_tight = true;
                                            } else if (is_Boosted_DY_EE_OS) {
                                                if (n_e_tight == 2) is_Boosted_DY_EE_OS_2e_tight = true;
                                                else if (n_e_tight == 1) is_Boosted_DY_EE_OS_1e_tight = true;
                                                else is_Boosted_DY_EE_OS_0e_tight = true;
                                            }
                                        }
                                    }
                                    if (is_tmp_lead_mu){// mumu
                                        muon1_tight_charge = Tight_muons[0]->TightCharge() ;
                                        muon2_tight_charge = ((Muon*)LowMllLooseLepton)->TightCharge() ;
                                        FillHist(this_syst + "/Boost_DY_CR_MM_muon_tight_charge", muon1_tight_charge + muon2_tight_charge , 1.0, 5, 0., 5.);
                                        bool tight_charge = ((muon1_tight_charge==2) && ( muon2_tight_charge == 2)) ;
                                        is_Boosted_DY_MM = true;
                                        float leadlep_charge = LeadLep->Charge();
                                        float looselep_charge = LowMllLooseLepton->Charge();
                                        if ( leadlep_charge * looselep_charge > 0 ) {
                                            is_Boosted_DY_MM_SS = true;
                                            if ( tight_charge ){
                                                is_Boosted_DY_MM_SS_tight = true;
                                            }
                                            else{
                                                is_Boosted_DY_MM_SS_not_tight = true;
                                            }
                                        } else {
                                            is_Boosted_DY_MM_OS = true;
                                            if ( tight_charge ){
                                                is_Boosted_DY_MM_OS_tight = true;
                                            }
                                            else{
                                                is_Boosted_DY_MM_OS_not_tight = true;
                                            }
                                        }
                                        Boost_DYCRMMpt = (*LeadLep + *LowMllLooseLepton).Pt();
                                        Boost_DYCRMMfatjetpt = HNFatJet.Pt();
                                        Boost_DYCRMMmlljj = WRCand.M();
                                        Boost_DYCRMMleadleppt = LeadLep->Pt();
                                        Boost_DYCRMMsubleadleppt = LowMllLooseLepton->Pt();
                                        // new variables
                                        Boost_DYCRMMmass    = (*LeadLep + *LowMllLooseLepton).M();
                                        Boost_DYCRMMeta     = (*LeadLep + *LowMllLooseLepton).Eta();
                                        Boost_DYCRMMphi     = (*LeadLep + *LowMllLooseLepton).Phi();
                                        Boost_DYCRMMfatjeteta  = HNFatJet.Eta();
                                        Boost_DYCRMMfatjetphi  = HNFatJet.Phi();
                                        Boost_DYCRMMfatjetSDM  = HNFatJet.SDMass();
                                        Boost_DYCRMMmll        = lowmllmass;
                                        Boost_DYCRMMWRpt       = WRCand.Pt();
                                        Boost_DYCRMMWReta      = WRCand.Eta();
                                        Boost_DYCRMMWRphi      = WRCand.Phi();
                                        Boost_DYCRMMleadlepeta    = LeadLep->Eta();
                                        Boost_DYCRMMleadlepphi    = LeadLep->Phi();
                                        Boost_DYCRMMsubleadlepeta = LowMllLooseLepton->Eta();
                                        Boost_DYCRMMsubleadlepphi = LowMllLooseLepton->Phi();
                                        Boost_DYCRMMfatjet_lsf3               = HNFatJet.LSF3();
                                        Boost_DYCRMMsubleadlep_miniiso        = LowMllLooseLepton->MiniPFRelIso();
                                        Boost_DYCRMMdeltaR_leadlep_fatjet     = LeadLep->DeltaR(HNFatJet);
                                        Boost_DYCRMMdphi_leadlep_fatjet       = (float)abs(LeadLep->DeltaPhi(HNFatJet));
                                        Boost_DYCRMMpileup_num = ev.nTrueInt();
                                        Boost_DYCRMMpvgood = ev.nPVsGood();
                                        Boost_DYCRMMpv = ev.nPV();
                                        Boost_DYCRMMjet_num    = (float)selected_jets.size();
                                        
                                        float tightmuon1_org_pt = Tight_muons[0]->OriginalPt();                                        
                                        float tightmuon1_pt = Tight_muons[0]->Pt();                                        
                                            if (tightmuon1_org_pt != tightmuon1_pt) {
                                                float rel = tightmuon1_pt / tightmuon1_org_pt;
                                                FillHist(this_syst + "/Muon_TuneP_RelPt", rel, weight, 100, -1., 1.);
                                            }
                                        
                                        if (looselepton_infatjet){
                                            FillHist(this_syst + "/Boosted_DY_CR_MM_looselepton_infatjet_Fatjet_SDMass", HNFatJet.SDMass() , weight, 10000, 0., 10000.);
                                        }
                                        else{
                                            FillHist(this_syst + "/Boosted_DY_CR_MM_looselepton_outsidefatjet_Fatjet_SDMass", HNFatJet.SDMass() , weight, 10000, 0., 10000.);
                                        }
                                    }
                                
                                // Subsets of the 60-150 DY CR above, so the Boost_DYCR{EE,MM}*
                                // kinematics are already cached there; only the flag is needed.
                                if (lowmllmass < 100){
                            // Boosted CR1
                                    if (is_tmp_lead_el){ // ee
                                        is_Boosted_DY_EE_CR1 = true;
                                    }
                                    if (is_tmp_lead_mu){// mumu
                                        is_Boosted_DY_MM_CR1 = true;
                                    }
                                }
                                else{ // 100 < lowmllmass < 150
                                    if (is_tmp_lead_el){ // ee
                                        is_Boosted_DY_EE_CR2 = true;
                                    }
                                    if (is_tmp_lead_mu){// mumu
                                        is_Boosted_DY_MM_CR2 = true;
                                    }
                            // Boosted CR2
                                }
                            }
                            break;
                        }// lead lep fat jet backto back end 
                    }
                    FillHist(this_syst + "/fatjet_DYBoost_num", fatjet_DYBoost_num , weight, 10, 0., 10.);
                } // has low mll end
            // dont have low mll ( > 150 )
                else{
                    if (is_tmp_lead_el){
                    FillHist(this_syst + "/Cutflow_for_e_mujet", 6.0 , 1.0, 20, 0., 20.);
                    FillHist(this_syst + "/Cutflow_for_e_mujet", 7.0 , 1.0,20, 0., 20.);
                    }
                    if (is_tmp_lead_mu){
                    FillHist(this_syst + "/Cutflow_for_mu_ejet", 6.0 , 1.0, 20, 0., 20.);
                    FillHist(this_syst + "/Cutflow_for_mu_ejet", 7.0 , 1.0, 20, 0., 20.);
                    }
                    
                    FillHist(this_syst + "/Cutflow_for_Boosted_SR", 6.0 , weight, 13, 0., 13.);
                    FillSignalCutflow(this_syst, false, 6.0, weight);
                    FillHist(this_syst + "/Boost_cutflow_FLV", 4 , weight, 20,-10,10.);
                    bool hasawaymergedfatjet = false;
                    FatJet Ncand;
                    Particle WRCand;
                    FatJet HNFatJet;

                    for (unsigned int i=0 ; i<fatjets_LSF.size(); i++) {
                        FatJet this_fatjet = fatjets_LSF.at(i);
                        FillHist(this_syst + "/deltaPhi_LeadLep_Fatjet_FLV", abs( LeadLep->DeltaPhi(this_fatjet)) , weight, 100, 0., 3.5);
                        if (abs( LeadLep->DeltaPhi(this_fatjet))>2.0) {
                            
                            HNFatJet = this_fatjet;
                            Ncand = HNFatJet;
                            WRCand = *LeadLep + Ncand;
                            hasawaymergedfatjet = true;
                            break;
                        }
                    }
                    // 이거 만족하는것 중에 리딩 골라야 하는거 아닌가?
                    //if  lead lep delta phi cut > 2.0 with fatjet
                    if (hasawaymergedfatjet) {
                        if (is_tmp_lead_el){
                        FillHist(this_syst + "/Cutflow_for_e_mujet", 8.0 , 1.0, 20, 0., 20.);
                        }
                        if (is_tmp_lead_mu){
                        FillHist(this_syst + "/Cutflow_for_mu_ejet", 8.0 , 1.0, 20, 0., 20.);
                        }
                        FillHist(this_syst + "/Cutflow_for_Boosted_SR", 7.0 , weight,13, 0., 13.);
                        FillSignalCutflow(this_syst, false, 7.0, weight);
                        FillHist(this_syst + "/Boost_cutflow_FLV", 5 , weight, 20,-10,10.);
                        bool hassflooselepton(false);
                        bool hassflooseleptonoutfatjet (false);
                        bool hasoflooselepton(false);
                        bool emucontamination (false);
                        Lepton *SFLooseLepton = nullptr;
                        Lepton *OFLooseLepton = nullptr;
                        FillHist(this_syst +"/numofhnfatjet_FLV", HNFatJet.SDMass() , weight, 10000, 0., 10000.); 
                    //num SF loose lepton inside fatjet for high mll
                        
                        int SFLooselep_inout_fatjet = 0;
                        int SFLooselep_in_fatjet = 0 ;
                        int SFLooselep_out_fatjet = 0 ; 
                        
                        for (unsigned int k=0 ; k< Loose_SF_leps.size(); k ++) {
                            if (LeadLep->DeltaR( *Loose_SF_leps[k])<0.01) continue;
                            FillHist(this_syst + "/Loosesflepton_pt_beforeDR", Loose_SF_leps.at(k)->Pt() , weight, 100, 0., 500.);
                            if ( Loose_SF_leps.at(k)->Pt() < 53.0) continue;
                            FillHist(this_syst + "/Loosesfleptondr", HNFatJet.DeltaR(*Loose_SF_leps[k]) , weight, 100, 0., 5.);
                            hassflooseleptonoutfatjet = true ; // 이게 살아남는 경우는 sf loose lep 이 팻젯 밖인 경우 
                            SFLooselep_inout_fatjet += 1; // 어차피 sf 가 팻젯 안으로 안 들어가면 break 안일어남 -> 모든 loose lep 셀 수 있음
                            if (HNFatJet.DeltaR(*Loose_SF_leps[k]) < 0.8) {
                                SFLooselep_in_fatjet += 1 ; 
                                hassflooseleptonoutfatjet = false ; 
                                hassflooselepton = true;
                                SFLooseLepton = Loose_SF_leps[k];
                                FillHist(this_syst + "/SFLooseLepton_infatjet_miniiso_beforecut", SFLooseLepton->MiniPFRelIso(), weight, 200, 0., 1.);
                                break;
                            }
                            else{
                                SFLooselep_out_fatjet += 1 ; 
                            }
                        }
                        
                    //num OF loose lepton inside fatjet for high mll

                        for (unsigned int m=0 ; m< Loose_OF_leps.size(); m ++) {
                            if (LeadLep->DeltaR( *Loose_OF_leps[m])<0.01) continue;
                            FillHist(this_syst + "/Looseoflepton_pt_beforeDR", Loose_OF_leps.at(m)->Pt() , weight, 100, 0., 500.);
                            if ( Loose_OF_leps.at(m)->Pt() < 53.0) continue;
                            FillHist(this_syst + "/Looseofleptondr", HNFatJet.DeltaR(*Loose_OF_leps[m]) , weight, 100, 0., 5.);
                            if (HNFatJet.DeltaR(*Loose_OF_leps[m]) < 0.8) {
                                hasoflooselepton = true;
                                OFLooseLepton = Loose_OF_leps[m];
                                FillHist(this_syst + "/OFLooseLepton_infatjet_miniiso_beforecut", OFLooseLepton->MiniPFRelIso(), weight, 200, 0., 1.);
                                cout << "has of loose lepton in fatjet and has " << hassflooseleptonoutfatjet << "number of sf loose lepton out fatjet" << endl ;
                                if (hassflooseleptonoutfatjet){
                                    // case of SF loose lepton outside fatjet but OF in fatjet 
                                    emucontamination = true;
                                }
                                break;
                            }
                        }
                        // 여기 둘다 다 모아 놓고 플레이버가 안에 다른게 있으면 탈락 , 다 같으면 리딩만 골라야
                        //그래서 SF 인 경우 OF 인 경우 모두 같은 플레이버 들어있으니까 리딩만 골라야?
                        // Veto tight lepton
                        int NExtraTightLepton(0);
                            for (unsigned int i=0 ; i< Tight_leps.size(); i ++) {
                                if ( Tight_leps[i] == LeadLep || Tight_leps[i] == SFLooseLepton || Tight_leps[i] == OFLooseLepton ) continue;
                                FillHist(this_syst + "/ExtraTightLepton_pt", Tight_leps.at(i)->Pt() , weight, 100, 0., 500.);
                                NExtraTightLepton++;
                            }
                        FillHist(this_syst + "/Num_of_ExtraTightLepton", NExtraTightLepton , weight, 10, 0., 10.);
                        bool hasnoextralep = (NExtraTightLepton == 0);
                        bool WRMassGT800 = ( WRCand.M() > 800.0 );
                        if (hasnoextralep ){
                            FillHist(this_syst + "/Cutflow_for_Boosted_SR", 8.0 , weight, 13, 0., 13.);
                            FillSignalCutflow(this_syst, false, 8.0, weight);
                            // tight fatjet 밖 한개 , loose lepton same flavor 안에 
                            FillHist(this_syst + "/Boost_cutflow_FLV", 6 , weight, 20, -10., 10.);
                            // miniIso-only aux cutflow: bin0 = SF loose lep in fatjet, bin1 = bin0 + miniIso<0.1
                            // (main cutflow bin numbers stay identical to Reproduce20_002_copy)
                            if (hassflooselepton) {
                                TString miniiso_cf = is_tmp_lead_el ? "/SR_EE_miniiso_cutflow" : "/SR_MM_miniiso_cutflow";
                                FillHist(this_syst + miniiso_cf, 0.0, weight, 2, 0., 2.);
                                if (SFLooseLepton->MiniPFRelIso() < fatjet_set.Sublead_MiniIso) FillHist(this_syst + miniiso_cf, 1.0, weight, 2, 0., 2.);
                            }
                            if (hassflooselepton && (SFLooseLepton->MiniPFRelIso() < fatjet_set.Sublead_MiniIso)) {
                                FillHist(this_syst + "/Cutflow_for_Boosted_SR", 9.0 , weight, 13, 0., 13.);
                                FillSignalCutflow(this_syst, false, 9.0, weight);
                                FillHist(this_syst + "/Boost_cutflow_FLV", 7 , weight, 20,-10,10.);
                                if (!hasoflooselepton){
                                    FillHist(this_syst + "/Cutflow_for_Boosted_SR", 10.0 , weight, 13, 0., 13.);
                                    FillSignalCutflow(this_syst, false, 10.0, weight);
                                    FillHist(this_syst + "/Boost_cutflow_FLV", 8 , weight, 20,-10,10.);
                        
                        //    if(tmp_IsLeadM){
                        //==== In this case, the loose ID is HighPt ID muon.
                        //==== we want to apply the lepton scale factors to these muons
                        //Muon *looseMuon = (Muon *)SFLooseLepton;
                        //ForSF_muons.push_back( looseMuon );
                        //    }
                                    if ( (*LeadLep + *SFLooseLepton).M() >200.0 ) {
                                        FillHist(this_syst + "/Cutflow_for_Boosted_SR", 11.0 , weight, 13, 0., 13.);
                                        FillSignalCutflow(this_syst, false, 11.0, weight);
                                        //charge 
                                        float LeadLepCharge = LeadLep->Charge();
                                        float SFLooseLeptonCharge = SFLooseLepton->Charge();
                                        FillHist(this_syst + "/Boost_cutflow_FLV", 9 , weight, 20,-10,10.);
                                        // Boosted DY SR
                                        if (WRMassGT800) {
                                            if(!IsDATA){
                                                //SF
                                                if(is_tmp_lead_el){
                                                    FillHist (this_syst + "/Cutflow_for_Boosted_SR", 12.0 , weight, 13, 0., 13.);
                                                    FillSignalCutflow(this_syst, false, 12.0, weight);
                                                    
                                                    weight_function_map["E_Reco_Weight"] = [&](MyCorrection::variation var, TString source)  {
                                                    return (myCorr->GetElectronRECOSF(((Electron *)LeadLep)->scEta(), LeadLep->Pt(), LeadLep->Phi(),var)) ;};
                                                    if (DataEra != "2017") {
                                                    weight_function_map["E_Trig_Weight"] = [&](MyCorrection::variation var, TString source)  {
                                                        return GetElectronTriggerSF_TnP(Tight_electrons[0]->scEta(), Tight_electrons[0]->Pt(), var);
                                                    };
                                                }
                                                    // SR EE: the loose lepton inside the fatjet is an electron, so the
                                                    // miniIso cut that replaced LSF3 carries its egamma-tnp SF.
                                                    // scEta/pt are read out here rather than inside the lambda:
                                                    // SFLooseLepton is out of scope by the time calculateWeight() runs.
                                                    {
                                                        const double miniiso_sceta = ((Electron *)SFLooseLepton)->scEta();
                                                        const double miniiso_pt    = SFLooseLepton->Pt();
                                                        weight_function_map["MiniIso_Weight"] = [this, miniiso_sceta, miniiso_pt](MyCorrection::variation var, TString source) -> float {
                                                            return GetElectronMiniIsoSF_TnP(miniiso_sceta, miniiso_pt, var);
                                                        };
                                                    }

                                                }
                                                if(is_tmp_lead_mu){
                                                    FillHist (this_syst + "/Cutflow_for_Boosted_SR", 12.0 , weight, 13, 0., 13.);
                                                    FillSignalCutflow(this_syst, false, 12.0, weight);
                                                    
                                                    //temp_two_muon.clear();
                                                    //Muon * SFLooseLepton_mu = (Muon *)SFLooseLepton;
                                                    //temp_two_muon.push_back(*SFLooseLepton_mu);

                                                    
                                                    weight_function_map["M_Id_Weight"] = [&](MyCorrection::variation var, TString source) -> float   {
                                                    if (DataEra=="2017") return 1.0;
                                                    return (myCorr->GetMuonIDSF("NUM_HighPtID_DEN_GlobalMuonProbes", *Tight_muons[0],var));};
                                                    weight_function_map["M_Reco_Weight"] = [&](MyCorrection::variation var, TString source)  {
                                                    return (myCorr->GetMuonHighPtRECOSF(*Tight_muons[0],var));};
                                                    
                                                    // Fix: capture SFLooseLepton by value to avoid dangling reference
                                                    // (SFLooseLepton goes out of scope before lambda is evaluated at systHelper->calculateWeight())
                                                    
                                                    weight_function_map["M_Trig_Weight"] = [&, SFLooseLepton](MyCorrection::variation var, TString source) -> float {
                                                        RVec<Muon*> trig_muons;
                                                        trig_muons.push_back(Tight_muons[0]);
                                                        trig_muons.push_back((Muon*)SFLooseLepton);
                                                        if (DataEra=="2017") return 1.0;
                                                    return myCorr->GetMuonTriggerSF("NUM_HLT_DEN_HighPtLooseRelIsoProbes", trig_muons, var);};

                                                    weight_function_map["M_Iso_Weight"] = [&](MyCorrection::variation var, TString source) -> float {
                                                    if (DataEra=="2017") return 1.0;
                                                        return (myCorr->GetMuonIDSF("NUM_probe_LooseRelTkIso_DEN_HighPtProbes",*Tight_muons[0],var));};

                                                    // SR MM: the loose lepton inside the fatjet is a muon. No MiniIso_Weight --
                                                    // the miniIso SF is an electron-only measurement, so this region takes
                                                    // the miniIso cut without a SF.

                                                }
                                            }
                                                if (is_tmp_lead_el) {
                                                    //SF 
                                                    
                                                    is_Boosted_SR_EE = true;
                                                    Boost_SREEpt = (*LeadLep + *SFLooseLepton).Pt();
                                                    Boost_SREEfatjetpt = HNFatJet.Pt();
                                                    Boost_SREEmlljj = WRCand.M();
                                                    Boost_SREEleadleppt = LeadLep->Pt();
                                                    Boost_SREEsubleadleppt = SFLooseLepton->Pt();
                                                    // new variables
                                                    Boost_SREEmass    = (*LeadLep + *SFLooseLepton).M();
                                                    Boost_SREEeta     = (*LeadLep + *SFLooseLepton).Eta();
                                                    Boost_SREEphi     = (*LeadLep + *SFLooseLepton).Phi();
                                                    Boost_SREEfatjeteta  = HNFatJet.Eta();
                                                    Boost_SREEfatjetphi  = HNFatJet.Phi();
                                                    Boost_SREEfatjetSDM  = HNFatJet.SDMass();
                                                    Boost_SREEmll        = (*LeadLep + *SFLooseLepton).M();
                                                    Boost_SREEWRpt       = WRCand.Pt();
                                                    Boost_SREEWReta      = WRCand.Eta();
                                                    Boost_SREEWRphi      = WRCand.Phi();
                                                    Boost_SREEleadlepeta    = LeadLep->Eta();
                                                    Boost_SREEleadlepphi    = LeadLep->Phi();
                                                    Boost_SREEsubleadlepeta = SFLooseLepton->Eta();
                                                    Boost_SREEsubleadlepphi = SFLooseLepton->Phi();
                                                    Boost_SREEfatjet_lsf3               = HNFatJet.LSF3();
                                                    Boost_SREEsubleadlep_miniiso        = SFLooseLepton->MiniPFRelIso();
                                                    Boost_SREEdeltaR_leadlep_fatjet     = LeadLep->DeltaR(HNFatJet);
                                                    Boost_SREEdphi_leadlep_fatjet       = (float)abs(LeadLep->DeltaPhi(HNFatJet));
                                                    Boost_SREEpileup_num = ev.nTrueInt();
                                                    Boost_SREEpvgood = ev.nPVsGood();
                                                    Boost_SREEpv = ev.nPV();
                                                    Boost_SREEjet_num    = (float)selected_jets.size();
                                                    
                                                    for (int i = 0; i < lhe.size(); i++) {
                                                        float q1 = lhe[i].PdgId();
                                                        float q2 = lhe[i].PdgId();
                                                        if (i > 2) {
                                                            if (abs(q1) < 7 && abs(q2) < 7) {
                                                                FillHist( this_syst + "/Boosted_SR_EE_q", q1, weight, 14, -7., 7.);
                                                                FillHist( this_syst + "/Boosted_SR_EE_q", q2, weight, 14, -7., 7.);
                                                            }
                                                        }
                                                    }
                                                    //charge
                                                    if ( LeadLepCharge * SFLooseLeptonCharge > 0 ) {
                                                        is_Boosted_SR_EE_SS = true;
                                                        
                                                    }
                                                    else {
                                                        is_Boosted_SR_EE_OS = true;
                                                        
                                                    }
                                                    electron2_tight_charge = ((Electron*)SFLooseLepton)->TightCharge();
                                                    FillHist(this_syst + "/Boosted_SR_EE_electron1_tight_charge", electron1_tight_charge , 1.0, 5, 0., 5.);
                                                    FillHist(this_syst + "/Boosted_SR_EE_electron2_tight_charge", electron2_tight_charge , 1.0, 5, 0., 5.);
                                                    {
                                                        bool e1_tight = (electron1_tight_charge == 2);
                                                        bool e2_tight = (electron2_tight_charge == 2);
                                                        int n_e_tight = (e1_tight ? 1 : 0) + (e2_tight ? 1 : 0);
                                                        if (is_Boosted_SR_EE_SS) {
                                                            if (n_e_tight == 2) is_Boosted_SR_EE_SS_2e_tight = true;
                                                            else if (n_e_tight == 1) is_Boosted_SR_EE_SS_1e_tight = true;
                                                            else is_Boosted_SR_EE_SS_0e_tight = true;
                                                        } else if (is_Boosted_SR_EE_OS) {
                                                            if (n_e_tight == 2) is_Boosted_SR_EE_OS_2e_tight = true;
                                                            else if (n_e_tight == 1) is_Boosted_SR_EE_OS_1e_tight = true;
                                                            else is_Boosted_SR_EE_OS_0e_tight = true;
                                                        }
                                                    }
                                                }
                                                if (is_tmp_lead_mu) {
                                                    
                                                    is_Boosted_SR_MM = true;
                                                    muon1_tight_charge = Tight_muons[0]->TightCharge() ;
                                                    muon2_tight_charge = ((Muon*)SFLooseLepton)->TightCharge() ;
                                                    FillHist(this_syst + "/Boosted_SR_MM_muon_tight_charge", muon1_tight_charge + muon2_tight_charge , 1.0, 5, 0., 5.);
                                                    bool tight_charge = ((muon1_tight_charge==2) && ( muon2_tight_charge == 2)) ;

                                                    Boost_SRMMpt = (*LeadLep + *SFLooseLepton).Pt();
                                                    Boost_SRMMfatjetpt = HNFatJet.Pt();
                                                    Boost_SRMMmlljj = WRCand.M();
                                                    Boost_SRMMleadleppt = LeadLep->Pt();
                                                    Boost_SRMMsubleadleppt = SFLooseLepton->Pt();
                                                    // new variables
                                                    Boost_SRMMmass    = (*LeadLep + *SFLooseLepton).M();
                                                    Boost_SRMMeta     = (*LeadLep + *SFLooseLepton).Eta();
                                                    Boost_SRMMphi     = (*LeadLep + *SFLooseLepton).Phi();
                                                    Boost_SRMMfatjeteta  = HNFatJet.Eta();
                                                    Boost_SRMMfatjetphi  = HNFatJet.Phi();
                                                    Boost_SRMMfatjetSDM  = HNFatJet.SDMass();
                                                    Boost_SRMMmll        = (*LeadLep + *SFLooseLepton).M();
                                                    Boost_SRMMWRpt       = WRCand.Pt();
                                                    Boost_SRMMWReta      = WRCand.Eta();
                                                    Boost_SRMMWRphi      = WRCand.Phi();
                                                    Boost_SRMMleadlepeta    = LeadLep->Eta();
                                                    Boost_SRMMleadlepphi    = LeadLep->Phi();
                                                    Boost_SRMMsubleadlepeta = SFLooseLepton->Eta();
                                                    Boost_SRMMsubleadlepphi = SFLooseLepton->Phi();
                                                    Boost_SRMMfatjet_lsf3               = HNFatJet.LSF3();
                                                    Boost_SRMMsubleadlep_miniiso        = SFLooseLepton->MiniPFRelIso();
                                                    Boost_SRMMdeltaR_leadlep_fatjet     = LeadLep->DeltaR(HNFatJet);
                                                    Boost_SRMMdphi_leadlep_fatjet       = (float)abs(LeadLep->DeltaPhi(HNFatJet));
                                                    Boost_SRMMpileup_num = ev.nTrueInt();
                                                    Boost_SRMMpvgood = ev.nPVsGood();
                                                    Boost_SRMMpv = ev.nPV();
                                                    Boost_SRMMjet_num    = (float)selected_jets.size();
                                                    float tightmuon1_org_pt = Tight_muons[0]->OriginalPt();
                                                    float tightmuon1_pt = Tight_muons[0]->Pt();                                        
                                                    if (tightmuon1_org_pt != tightmuon1_pt) {
                                                        float rel = tightmuon1_pt / tightmuon1_org_pt;
                                                        FillHist(this_syst + "/Muon_TuneP_RelPt", rel, weight, 100, -1., 1.);
                                                    }
                                                    for (int i = 0; i < lhe.size(); i++) {
                                                        float q1 = lhe[i].PdgId();
                                                        float q2 = lhe[i].PdgId();
                                                        if (i > 2) {
                                                            if (abs(q1) < 7 && abs(q2) < 7) {
                                                                FillHist( this_syst + "/Boosted_SR_MM_q", q1, weight, 14, -7., 7.);
                                                                FillHist( this_syst + "/Boosted_SR_MM_q", q2, weight, 14, -7., 7.);
                                                            }
                                                        }
                                                    }
                                                    //charge
                                                    if ( LeadLepCharge * SFLooseLeptonCharge > 0 ) {
                                                        is_Boosted_SR_MM_SS = true;
                                                        if (tight_charge) {
                                                            is_Boosted_SR_MM_SS_tight = true;
                                                        }
                                                        else {
                                                            is_Boosted_SR_MM_SS_not_tight = true;
                                                        }
                                                    }
                                                    else {
                                                        is_Boosted_SR_MM_OS = true;
                                                        if (tight_charge) {
                                                            is_Boosted_SR_MM_OS_tight = true;
                                                        }
                                                        else {
                                                            is_Boosted_SR_MM_OS_not_tight = true;
                                                        }
                                                    }
                                                }                                            
                                            
                                        }
                                        else{ // low wr CR
                                            if (is_tmp_lead_el) {
                                                // Boosted low WR ee CR
                                            }
                                            else if (is_tmp_lead_mu) {
                                                // Boosted low WR mumu CR
                                            }
                                        }
                                        // Boosted DY CR
                                    }
                                }
                            }
                    if(is_tmp_lead_el){
                    FillHist(this_syst + "/Cutflow_for_e_mujet", 9.0 , 1.0,20, 0., 20.);
                    }
                    if(is_tmp_lead_mu){
                    FillHist(this_syst + "/Cutflow_for_mu_ejet", 9.0 , 1.0, 20, 0., 20.);
                    }
                    // tight fatjet 밖 한개 , loose lepton oppo flavor 안에
                        if (!hassflooselepton){
                            if(is_tmp_lead_el){
                                FillHist(this_syst + "/Cutflow_for_e_mujet", 10.0 , 1.0,20, 0., 20.);
                            }
                            if(is_tmp_lead_mu){
                                FillHist(this_syst + "/Cutflow_for_mu_ejet", 10.0 , 1.0, 20, 0., 20.);
                            }
                            FillHist(this_syst + "/Boost_cutflow_FLV", -6 , weight, 20, -10, 10.);
                            // miniIso-only aux cutflow: bin0 = OF loose lep in fatjet, bin1 = bin0 + miniIso<0.1
                            if (hasoflooselepton) {
                                TString miniiso_cf = is_tmp_lead_el ? "/FlavEMJ_miniiso_cutflow" : "/FlavMEJ_miniiso_cutflow";
                                FillHist(this_syst + miniiso_cf, 0.0, weight, 2, 0., 2.);
                                if (OFLooseLepton->MiniPFRelIso() < fatjet_set.Sublead_MiniIso) FillHist(this_syst + miniiso_cf, 1.0, weight, 2, 0., 2.);
                            }
                            if (hasoflooselepton && (OFLooseLepton->MiniPFRelIso() < fatjet_set.Sublead_MiniIso)){
                                if(is_tmp_lead_el){
                                    FillHist(this_syst + "/Cutflow_for_e_mujet", 11.0 , 1.0,20, 0., 20.);
                                }
                                if(is_tmp_lead_mu){
                                    FillHist(this_syst + "/Cutflow_for_mu_ejet", 11.0 , 1.0, 20, 0., 20.);
                                }   
                                // Boosted Flavor CR
                                if ((*LeadLep + *OFLooseLepton).M() > 200.0) {
                                    if(is_tmp_lead_el){
                                        FillHist(this_syst + "/Cutflow_for_e_mujet", 12.0 , 1.0,20, 0., 20.);
                                    }
                                    if(is_tmp_lead_mu){
                                        FillHist(this_syst + "/Cutflow_for_mu_ejet", 12.0 , 1.0, 20, 0., 20.);
                                    }   
                                    FillHist(this_syst + "/Boost_cutflow_FLV", -7 , weight, 20, -10, 10.);
                                    if (WRMassGT800) {
                                        if(is_tmp_lead_el){
                                            FillHist(this_syst + "/Cutflow_for_e_mujet", 13.0 , 1.0,20, 0., 20.);
                                        }
                                    if(is_tmp_lead_mu){
                                            FillHist(this_syst + "/Cutflow_for_mu_ejet", 13.0 , 1.0, 20, 0., 20.);
                                        }   
                                        FillHist(this_syst + "/Boost_cutflow_FLV", -8 , weight, 20, -10, 10.);
                                        if(!IsDATA){
                                            if(is_tmp_lead_el){
                                                        //float ElIDSF = heep_ID_weight;
                                                        //cout<<"ok1"<<endl;
                                                        
                                                        weight_function_map["E_Reco_Weight"] = [&](MyCorrection::variation var, TString source)  {
                                                        return (myCorr->GetElectronRECOSF(((Electron *)LeadLep)->scEta(), LeadLep->Pt(), LeadLep->Phi(),var)) ;};
                                                        if (DataEra != "2017") {
                                                        weight_function_map["E_Trig_Weight"] = [&](MyCorrection::variation var, TString source)  {
                                                            return GetElectronTriggerSF_TnP(Tight_electrons[0]->scEta(), Tight_electrons[0]->Pt(), var);
                                                        };
                                                    }
                                                        // Flavour CR e-mujet: the loose lepton inside the fatjet is a
                                                        // muon -> no miniIso SF (electron-only measurement).
                                                }
                                                if(is_tmp_lead_mu){
                                                    ////cout<<"ok11"<<endl;
                                                    
                                                    weight_function_map["M_Id_Weight"] = [&](MyCorrection::variation var, TString source) -> float   {
                                                    if (DataEra=="2017") return 1.0;
                                                    return (myCorr->GetMuonIDSF("NUM_HighPtID_DEN_GlobalMuonProbes", *Tight_muons[0],var));};
                                                    weight_function_map["M_Reco_Weight"] = [&](MyCorrection::variation var, TString source)  {
                                                    return (myCorr->GetMuonHighPtRECOSF(*Tight_muons[0],var));};
                                                    
                                                    weight_function_map["M_Trig_Weight"] = [&](MyCorrection::variation var, TString source) -> float {
                                                    if (DataEra=="2017") return 1.0;
                                                        return (myCorr->GetMuonTriggerSF("NUM_HLT_DEN_HighPtLooseRelIsoProbes",*Tight_muons[0],var));};

                                                    weight_function_map["M_Iso_Weight"] = [&](MyCorrection::variation var, TString source) -> float {
                                                        if (DataEra=="2017") return 1.0;
                                                    return (myCorr->GetMuonIDSF("NUM_probe_LooseRelTkIso_DEN_HighPtProbes",*Tight_muons[0],var));};

                                                    // Flavour CR mu-ejet: the loose lepton inside the fatjet is an
                                                    // electron -> apply the miniIso SF on the MiniIso_Weight target.
                                                    // scEta/pt read out here: OFLooseLepton is out of scope by the
                                                    // time calculateWeight() evaluates the lambda.
                                                    {
                                                        const double miniiso_sceta = ((Electron *)OFLooseLepton)->scEta();
                                                        const double miniiso_pt    = OFLooseLepton->Pt();
                                                        weight_function_map["MiniIso_Weight"] = [this, miniiso_sceta, miniiso_pt](MyCorrection::variation var, TString source) -> float {
                                                            return GetElectronMiniIsoSF_TnP(miniiso_sceta, miniiso_pt, var);
                                                        };
                                                    }

                                                }
                                        }
                                        if (is_tmp_lead_el) {
                                            ////cout<<"ok2"<<endl;
                                            is_Boosted_Flav_EMJ = true;
                                            if (emucontamination){
                                                cout << "!!!!!!!! IT HAS CONTAMINATION" << endl;
                                                FillHist(this_syst + "/Boosted_Flav_EMJ_elout_fat_mu_in_fat",1,weight, 10,0,10);
                                            }
                                            for (int i = 0; i < lhe.size(); i++) {
                                                float el = lhe[i].PdgId();
                                                if (abs(el)==11) {
                                                    float el_pt = lhe[i].Pt();
                                                    float el_eta = lhe[i].Eta();
                                                    FillHist(this_syst + "/Boosted_Flav_EMJ_el_pt",el_pt,weight, 1000,0,1000);
                                                    FillHist(this_syst + "/Boosted_Flav_EMJ_el_eta",el_eta,weight, 1000,0,1000);
                                                }
                                            }

                                            FillHist(this_syst + "/Boosted_Flav_EMJ_total_loose_lep",SFLooselep_inout_fatjet, 1, 10,0,10);
                                            FillHist(this_syst + "/Boosted_Flav_EMJ_in_fatjet_loose_lep",SFLooselep_in_fatjet, 1, 10,0,10);
                                            FillHist(this_syst + "/Boosted_Flav_EMJ_out_fatjet_loose_lep",SFLooselep_out_fatjet, 1, 10,0,10);
                                            //cout << SFLooselep_inout_fatjet << endl;
                                            muon1_tight_charge = ((Muon*)OFLooseLepton)->TightCharge() ;
                                            FillHist(this_syst + "/Boosted_Flav_EMJ_muon_tight_charge", muon1_tight_charge , 1.0, 3, 0., 3.);
                                            bool emj_m_tight = (muon1_tight_charge == 2);
                                            bool emj_e_tight = (electron1_tight_charge == 2);
                                            if (LeadLep->Charge() * OFLooseLepton->Charge() > 0) {
                                                is_Boosted_Flav_EMJ_SS = true;
                                                if (emj_m_tight) {
                                                    is_Boosted_Flav_EMJ_SS_tight = true;
                                                }
                                                else {
                                                    is_Boosted_Flav_EMJ_SS_not_tight = true;
                                                }
                                                if (emj_m_tight && emj_e_tight) is_Boosted_Flav_EMJ_SS_M_tight_E_tight = true;
                                                else if (emj_m_tight && !emj_e_tight) is_Boosted_Flav_EMJ_SS_M_tight_E_not_tight = true;
                                                else if (!emj_m_tight && emj_e_tight) is_Boosted_Flav_EMJ_SS_M_not_tight_E_tight = true;
                                                else is_Boosted_Flav_EMJ_SS_M_not_tight_E_not_tight = true;
                                            } else {
                                                is_Boosted_Flav_EMJ_OS = true;
                                                if (emj_m_tight) {
                                                    is_Boosted_Flav_EMJ_OS_tight = true;
                                                }
                                                else {
                                                    is_Boosted_Flav_EMJ_OS_not_tight = true;
                                                }
                                                if (emj_m_tight && emj_e_tight) is_Boosted_Flav_EMJ_OS_M_tight_E_tight = true;
                                                else if (emj_m_tight && !emj_e_tight) is_Boosted_Flav_EMJ_OS_M_tight_E_not_tight = true;
                                                else if (!emj_m_tight && emj_e_tight) is_Boosted_Flav_EMJ_OS_M_not_tight_E_tight = true;
                                                else is_Boosted_Flav_EMJ_OS_M_not_tight_E_not_tight = true;
                                            }
                                            Boost_FlavEMJpt = (*LeadLep + *OFLooseLepton).Pt();
                                            Boost_FlavEMJleadfatjetpt = HNFatJet.Pt();
                                            Boost_FlavEMJmlljj = WRCand.M();
                                            Boost_FlavEMJSDM = HNFatJet.SDMass();
                                            Boost_FlavEMJleadleppt = LeadLep->Pt();
                                            Boost_FlavEMJsubleadleppt = OFLooseLepton->Pt();
                                            //eta 
                                            Boost_FlavEMJleadlepeta = LeadLep->Eta();
                                            Boost_FlavEMJsubleadlepeta = OFLooseLepton->Eta();
                                            Boost_FlavEMJfatjeteta = HNFatJet.Eta();
                                            Boost_FlavEMJfatjetphi = HNFatJet.Phi();

                                            Boost_FlavEMJdeltaR_leadlep_fatjet = LeadLep->DeltaR(HNFatJet);


                                            // lsf
                                            Boost_FlavEMJleadlep_lsf = HNFatJet.LSF3();
                                            
                                            //mll
                                            Boost_FlavEMJmll = (*LeadLep + *OFLooseLepton).M();
                                            // fatjet pt 
                                            Boost_FlavEMJfatjetpt = HNFatJet.Pt();
                                            // wr pt
                                            Boost_FlavEMJWRpt = WRCand.Pt();
                                            // new variables
                                            Boost_FlavEMJmass    = (*LeadLep + *OFLooseLepton).M();
                                            Boost_FlavEMJeta     = (*LeadLep + *OFLooseLepton).Eta();
                                            Boost_FlavEMJphi     = (*LeadLep + *OFLooseLepton).Phi();
                                            Boost_FlavEMJWReta   = WRCand.Eta();
                                            Boost_FlavEMJWRphi   = WRCand.Phi();
                                            Boost_FlavEMJleadlepphi    = LeadLep->Phi();
                                            Boost_FlavEMJsubleadlepphi = OFLooseLepton->Phi();
                                            Boost_FlavEMJfatjet_lsf3   = HNFatJet.LSF3();
                                            Boost_FlavEMJsubleadlep_miniiso = OFLooseLepton->MiniPFRelIso();
                                            Boost_FlavEMJdphi_leadlep_fatjet = (float)abs(LeadLep->DeltaPhi(HNFatJet));
                                            Boost_FlavEMJpileup_num = ev.nTrueInt();
                                            Boost_FlavEMJpvgood = ev.nPVsGood();
                                            Boost_FlavEMJpv = ev.nPV();
                                            Boost_FlavEMJjet_num    = (float)selected_jets.size();

                                            for (int i = 0; i < lhe.size(); i++) {
                                                float q1 = lhe[i].PdgId();
                                                float q2 = lhe[i].PdgId();
                                                if (i > 2) {
                                                    if (abs(q1) < 7 && abs(q2) < 7) {
                                                        FillHist( this_syst + "/Boosted_Flav_EMJ_q", q1, 1.0, 14, -7., 7.);
                                                        FillHist( this_syst + "/Boosted_Flav_EMJ_q", q2, 1.0, 14, -7., 7.);
                                                    }
                                                }
                                            }
                                            FillHist(this_syst + "/Boosted_Flav_EMJ_electron1_tight_charge", electron1_tight_charge , 1.0, 5, 0., 5.);

                                            /*
                                            // LHE-reco matching for Flav_EMJ
                                            // Step 1: Match first tight electron (LeadLep) to LHE electron with deltaR < 0.3
                                            bool lhe_match_tight_el = false;
                                            int lhe_matched_tight_el_idx = -1;
                                            for (int i = 0; i < (int)lhe.size(); i++) {
                                                if (abs(lhe[i].PdgId()) == 11) {
                                                    if (LeadLep->DeltaR(lhe[i]) < 0.3) {
                                                        lhe_match_tight_el = true;
                                                        lhe_matched_tight_el_idx = i;
                                                        break;
                                                    }
                                                }
                                            }
                                            FillHist(this_syst + "/Boosted_Flav_EMJ_lhe_match_tight_el", lhe_match_tight_el ? 1.0 : 0.0, weight, 2, 0., 2.);

                                            // Step 2: If tight electron matched, find same-flavor loose electron
                                            // with pt < 53 GeV (did not pass pt cut) and inside fatjet, then match to LHE
                                            if (lhe_match_tight_el) {
                                                bool lhe_match_loose_el_in_fatjet = false;
                                                for (unsigned int k = 0; k < Loose_SF_leps.size(); k++) {
                                                    if (LeadLep->DeltaR(*Loose_SF_leps[k]) < 0.01) continue;   // skip LeadLep
                                                    //if (Loose_SF_leps[k]->Pt() >= 53.0) continue;              // only pt < 53 GeV (did not pass pt cut)
                                                    //if (HNFatJet.DeltaR(*Loose_SF_leps[k]) >= 0.8) continue;  // must be inside fatjet
                                                    for (int i = 0; i < (int)lhe.size(); i++) {
                                                        if (i == lhe_matched_tight_el_idx) continue;  // don't reuse already matched LHE particle
                                                        if (abs(lhe[i].PdgId()) == 11) {
                                                            if (Loose_SF_leps[k]->DeltaR(lhe[i]) < 0.3) {
                                                                lhe_match_loose_el_in_fatjet = true;
                                                                break;
                                                            }
                                                        }
                                                    }
                                                    if (lhe_match_loose_el_in_fatjet) break;
                                                }
                                                FillHist(this_syst + "/Boosted_Flav_EMJ_lhe_match_loose_el_in_fatjet", lhe_match_loose_el_in_fatjet ? 1.0 : 0.0, weight, 2, 0., 2.);

                                                // Gen-LHE matching chain
                                                // Find loose LHE: another LHE electron not chosen as tight LHE
                                                bool lhe_loose_found = false;
                                                int lhe_loose_idx = -1;
                                                for (int i = 0; i < (int)lhe.size(); i++) {
                                                    if (i == lhe_matched_tight_el_idx) continue;
                                                    if (abs(lhe[i].PdgId()) == 11) {
                                                        lhe_loose_found = true;
                                                        lhe_loose_idx = i;
                                                        break;
                                                    }
                                                }
                                                FillHist(this_syst + "/Boosted_Flav_EMJ_lhe_loose_found", lhe_loose_found ? 1.0 : 0.0, weight, 2, 0., 2.);

                                                if (lhe_loose_found) {
                                                    float lhe_loose_charge = lhe[lhe_loose_idx].Charge();
                                                    FillHist(this_syst + "/Boosted_Flav_EMJ_lhe_loose_charge", lhe_loose_charge, weight, 5, -2., 3.);
                                                    FillHist(this_syst + "/Boosted_Flav_EMJ_lhe_loose_pt",  lhe[lhe_loose_idx].Pt(),  weight, 500, 0., 500.);
                                                    FillHist(this_syst + "/Boosted_Flav_EMJ_lhe_loose_eta", lhe[lhe_loose_idx].Eta(), weight, 100, -5., 5.);
                                                    FillHist(this_syst + "/Boosted_Flav_EMJ_lhe_loose_dR_fatjet", HNFatJet.DeltaR(lhe[lhe_loose_idx]), weight, 100, 0., 5.);

                                                    // Match loose LHE to gen electron (PID=+-11) with deltaR < 0.3
                                                    bool gen_lhe_loose_matched = false;
                                                    int gen_matched_idx = -1;
                                                    for (int i = 0; i < (int)gen_set.gens.size(); i++) {
                                                        if (abs(gen_set.gens[i].PID()) == 11) {
                                                            if (lhe[lhe_loose_idx].DeltaR(gen_set.gens[i]) < 0.3) {
                                                                gen_lhe_loose_matched = true;
                                                                gen_matched_idx = i;
                                                                break;
                                                            }
                                                        }
                                                    }
                                                    FillHist(this_syst + "/Boosted_Flav_EMJ_gen_lhe_loose_match", gen_lhe_loose_matched ? 1.0 : 0.0, weight, 2, 0., 2.);

                                                    if (gen_lhe_loose_matched) {
                                                        float gen_matched_charge = gen_set.gens[gen_matched_idx].Charge();
                                                        FillHist(this_syst + "/Boosted_Flav_EMJ_gen_matched_charge", gen_matched_charge, weight, 5, -2., 3.);
                                                        FillHist(this_syst + "/Boosted_Flav_EMJ_gen_matched_pt",  gen_set.gens[gen_matched_idx].Pt(),  weight, 500, 0., 500.);
                                                        FillHist(this_syst + "/Boosted_Flav_EMJ_gen_matched_eta", gen_set.gens[gen_matched_idx].Eta(), weight, 100, -5., 5.);
                                                        FillHist(this_syst + "/Boosted_Flav_EMJ_gen_matched_dR_fatjet", HNFatJet.DeltaR(gen_set.gens[gen_matched_idx]), weight, 100, 0., 5.);

                                                        // Match gen to Loose SF lepton (electron, not LeadLep)
                                                        bool gen_match_loose_sf = false;
                                                        for (unsigned int k = 0; k < Loose_SF_leps.size(); k++) {
                                                            if (LeadLep->DeltaR(*Loose_SF_leps[k]) < 0.01) continue;
                                                            if (gen_set.gens[gen_matched_idx].DeltaR(*Loose_SF_leps[k]) < 0.3) {
                                                                gen_match_loose_sf = true;
                                                                break;
                                                            }
                                                        }
                                                        FillHist(this_syst + "/Boosted_Flav_EMJ_gen_match_loose_sf_lep", gen_match_loose_sf ? 1.0 : 0.0, weight, 2, 0., 2.);

                                                        // Match gen to all electrons (my_electrons)
                                                        bool gen_match_any_el = false;
                                                        for (unsigned int k = 0; k < my_electrons.size(); k++) {
                                                            if (gen_set.gens[gen_matched_idx].DeltaR(my_electrons[k]) < 0.3) {
                                                                gen_match_any_el = true;
                                                                break;
                                                            }
                                                        }
                                                        FillHist(this_syst + "/Boosted_Flav_EMJ_gen_match_any_el", gen_match_any_el ? 1.0 : 0.0, weight, 2, 0., 2.);
                                                    }
                                                }
                                            }
                                            */
                                        }
                                        else if (is_tmp_lead_mu) {
                                            // Boosted Flavor CR
                                            ////cout<<"ok22"<<endl;
                                            is_Boosted_Flav_MEJ = true;
                                            muon1_tight_charge = Tight_muons[0]-> TightCharge() ;
                                            FillHist(this_syst + "/Boosted_Flav_MEJ_muon_tight_charge", muon1_tight_charge , 1.0, 3, 0., 3.);
                                            bool tight_charge = Tight_muons[0]-> TightCharge() == 2;
                                            if (LeadLep->Charge() * OFLooseLepton->Charge() > 0) {
                                                is_Boosted_Flav_MEJ_SS = true;
                                                if (tight_charge) {
                                                    is_Boosted_Flav_MEJ_SS_tight = true;
                                                }
                                                else {
                                                    is_Boosted_Flav_MEJ_SS_not_tight = true;
                                                }
                                            } else {
                                                is_Boosted_Flav_MEJ_OS = true;
                                                if (tight_charge) {
                                                    is_Boosted_Flav_MEJ_OS_tight = true;
                                                }
                                                else {
                                                    is_Boosted_Flav_MEJ_OS_not_tight = true;
                                                }
                                            }
                                            Boost_FlavMEJpt = (*LeadLep + *OFLooseLepton).Pt();
                                            Boost_FlavMEJleadfatjetpt = HNFatJet.Pt();
                                            Boost_FlavMEJmlljj = WRCand.M();
                                            Boost_FlavMEJSDM = HNFatJet.SDMass();
                                            Boost_FlavMEJleadleppt = LeadLep->Pt();
                                            Boost_FlavMEJsubleadleppt = OFLooseLepton->Pt();
                                            
                                            Boost_FlavMEJleadlepeta = LeadLep->Eta();
                                            Boost_FlavMEJsubleadlepeta = OFLooseLepton->Eta();
                                            Boost_FlavMEJfatjeteta = HNFatJet.Eta();
                                            Boost_FlavMEJfatjetphi = HNFatJet.Phi();

                                            Boost_FlavMEJdeltaR_leadlep_fatjet = LeadLep->DeltaR(HNFatJet);


                                            // lsf
                                            Boost_FlavMEJleadlep_lsf = HNFatJet.LSF3();
                                            
                                            //mll
                                            Boost_FlavMEJmll = (*LeadLep + *OFLooseLepton).M();
                                            // fatjet pt 
                                            Boost_FlavMEJfatjetpt = HNFatJet.Pt();
                                            // wr pt
                                            Boost_FlavMEJWRpt = WRCand.Pt();
                                            // new variables
                                            Boost_FlavMEJmass    = (*LeadLep + *OFLooseLepton).M();
                                            Boost_FlavMEJeta     = (*LeadLep + *OFLooseLepton).Eta();
                                            Boost_FlavMEJphi     = (*LeadLep + *OFLooseLepton).Phi();
                                            Boost_FlavMEJWReta   = WRCand.Eta();
                                            Boost_FlavMEJWRphi   = WRCand.Phi();
                                            Boost_FlavMEJleadlepphi    = LeadLep->Phi();
                                            Boost_FlavMEJsubleadlepphi = OFLooseLepton->Phi();
                                            Boost_FlavMEJfatjet_lsf3   = HNFatJet.LSF3();
                                            Boost_FlavMEJsubleadlep_miniiso = OFLooseLepton->MiniPFRelIso();
                                            Boost_FlavMEJdphi_leadlep_fatjet = (float)abs(LeadLep->DeltaPhi(HNFatJet));
                                            Boost_FlavMEJpileup_num = ev.nTrueInt();
                                            Boost_FlavMEJpvgood = ev.nPVsGood();
                                            Boost_FlavMEJpv = ev.nPV();
                                            Boost_FlavMEJjet_num    = (float)selected_jets.size();
                                            float tightmuon1_org_pt = Tight_muons[0]->OriginalPt();                                        
                                            float tightmuon1_pt = Tight_muons[0]->Pt();                                        
                                            if (tightmuon1_org_pt != tightmuon1_pt) {
                                                float rel = tightmuon1_pt / tightmuon1_org_pt;
                                                FillHist(this_syst + "/Muon_TuneP_RelPt", rel, weight, 100, -1., 1.);
                                            }
                                            electron2_tight_charge = ((Electron*)OFLooseLepton)->TightCharge();
                                            FillHist(this_syst + "/Boosted_Flav_EMJ_electron1_tight_charge", electron1_tight_charge , 1.0, 5, 0., 5.);
                                            FillHist(this_syst + "/Boosted_Flav_EMJ_electron2_tight_charge", electron2_tight_charge , 1.0, 5, 0., 5.);
                                            {
                                                bool mej_m_tight = (muon1_tight_charge == 2);
                                                bool mej_e_tight = (electron2_tight_charge == 2);
                                                if (is_Boosted_Flav_MEJ_SS) {
                                                    if (mej_m_tight && mej_e_tight) is_Boosted_Flav_MEJ_SS_M_tight_E_tight = true;
                                                    else if (mej_m_tight && !mej_e_tight) is_Boosted_Flav_MEJ_SS_M_tight_E_not_tight = true;
                                                    else if (!mej_m_tight && mej_e_tight) is_Boosted_Flav_MEJ_SS_M_not_tight_E_tight = true;
                                                    else is_Boosted_Flav_MEJ_SS_M_not_tight_E_not_tight = true;
                                                } else if (is_Boosted_Flav_MEJ_OS) {
                                                    if (mej_m_tight && mej_e_tight) is_Boosted_Flav_MEJ_OS_M_tight_E_tight = true;
                                                    else if (mej_m_tight && !mej_e_tight) is_Boosted_Flav_MEJ_OS_M_tight_E_not_tight = true;
                                                    else if (!mej_m_tight && mej_e_tight) is_Boosted_Flav_MEJ_OS_M_not_tight_E_tight = true;
                                                    else is_Boosted_Flav_MEJ_OS_M_not_tight_E_not_tight = true;
                                                }
                                            }
                                        

                                        }
                                        // B    oosted Flavor CR
                                    }
                                    else {
                                            if (is_tmp_lead_el) {
                                        // Boosted low WR Flavor CR
                                        
                                        }
                                            else if (is_tmp_lead_mu) {
                                            // Boosted low WR Flavor CR
                                        
                                        }
                                    }
                                }
                            }
                        }
                    } // has no extra lepton finish 
                } // leading lepton back to back fatjet end   
            } // dont have low mll end
        }
        } // boost lead lep pt cut end
    }// boost selected event end
    // Validation hook (--userflags corrShapes): decorate every weight function so its
    // nominal scale factor is histogrammed into CorrShapes/weight_<name>/sf. Wrapping the
    // map generically covers all entries and picks up new ones automatically.
    if (FillingCorrShapes() && !IsDATA) {
        for (auto &kv : weight_function_map) {
            const TString wname = TString("weight_") + kv.first;
            if (auto *f2 = std::get_if<std::function<float(MyCorrection::variation, TString)>>(&kv.second)) {
                auto inner = *f2;
                kv.second = std::function<float(MyCorrection::variation, TString)>(
                    [this, wname, inner](MyCorrection::variation var, TString src) -> float {
                        const float sf = inner(var, src);
                        if (var == MyCorrection::variation::nom) FillCorrWeight(wname, sf);
                        return sf;
                    });
            } else if (auto *f0 = std::get_if<std::function<float()>>(&kv.second)) {
                auto inner = *f0;
                kv.second = std::function<float()>([this, wname, inner]() -> float {
                    const float sf = inner();
                    FillCorrWeight(wname, sf);
                    return sf;
                });
            }
        }
    }

    systHelper->assignWeightFunctionMap(weight_function_map);

    //if (!exist_data) return;
    {
        // (name, weight, R_resolved, R_boosted). R is carried alongside rather
        // than folded into the weight because is_Resolved_DY_* and
        // is_Boosted_DY_* are not exclusive -- one event can be filled into both
        // -- so the two categories need different factors on the same event.
        //
        // R is looked up from THIS pass's jet collections, so an object
        // systematic that moves the jet pT automatically moves where R is read.
        std::vector<std::tuple<std::string, float, float, float>> fill_targets;
        const float R_res_nom = (dycorr.apply && selected_jets.size() > 0)
            ? GetJetPtR(true, selected_jets[0].Pt()) : 1.0f;
        // The boosted lookup pT must be the variable R was DERIVED against:
        // the DY CR histograms the first fatjet with |dPhi(lead lepton, J)| >
        // 2.0 (HNWR_miniiso.cc:2224), the requirement every
        // boosted region shares -- not the plain leading fatjet. Until
        // 2026-07-29 this used fatjets[0].Pt(); when the leading fatjet sits
        // on the lepton side and a subleading one is the region's fatjet
        // (~30% of the 400-600 GeV lookup bin in the DY CR), R was read one
        // bin too high. The lead lepton here is Tight_leps[0], exactly what
        // the boosted branches use as LeadLep. Fallback: fatjets[0] -- if no
        // fatjet passes dPhi, no boosted region is filled and the value is
        // never used.
        float r_boo_pt = fatjets.size() > 0 ? fatjets[0].Pt() : -1.f;
        if (fatjets.size() > 0 && Tight_leps.size() > 0) {
            for (const auto &fj : fatjets) {
                if (std::abs(Tight_leps[0]->DeltaPhi(fj)) > 2.0) {
                    r_boo_pt = fj.Pt();
                    break;
                }
            }
        }
        const float R_boo_nom = (dycorr.apply && r_boo_pt > 0.f)
            ? GetJetPtR(false, r_boo_pt) : 1.0f;

        std::unordered_map<std::string, float> weight_map;
        if (!IsDATA) {
            weight_map = systHelper->calculateWeight();
            for (const auto& [sn, sf_val] : weight_map) {
                fill_targets.push_back({sn, weight * sf_val, R_res_nom, R_boo_nom});
            }
        } else {
            fill_targets.push_back({std::string(this_syst), 1.0f, 1.0f, 1.0f});
        }

        // Per-bin DYReshape nuisances. Only in the Central pass: they are a
        // property of R, not of the object variations, and crossing them with
        // every JES/JER pass would multiply the output for no gain. Each moves
        // one bin by its sigma_R -- statistics and the non-DY cross sections
        // already folded into a single band, following the Run 2 AN treatment --
        // and leaves the other bins nominal.
        if (!IsDATA && dycorr.apply && this_syst == "Central") {
            const float w_central = weight * weight_map["Central"];
            // nuis_bin is the index on the rebinned R curve, whose bin 0 is
            // the undefined below-cut bin (NaN, GetJetPtR returns before the
            // variation). BinK therefore targets curve index K = b + 1.
            // Passing b (as until 2026-07-29) made Bin1 a no-op variation and
            // left the last R bin with no variation at all.
            for (int b = 0; b < dycorr.n_nuis_res; b++) {
                for (int d = -1; d <= 1; d += 2) {
                    const TString nm = TString::Format("ResolvedDYReshapeBin%d_%s",
                                                       b + 1, d > 0 ? "Up" : "Down");
                    fill_targets.push_back({std::string(nm), w_central,
                        selected_jets.size() > 0
                            ? GetJetPtR(true, selected_jets[0].Pt(), b + 1, d) : 1.0f,
                        R_boo_nom});
                }
            }
            for (int b = 0; b < dycorr.n_nuis_boo; b++) {
                for (int d = -1; d <= 1; d += 2) {
                    const TString nm = TString::Format("BoostedDYReshapeBin%d_%s",
                                                       b + 1, d > 0 ? "Up" : "Down");
                    fill_targets.push_back({std::string(nm), w_central, R_res_nom,
                        r_boo_pt > 0.f
                            ? GetJetPtR(false, r_boo_pt, b + 1, d) : 1.0f});
                }
            }
        }

        for (const auto& [syst_name, final_weight_noR, R_res, R_boo] : fill_targets) {
        // RunXsecSyst writes only the PDF member histograms (filled after this loop).
        // Skipping the ordinary fills takes the output from 279 MB to a few tens of MB per
        // sample, which is what makes running the flag over every signal alias practical.
        if (RunXsecSyst) break;
        //resolved
        //DY CR
        {
            // R multiplies the fill weight, not `weight`: an event can be in
            // both a resolved and a boosted region, and they take different
            // factors. Non-DY and data have R = 1.
            const float final_weight = final_weight_noR * R_res;
            auto fill = [&](const char* pfx) {
                FillHist(syst_name + "/" + pfx + "_pt", Resolve_DYCREEpt, final_weight, 100, 0., 1000.);
                FillHist(syst_name + "/" + pfx + "_leading_jet_pt", Resolve_DYCREEleadjetpt, final_weight, 200, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_subleading_jet_pt", Resolve_DYCREEsubleadjetpt, final_weight, 200, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_mlljj", Resolve_DYCREEmlljj, final_weight, 800, 0., 8000.);
                FillHist(syst_name + "/" + pfx + "_leading_lep_pt", Resolve_DYCREEleadleppt, final_weight, 2000, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_subleading_lep_pt", Resolve_DYCREEsubleadleppt, final_weight, 2000, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_mass", Resolve_DYCREEmass, final_weight, 1000, 0., 1000.);
                FillHist(syst_name + "/" + pfx + "_eta", Resolve_DYCREEeta, final_weight, 100, -5., 5.);
                FillHist(syst_name + "/" + pfx + "_phi", Resolve_DYCREEphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_leading_jet_eta", Resolve_DYCREEleadjeteta, final_weight, 100, -2.5, 2.5);
                FillHist(syst_name + "/" + pfx + "_leading_jet_phi", Resolve_DYCREEleadjetphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_subleading_jet_eta", Resolve_DYCREEsubleadjeteta, final_weight, 100, -2.5, 2.5);
                FillHist(syst_name + "/" + pfx + "_subleading_jet_phi", Resolve_DYCREEsubleadjetphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_j1j2_mass", Resolve_DYCREEj1j2mass, final_weight, 100, 0., 1000.);
                FillHist(syst_name + "/" + pfx + "_j1j2_pt", Resolve_DYCREEj1j2pt, final_weight, 100, 0., 1000.);
                FillHist(syst_name + "/" + pfx + "_j1j2_eta", Resolve_DYCREEj1j2eta, final_weight, 100, -5., 5.);
                FillHist(syst_name + "/" + pfx + "_j1j2_phi", Resolve_DYCREEj1j2phi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_l1j1j2_mass", Resolve_DYCREEl1j1j2mass, final_weight, 200, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_l1j1j2_pt", Resolve_DYCREEl1j1j2pt, final_weight, 200, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_l1j1j2_eta", Resolve_DYCREEl1j1j2eta, final_weight, 100, -5., 5.);
                FillHist(syst_name + "/" + pfx + "_l1j1j2_phi", Resolve_DYCREEl1j1j2phi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_l2j1j2_mass", Resolve_DYCREEl2j1j2mass, final_weight, 200, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_l2j1j2_pt", Resolve_DYCREEl2j1j2pt, final_weight, 200, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_l2j1j2_eta", Resolve_DYCREEl2j1j2eta, final_weight, 100, -5., 5.);
                FillHist(syst_name + "/" + pfx + "_l2j1j2_phi", Resolve_DYCREEl2j1j2phi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_mlljj_pt", Resolve_DYCREEmlljjpt, final_weight, 200, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_mlljj_eta", Resolve_DYCREEmlljjeta, final_weight, 100, -5., 5.);
                FillHist(syst_name + "/" + pfx + "_mlljj_phi", Resolve_DYCREEmlljjphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_leading_lep_eta", Resolve_DYCREEleadlepeta, final_weight, 100, -2.5, 2.5);
                FillHist(syst_name + "/" + pfx + "_leading_lep_phi", Resolve_DYCREEleadlepphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_subleading_lep_eta", Resolve_DYCREEsubleadlepeta, final_weight, 100, -2.5, 2.5);
                FillHist(syst_name + "/" + pfx + "_subleading_lep_phi", Resolve_DYCREEsubleadlepphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_jetnum", Resolve_DYCREEjetnum, final_weight, 20, 0., 20.);
                FillHist(syst_name + "/" + pfx + "_punum", Resolve_DYCREEpunum, final_weight, 80, 0., 80.);
                FillHist(syst_name + "/" + pfx + "_pv", Resolve_DYCREEpv, final_weight, 80, 0., 80.);
                FillHist(syst_name + "/" + pfx + "_pvgood", Resolve_DYCREEpvgood, final_weight, 80, 0., 80.);
            };
            for (auto [cond, pfx] : std::initializer_list<std::pair<bool,const char*>>{
                {is_Resolved_DY_EE,             "DYCR_Resolved_EE"},
                {is_Resolved_DY_EE_OS,          "DYCR_Resolved_EE_OS"},
                {is_Resolved_DY_EE_OS_2e_tight, "DYCR_Resolved_EE_OS_2e_tight"},
                {is_Resolved_DY_EE_OS_1e_tight, "DYCR_Resolved_EE_OS_1e_tight"},
                {is_Resolved_DY_EE_OS_0e_tight, "DYCR_Resolved_EE_OS_0e_tight"},
                {is_Resolved_DY_EE_SS,          "DYCR_Resolved_EE_SS"},
                {is_Resolved_DY_EE_SS_2e_tight, "DYCR_Resolved_EE_SS_2e_tight"},
                {is_Resolved_DY_EE_SS_1e_tight, "DYCR_Resolved_EE_SS_1e_tight"},
                {is_Resolved_DY_EE_SS_0e_tight, "DYCR_Resolved_EE_SS_0e_tight"},
                // mll-split DY CRs: CR1 = 60-100, CR2 = 100-150
                {is_Resolved_DY_EE_CR1,                          "DYCR1_Resolved_EE"},
                {is_Resolved_DY_EE_CR2,                          "DYCR2_Resolved_EE"},
            }) { if (cond) fill(pfx); }
        }
        {
            // R multiplies the fill weight, not `weight`: an event can be in
            // both a resolved and a boosted region, and they take different
            // factors. Non-DY and data have R = 1.
            const float final_weight = final_weight_noR * R_res;
            auto fill = [&](const char* pfx) {
                FillHist(syst_name + "/" + pfx + "_pt", Resolve_DYCRMMpt, final_weight, 100, 0., 1000.);
                FillHist(syst_name + "/" + pfx + "_leading_jet_pt", Resolve_DYCRMMleadjetpt, final_weight, 200, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_subleading_jet_pt", Resolve_DYCRMMsubleadjetpt, final_weight, 200, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_mlljj", Resolve_DYCRMMmlljj, final_weight, 800, 0., 8000.);
                FillHist(syst_name + "/" + pfx + "_leading_lep_pt", Resolve_DYCRMMleadleppt, final_weight, 2000, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_subleading_lep_pt", Resolve_DYCRMMsubleadleppt, final_weight, 2000, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_mass", Resolve_DYCRMMmass, final_weight, 1000, 0., 1000.);
                FillHist(syst_name + "/" + pfx + "_eta", Resolve_DYCRMMeta, final_weight, 100, -5., 5.);
                FillHist(syst_name + "/" + pfx + "_phi", Resolve_DYCRMMphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_leading_jet_eta", Resolve_DYCRMMleadjeteta, final_weight, 100, -2.5, 2.5);
                FillHist(syst_name + "/" + pfx + "_leading_jet_phi", Resolve_DYCRMMleadjetphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_subleading_jet_eta", Resolve_DYCRMMsubleadjeteta, final_weight, 100, -2.5, 2.5);
                FillHist(syst_name + "/" + pfx + "_subleading_jet_phi", Resolve_DYCRMMsubleadjetphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_j1j2_mass", Resolve_DYCRMMj1j2mass, final_weight, 100, 0., 1000.);
                FillHist(syst_name + "/" + pfx + "_j1j2_pt", Resolve_DYCRMMj1j2pt, final_weight, 100, 0., 1000.);
                FillHist(syst_name + "/" + pfx + "_j1j2_eta", Resolve_DYCRMMj1j2eta, final_weight, 100, -5., 5.);
                FillHist(syst_name + "/" + pfx + "_j1j2_phi", Resolve_DYCRMMj1j2phi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_l1j1j2_mass", Resolve_DYCRMMl1j1j2mass, final_weight, 200, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_l1j1j2_pt", Resolve_DYCRMMl1j1j2pt, final_weight, 200, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_l1j1j2_eta", Resolve_DYCRMMl1j1j2eta, final_weight, 100, -5., 5.);
                FillHist(syst_name + "/" + pfx + "_l1j1j2_phi", Resolve_DYCRMMl1j1j2phi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_l2j1j2_mass", Resolve_DYCRMMl2j1j2mass, final_weight, 200, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_l2j1j2_pt", Resolve_DYCRMMl2j1j2pt, final_weight, 200, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_l2j1j2_eta", Resolve_DYCRMMl2j1j2eta, final_weight, 100, -5., 5.);
                FillHist(syst_name + "/" + pfx + "_l2j1j2_phi", Resolve_DYCRMMl2j1j2phi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_mlljj_pt", Resolve_DYCRMMmlljjpt, final_weight, 200, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_mlljj_eta", Resolve_DYCRMMmlljjeta, final_weight, 100, -5., 5.);
                FillHist(syst_name + "/" + pfx + "_mlljj_phi", Resolve_DYCRMMmlljjphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_leading_lep_eta", Resolve_DYCRMMleadlepeta, final_weight, 100, -2.5, 2.5);
                FillHist(syst_name + "/" + pfx + "_leading_lep_phi", Resolve_DYCRMMleadlepphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_subleading_lep_eta", Resolve_DYCRMMsubleadlepeta, final_weight, 100, -2.5, 2.5);
                FillHist(syst_name + "/" + pfx + "_subleading_lep_phi", Resolve_DYCRMMsubleadlepphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_jetnum", Resolve_DYCRMMjetnum, final_weight, 20, 0., 20.);
                FillHist(syst_name + "/" + pfx + "_punum", Resolve_DYCRMMpunum, final_weight, 80, 0., 80.);
                FillHist(syst_name + "/" + pfx + "_pv", Resolve_DYCRMMpv, final_weight, 80, 0., 80.);
                FillHist(syst_name + "/" + pfx + "_pvgood", Resolve_DYCRMMpvgood, final_weight, 80, 0., 80.);
            };
            for (auto [cond, pfx] : std::initializer_list<std::pair<bool,const char*>>{
                {is_Resolved_DY_MM,              "DYCR_Resolved_MM"},
                {is_Resolved_DY_MM_OS,           "DYCR_Resolved_MM_OS"},
                {is_Resolved_DY_MM_OS_tight,     "DYCR_Resolved_MM_OS_tight"},
                {is_Resolved_DY_MM_OS_not_tight, "DYCR_Resolved_MM_OS_not_tight"},
                {is_Resolved_DY_MM_SS,           "DYCR_Resolved_MM_SS"},
                {is_Resolved_DY_MM_SS_tight,     "DYCR_Resolved_MM_SS_tight"},
                {is_Resolved_DY_MM_SS_not_tight, "DYCR_Resolved_MM_SS_not_tight"},
                // mll-split DY CRs: CR1 = 60-100, CR2 = 100-150
                {is_Resolved_DY_MM_CR1,                          "DYCR1_Resolved_MM"},
                {is_Resolved_DY_MM_CR2,                          "DYCR2_Resolved_MM"},
            }) { if (cond) fill(pfx); }
        }
        //SR
        {
            // R multiplies the fill weight, not `weight`: an event can be in
            // both a resolved and a boosted region, and they take different
            // factors. Non-DY and data have R = 1.
            const float final_weight = final_weight_noR * R_res;
            auto fill = [&](const char* pfx) {
                FillHist(syst_name + "/" + pfx + "_ll_pt", Resolve_SREEpt, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/" + pfx + "_leading_jet_pt", Resolve_SREEleadjetpt, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/" + pfx + "_subleading_jet_pt", Resolve_SREEsubleadjetpt, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/" + pfx + "_mlljj", Resolve_SREEmlljj, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/" + pfx + "_leading_lep_pt", Resolve_SREEleadleppt, final_weight, 2000, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_subleading_lep_pt", Resolve_SREEsubleadleppt, final_weight, 2000, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_mass", Resolve_SREEmass, final_weight, 2000, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_eta", Resolve_SREEeta, final_weight, 100, -5., 5.);
                FillHist(syst_name + "/" + pfx + "_phi", Resolve_SREEphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_leading_jet_eta", Resolve_SREEleadjeteta, final_weight, 100, -2.5, 2.5);
                FillHist(syst_name + "/" + pfx + "_leading_jet_phi", Resolve_SREEleadjetphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_subleading_jet_eta", Resolve_SREEsubleadjeteta, final_weight, 100, -2.5, 2.5);
                FillHist(syst_name + "/" + pfx + "_subleading_jet_phi", Resolve_SREEsubleadjetphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_j1j2_mass", Resolve_SREEj1j2mass, final_weight, 100, 0., 1000.);
                FillHist(syst_name + "/" + pfx + "_j1j2_pt", Resolve_SREEj1j2pt, final_weight, 100, 0., 1000.);
                FillHist(syst_name + "/" + pfx + "_j1j2_eta", Resolve_SREEj1j2eta, final_weight, 100, -5., 5.);
                FillHist(syst_name + "/" + pfx + "_j1j2_phi", Resolve_SREEj1j2phi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_l1j1j2_mass", Resolve_SREEl1j1j2mass, final_weight, 800, 0., 8000.);
                FillHist(syst_name + "/" + pfx + "_l1j1j2_pt", Resolve_SREEl1j1j2pt, final_weight, 200, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_l1j1j2_eta", Resolve_SREEl1j1j2eta, final_weight, 100, -5., 5.);
                FillHist(syst_name + "/" + pfx + "_l1j1j2_phi", Resolve_SREEl1j1j2phi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_l2j1j2_mass", Resolve_SREEl2j1j2mass, final_weight, 800, 0., 8000.);
                FillHist(syst_name + "/" + pfx + "_l2j1j2_pt", Resolve_SREEl2j1j2pt, final_weight, 200, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_l2j1j2_eta", Resolve_SREEl2j1j2eta, final_weight, 100, -5., 5.);
                FillHist(syst_name + "/" + pfx + "_l2j1j2_phi", Resolve_SREEl2j1j2phi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_mlljj_pt", Resolve_SREEmlljjpt, final_weight, 800, 0., 8000.);
                FillHist(syst_name + "/" + pfx + "_mlljj_eta", Resolve_SREEmlljjeta, final_weight, 100, -5., 5.);
                FillHist(syst_name + "/" + pfx + "_mlljj_phi", Resolve_SREEmlljjphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_leading_lep_eta", Resolve_SREEleadlepeta, final_weight, 100, -2.5, 2.5);
                FillHist(syst_name + "/" + pfx + "_leading_lep_phi", Resolve_SREEleadlepphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_subleading_lep_eta", Resolve_SREEsubleadlepeta, final_weight, 100, -2.5, 2.5);
                FillHist(syst_name + "/" + pfx + "_subleading_lep_phi", Resolve_SREEsubleadlepphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_jetnum", Resolve_SREEjetnum, final_weight, 20, 0., 20.);
                FillHist(syst_name + "/" + pfx + "_punum", Resolve_SREEpunum, final_weight, 80, 0., 80.);
                //FillHist(syst_name + "/" + pfx + "_pv", Resolve_SREEpv, final_weight, 80, 0., 80.);
                FillHist(syst_name + "/" + pfx + "_pvgood", Resolve_SREEpvgood, final_weight, 80, 0., 80.);
            };
            for (auto [cond, pfx] : std::initializer_list<std::pair<bool,const char*>>{
                {is_Resolved_SR_EE,             "SR_Resolved_EE"},
                {is_Resolved_SR_EE_SS,          "SR_Resolved_EE_SS"},
                {is_Resolved_SR_EE_SS_2e_tight, "SR_Resolved_EE_SS_2e_tight"},
                {is_Resolved_SR_EE_SS_1e_tight, "SR_Resolved_EE_SS_1e_tight"},
                {is_Resolved_SR_EE_SS_0e_tight, "SR_Resolved_EE_SS_0e_tight"},
                {is_Resolved_SR_EE_OS,          "SR_Resolved_EE_OS"},
                {is_Resolved_SR_EE_OS_2e_tight, "SR_Resolved_EE_OS_2e_tight"},
                {is_Resolved_SR_EE_OS_1e_tight, "SR_Resolved_EE_OS_1e_tight"},
                {is_Resolved_SR_EE_OS_0e_tight, "SR_Resolved_EE_OS_0e_tight"},
            }) { if (cond) fill(pfx); }
        }
        {
            // R multiplies the fill weight, not `weight`: an event can be in
            // both a resolved and a boosted region, and they take different
            // factors. Non-DY and data have R = 1.
            const float final_weight = final_weight_noR * R_res;
            auto fill = [&](const char* pfx) {
                FillHist(syst_name + "/" + pfx + "_ll_pt", Resolve_SRMMpt, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/" + pfx + "_leading_jet_pt", Resolve_SRMMleadjetpt, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/" + pfx + "_subleading_jet_pt", Resolve_SRMMsubleadjetpt, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/" + pfx + "_mlljj", Resolve_SRMMmlljj, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/" + pfx + "_leading_lep_pt", Resolve_SRMMleadleppt, final_weight, 2000, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_subleading_lep_pt", Resolve_SRMMsubleadleppt, final_weight, 2000, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_mass", Resolve_SRMMmass, final_weight, 2000, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_eta", Resolve_SRMMeta, final_weight, 100, -5., 5.);
                FillHist(syst_name + "/" + pfx + "_phi", Resolve_SRMMphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_leading_jet_eta", Resolve_SRMMleadjeteta, final_weight, 100, -2.5, 2.5);
                FillHist(syst_name + "/" + pfx + "_leading_jet_phi", Resolve_SRMMleadjetphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_subleading_jet_eta", Resolve_SRMMsubleadjeteta, final_weight, 100, -2.5, 2.5);
                FillHist(syst_name + "/" + pfx + "_subleading_jet_phi", Resolve_SRMMsubleadjetphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_j1j2_mass", Resolve_SRMMj1j2mass, final_weight, 100, 0., 1000.);
                FillHist(syst_name + "/" + pfx + "_j1j2_pt", Resolve_SRMMj1j2pt, final_weight, 100, 0., 1000.);
                FillHist(syst_name + "/" + pfx + "_j1j2_eta", Resolve_SRMMj1j2eta, final_weight, 100, -5., 5.);
                FillHist(syst_name + "/" + pfx + "_j1j2_phi", Resolve_SRMMj1j2phi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_l1j1j2_mass", Resolve_SRMMl1j1j2mass, final_weight, 800, 0., 8000.);
                FillHist(syst_name + "/" + pfx + "_l1j1j2_pt", Resolve_SRMMl1j1j2pt, final_weight, 200, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_l1j1j2_eta", Resolve_SRMMl1j1j2eta, final_weight, 100, -5., 5.);
                FillHist(syst_name + "/" + pfx + "_l1j1j2_phi", Resolve_SRMMl1j1j2phi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_l2j1j2_mass", Resolve_SRMMl2j1j2mass, final_weight, 800, 0., 8000.);
                FillHist(syst_name + "/" + pfx + "_l2j1j2_pt", Resolve_SRMMl2j1j2pt, final_weight, 200, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_l2j1j2_eta", Resolve_SRMMl2j1j2eta, final_weight, 100, -5., 5.);
                FillHist(syst_name + "/" + pfx + "_l2j1j2_phi", Resolve_SRMMl2j1j2phi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_mlljj_pt", Resolve_SRMMmlljjpt, final_weight, 800, 0., 8000.);
                FillHist(syst_name + "/" + pfx + "_mlljj_eta", Resolve_SRMMmlljjeta, final_weight, 100, -5., 5.);
                FillHist(syst_name + "/" + pfx + "_mlljj_phi", Resolve_SRMMmlljjphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_leading_lep_eta", Resolve_SRMMleadlepeta, final_weight, 100, -2.5, 2.5);
                FillHist(syst_name + "/" + pfx + "_leading_lep_phi", Resolve_SRMMleadlepphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_subleading_lep_eta", Resolve_SRMMsubleadlepeta, final_weight, 100, -2.5, 2.5);
                FillHist(syst_name + "/" + pfx + "_subleading_lep_phi", Resolve_SRMMsubleadlepphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_jetnum", Resolve_SRMMjetnum, final_weight, 20, 0., 20.);
                FillHist(syst_name + "/" + pfx + "_punum", Resolve_SRMMpunum, final_weight, 80, 0., 80.);
                //FillHist(syst_name + "/" + pfx + "_pv", Resolve_SRMMpv, final_weight, 80, 0., 80.);
                FillHist(syst_name + "/" + pfx + "_pvgood", Resolve_SRMMpvgood, final_weight, 80, 0., 80.);
            };
            for (auto [cond, pfx] : std::initializer_list<std::pair<bool,const char*>>{
                {is_Resolved_SR_MM,              "SR_Resolved_MM"},
                {is_Resolved_SR_MM_SS,           "SR_Resolved_MM_SS"},
                {is_Resolved_SR_MM_SS_tight,     "SR_Resolved_MM_SS_tight"},
                {is_Resolved_SR_MM_SS_not_tight, "SR_Resolved_MM_SS_not_tight"},
                {is_Resolved_SR_MM_OS,           "SR_Resolved_MM_OS"},
                {is_Resolved_SR_MM_OS_tight,     "SR_Resolved_MM_OS_tight"},
                {is_Resolved_SR_MM_OS_not_tight, "SR_Resolved_MM_OS_not_tight"},
            }) { if (cond) fill(pfx); }
        }

        //Flav
        {
            // R multiplies the fill weight, not `weight`: an event can be in
            // both a resolved and a boosted region, and they take different
            // factors. Non-DY and data have R = 1.
            const float final_weight = final_weight_noR * R_res;
            auto fill = [&](const char* pfx) {
                FillHist(syst_name + "/" + pfx + "_ll_pt", Resolve_FlavCRpt, final_weight, 1000, 0., 1000.);
                FillHist(syst_name + "/" + pfx + "_leading_jet_pt", Resolve_FlavCRleadjetpt, final_weight, 2000, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_subleading_jet_pt", Resolve_FlavCRsubleadjetpt, final_weight, 2000, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_mlljj", Resolve_FlavCRmlljj, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/" + pfx + "_leading_lep_pt", Resolve_FlavCRleadleppt, final_weight, 2000, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_subleading_lep_pt", Resolve_FlavCRsubleadleppt, final_weight, 2000, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_mass", Resolve_FlavCRmass, final_weight, 2000, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_eta", Resolve_FlavCReta, final_weight, 100, -5., 5.);
                FillHist(syst_name + "/" + pfx + "_phi", Resolve_FlavCRphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_leading_jet_eta", Resolve_FlavCRleadjeteta, final_weight, 100, -2.5, 2.5);
                FillHist(syst_name + "/" + pfx + "_leading_jet_phi", Resolve_FlavCRleadjetphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_subleading_jet_eta", Resolve_FlavCRsubleadjeteta, final_weight, 100, -2.5, 2.5);
                FillHist(syst_name + "/" + pfx + "_subleading_jet_phi", Resolve_FlavCRsubleadjetphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_j1j2_mass", Resolve_FlavCRj1j2mass, final_weight, 100, 0., 1000.);
                FillHist(syst_name + "/" + pfx + "_j1j2_pt", Resolve_FlavCRj1j2pt, final_weight, 100, 0., 1000.);
                FillHist(syst_name + "/" + pfx + "_j1j2_eta", Resolve_FlavCRj1j2eta, final_weight, 100, -5., 5.);
                FillHist(syst_name + "/" + pfx + "_j1j2_phi", Resolve_FlavCRj1j2phi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_l1j1j2_mass", Resolve_FlavCRl1j1j2mass, final_weight, 800, 0., 8000.);
                FillHist(syst_name + "/" + pfx + "_l1j1j2_pt", Resolve_FlavCRl1j1j2pt, final_weight, 200, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_l1j1j2_eta", Resolve_FlavCRl1j1j2eta, final_weight, 100, -5., 5.);
                FillHist(syst_name + "/" + pfx + "_l1j1j2_phi", Resolve_FlavCRl1j1j2phi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_l2j1j2_mass", Resolve_FlavCRl2j1j2mass, final_weight, 800, 0., 8000.);
                FillHist(syst_name + "/" + pfx + "_l2j1j2_pt", Resolve_FlavCRl2j1j2pt, final_weight, 200, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_l2j1j2_eta", Resolve_FlavCRl2j1j2eta, final_weight, 100, -5., 5.);
                FillHist(syst_name + "/" + pfx + "_l2j1j2_phi", Resolve_FlavCRl2j1j2phi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_mlljj_pt", Resolve_FlavCRmlljjpt, final_weight, 800, 0., 8000.);
                FillHist(syst_name + "/" + pfx + "_mlljj_eta", Resolve_FlavCRmlljjeta, final_weight, 100, -5., 5.);
                FillHist(syst_name + "/" + pfx + "_mlljj_phi", Resolve_FlavCRmlljjphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_leading_lep_eta", Resolve_FlavCRleadlepeta, final_weight, 100, -2.5, 2.5);
                FillHist(syst_name + "/" + pfx + "_leading_lep_phi", Resolve_FlavCRleadlepphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_subleading_lep_eta", Resolve_FlavCRsubleadlepeta, final_weight, 100, -2.5, 2.5);
                FillHist(syst_name + "/" + pfx + "_subleading_lep_phi", Resolve_FlavCRsubleadlepphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_jetnum", Resolve_FlavCRjetnum, final_weight, 20, 0., 20.);
                FillHist(syst_name + "/" + pfx + "_punum", Resolve_FlavCRpunum, final_weight, 80, 0., 80.);
                FillHist(syst_name + "/" + pfx + "_pv", Resolve_FlavCRpv, final_weight, 80, 0., 80.);
                FillHist(syst_name + "/" + pfx + "_pvgood", Resolve_FlavCRpvgood, final_weight, 80, 0., 80.);
            };
            for (auto [cond, pfx] : std::initializer_list<std::pair<bool,const char*>>{
                {is_Resolved_Flav_EM,                            "FlavCR_Resolved_EM"},
                {is_Resolved_Flav_EM_OS,                         "FlavCR_Resolved_EM_OS"},
                {is_Resolved_Flav_EM_OS_tight,                   "FlavCR_Resolved_EM_OS_tight"},
                {is_Resolved_Flav_EM_OS_not_tight,               "FlavCR_Resolved_EM_OS_not_tight"},
                {is_Resolved_Flav_EM_OS_M_tight_E_tight,         "FlavCR_Resolved_EM_OS_M_tight_E_tight"},
                {is_Resolved_Flav_EM_OS_M_tight_E_not_tight,     "FlavCR_Resolved_EM_OS_M_tight_E_not_tight"},
                {is_Resolved_Flav_EM_OS_M_not_tight_E_tight,     "FlavCR_Resolved_EM_OS_M_not_tight_E_tight"},
                {is_Resolved_Flav_EM_OS_M_not_tight_E_not_tight, "FlavCR_Resolved_EM_OS_M_not_tight_E_not_tight"},
                {is_Resolved_Flav_EM_SS,                         "FlavCR_Resolved_EM_SS"},
                {is_Resolved_Flav_EM_SS_tight,                   "FlavCR_Resolved_EM_SS_tight"},
                {is_Resolved_Flav_EM_SS_not_tight,               "FlavCR_Resolved_EM_SS_not_tight"},
                {is_Resolved_Flav_EM_SS_M_tight_E_tight,         "FlavCR_Resolved_EM_SS_M_tight_E_tight"},
                {is_Resolved_Flav_EM_SS_M_tight_E_not_tight,     "FlavCR_Resolved_EM_SS_M_tight_E_not_tight"},
                {is_Resolved_Flav_EM_SS_M_not_tight_E_tight,     "FlavCR_Resolved_EM_SS_M_not_tight_E_tight"},
                {is_Resolved_Flav_EM_SS_M_not_tight_E_not_tight, "FlavCR_Resolved_EM_SS_M_not_tight_E_not_tight"},
            }) { if (cond) fill(pfx); }
        }
        //Boosted
        
        //DY CR
        //Boosted
        //DY CR EE
        {
            // R multiplies the fill weight, not `weight`: an event can be in
            // both a resolved and a boosted region, and they take different
            // factors. Non-DY and data have R = 1.
            const float final_weight = final_weight_noR * R_boo;
            auto fill = [&](const char* pfx) {
                FillHist(syst_name + "/" + pfx + "_ll_pt", Boost_DYCREEpt, final_weight, 1000, 0., 1000.);
                FillHist(syst_name + "/" + pfx + "_leading_fatjet_pt", Boost_DYCREEfatjetpt, final_weight, 2000, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_mlljj", Boost_DYCREEmlljj, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/" + pfx + "_leading_lep_pt", Boost_DYCREEleadleppt, final_weight, 2000, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_subleading_lep_pt", Boost_DYCREEsubleadleppt, final_weight, 2000, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_mass", Boost_DYCREEmass, final_weight, 1000, 0., 1000.);
                FillHist(syst_name + "/" + pfx + "_eta", Boost_DYCREEeta, final_weight, 100, -5., 5.);
                FillHist(syst_name + "/" + pfx + "_phi", Boost_DYCREEphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_fatjet_eta", Boost_DYCREEfatjeteta, final_weight, 100, -2.5, 2.5);
                FillHist(syst_name + "/" + pfx + "_fatjet_phi", Boost_DYCREEfatjetphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_fatjet_SDM", Boost_DYCREEfatjetSDM, final_weight, 100, 0., 200.);
                FillHist(syst_name + "/" + pfx + "_mll", Boost_DYCREEmll, final_weight, 1000, 0., 1000.);
                FillHist(syst_name + "/" + pfx + "_WR_pt", Boost_DYCREEWRpt, final_weight, 200, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_WR_eta", Boost_DYCREEWReta, final_weight, 100, -5., 5.);
                FillHist(syst_name + "/" + pfx + "_WR_phi", Boost_DYCREEWRphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_leading_lep_eta", Boost_DYCREEleadlepeta, final_weight, 100, -2.5, 2.5);
                FillHist(syst_name + "/" + pfx + "_leading_lep_phi", Boost_DYCREEleadlepphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_subleading_lep_eta", Boost_DYCREEsubleadlepeta, final_weight, 100, -2.5, 2.5);
                FillHist(syst_name + "/" + pfx + "_subleading_lep_phi", Boost_DYCREEsubleadlepphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_fatjet_lsf3", Boost_DYCREEfatjet_lsf3, final_weight, 100, 0., 1.);
                FillHist(syst_name + "/" + pfx + "_subleadlep_miniiso", Boost_DYCREEsubleadlep_miniiso, final_weight, 200, 0., 1.);
                FillHist(syst_name + "/" + pfx + "_deltaR_leadlep_fatjet", Boost_DYCREEdeltaR_leadlep_fatjet, final_weight, 100, 0., 5.);
                FillHist(syst_name + "/" + pfx + "_dphi_leadlep_fatjet", Boost_DYCREEdphi_leadlep_fatjet, final_weight, 100, 0., 3.14);
                FillHist(syst_name + "/" + pfx + "_punum", Boost_DYCREEpileup_num, final_weight, 80, 0., 80.);
                FillHist(syst_name + "/" + pfx + "_jetnum", Boost_DYCREEjet_num, final_weight, 20, 0., 20.);
                FillHist(syst_name + "/" + pfx + "_pv", Boost_DYCREEpv, final_weight, 80, 0., 80.);
                FillHist(syst_name + "/" + pfx + "_pvgood", Boost_DYCREEpvgood, final_weight, 80, 0., 80.);
            };
            for (auto [cond, pfx] : std::initializer_list<std::pair<bool,const char*>>{
                {is_Boosted_DY_EE,             "DYCR_Boosted_EE"},
                {is_Boosted_DY_EE_OS,          "DYCR_Boosted_EE_OS"},
                {is_Boosted_DY_EE_OS_2e_tight, "DYCR_Boosted_EE_OS_2e_tight"},
                {is_Boosted_DY_EE_OS_1e_tight, "DYCR_Boosted_EE_OS_1e_tight"},
                {is_Boosted_DY_EE_OS_0e_tight, "DYCR_Boosted_EE_OS_0e_tight"},
                {is_Boosted_DY_EE_SS,          "DYCR_Boosted_EE_SS"},
                {is_Boosted_DY_EE_SS_2e_tight, "DYCR_Boosted_EE_SS_2e_tight"},
                {is_Boosted_DY_EE_SS_1e_tight, "DYCR_Boosted_EE_SS_1e_tight"},
                {is_Boosted_DY_EE_SS_0e_tight, "DYCR_Boosted_EE_SS_0e_tight"},
                // mll-split DY CRs: CR1 = 60-100, CR2 = 100-150
                {is_Boosted_DY_EE_CR1,                         "DYCR1_Boosted_EE"},
                {is_Boosted_DY_EE_CR2,                         "DYCR2_Boosted_EE"},
            }) { if (cond) fill(pfx); }
        }
        //DY CR MM
        {
            // R multiplies the fill weight, not `weight`: an event can be in
            // both a resolved and a boosted region, and they take different
            // factors. Non-DY and data have R = 1.
            const float final_weight = final_weight_noR * R_boo;
            auto fill = [&](const char* pfx) {
                FillHist(syst_name + "/" + pfx + "_ll_pt", Boost_DYCRMMpt, final_weight, 1000, 0., 1000.);
                FillHist(syst_name + "/" + pfx + "_leading_fatjet_pt", Boost_DYCRMMfatjetpt, final_weight, 2000, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_mlljj", Boost_DYCRMMmlljj, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/" + pfx + "_leading_lep_pt", Boost_DYCRMMleadleppt, final_weight, 2000, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_subleading_lep_pt", Boost_DYCRMMsubleadleppt, final_weight, 2000, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_mass", Boost_DYCRMMmass, final_weight, 1000, 0., 1000.);
                FillHist(syst_name + "/" + pfx + "_eta", Boost_DYCRMMeta, final_weight, 100, -5., 5.);
                FillHist(syst_name + "/" + pfx + "_phi", Boost_DYCRMMphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_fatjet_eta", Boost_DYCRMMfatjeteta, final_weight, 100, -2.5, 2.5);
                FillHist(syst_name + "/" + pfx + "_fatjet_phi", Boost_DYCRMMfatjetphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_fatjet_SDM", Boost_DYCRMMfatjetSDM, final_weight, 100, 0., 200.);
                FillHist(syst_name + "/" + pfx + "_mll", Boost_DYCRMMmll, final_weight, 1000, 0., 1000.);
                FillHist(syst_name + "/" + pfx + "_WR_pt", Boost_DYCRMMWRpt, final_weight, 200, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_WR_eta", Boost_DYCRMMWReta, final_weight, 100, -5., 5.);
                FillHist(syst_name + "/" + pfx + "_WR_phi", Boost_DYCRMMWRphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_leading_lep_eta", Boost_DYCRMMleadlepeta, final_weight, 100, -2.5, 2.5);
                FillHist(syst_name + "/" + pfx + "_leading_lep_phi", Boost_DYCRMMleadlepphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_subleading_lep_eta", Boost_DYCRMMsubleadlepeta, final_weight, 100, -2.5, 2.5);
                FillHist(syst_name + "/" + pfx + "_subleading_lep_phi", Boost_DYCRMMsubleadlepphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_fatjet_lsf3", Boost_DYCRMMfatjet_lsf3, final_weight, 100, 0., 1.);
                FillHist(syst_name + "/" + pfx + "_subleadlep_miniiso", Boost_DYCRMMsubleadlep_miniiso, final_weight, 200, 0., 1.);
                FillHist(syst_name + "/" + pfx + "_deltaR_leadlep_fatjet", Boost_DYCRMMdeltaR_leadlep_fatjet, final_weight, 100, 0., 5.);
                FillHist(syst_name + "/" + pfx + "_dphi_leadlep_fatjet", Boost_DYCRMMdphi_leadlep_fatjet, final_weight, 100, 0., 3.14);
                FillHist(syst_name + "/" + pfx + "_punum", Boost_DYCRMMpileup_num, final_weight, 80, 0., 80.);
                FillHist(syst_name + "/" + pfx + "_jetnum", Boost_DYCRMMjet_num, final_weight, 20, 0., 20.);
                FillHist(syst_name + "/" + pfx + "_pv", Boost_DYCRMMpv, final_weight, 80, 0., 80.);
                FillHist(syst_name + "/" + pfx + "_pvgood", Boost_DYCRMMpvgood, final_weight, 80, 0., 80.);
            };
            for (auto [cond, pfx] : std::initializer_list<std::pair<bool,const char*>>{
                {is_Boosted_DY_MM,              "DYCR_Boosted_MM"},
                {is_Boosted_DY_MM_OS,           "DYCR_Boosted_MM_OS"},
                {is_Boosted_DY_MM_OS_tight,     "DYCR_Boosted_MM_OS_tight"},
                {is_Boosted_DY_MM_OS_not_tight, "DYCR_Boosted_MM_OS_not_tight"},
                {is_Boosted_DY_MM_SS,           "DYCR_Boosted_MM_SS"},
                {is_Boosted_DY_MM_SS_tight,     "DYCR_Boosted_MM_SS_tight"},
                {is_Boosted_DY_MM_SS_not_tight, "DYCR_Boosted_MM_SS_not_tight"},
                // mll-split DY CRs: CR1 = 60-100, CR2 = 100-150
                {is_Boosted_DY_MM_CR1,                         "DYCR1_Boosted_MM"},
                {is_Boosted_DY_MM_CR2,                         "DYCR2_Boosted_MM"},
            }) { if (cond) fill(pfx); }
        }
        //SR EE
        {
            // R multiplies the fill weight, not `weight`: an event can be in
            // both a resolved and a boosted region, and they take different
            // factors. Non-DY and data have R = 1.
            const float final_weight = final_weight_noR * R_boo;
            auto fill = [&](const char* pfx) {
                FillHist(syst_name + "/" + pfx + "_ll_pt", Boost_SREEpt, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/" + pfx + "_leading_fatjet_pt", Boost_SREEfatjetpt, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/" + pfx + "_mlljj", Boost_SREEmlljj, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/" + pfx + "_leading_lep_pt", Boost_SREEleadleppt, final_weight, 2000, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_subleading_lep_pt", Boost_SREEsubleadleppt, final_weight, 2000, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_mass", Boost_SREEmass, final_weight, 2000, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_eta", Boost_SREEeta, final_weight, 100, -5., 5.);
                FillHist(syst_name + "/" + pfx + "_phi", Boost_SREEphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_fatjet_eta", Boost_SREEfatjeteta, final_weight, 100, -2.5, 2.5);
                FillHist(syst_name + "/" + pfx + "_fatjet_phi", Boost_SREEfatjetphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_fatjet_SDM", Boost_SREEfatjetSDM, final_weight, 100, 0., 200.);
                FillHist(syst_name + "/" + pfx + "_mll", Boost_SREEmll, final_weight, 2000, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_WR_pt", Boost_SREEWRpt, final_weight, 800, 0., 8000.);
                FillHist(syst_name + "/" + pfx + "_WR_eta", Boost_SREEWReta, final_weight, 100, -5., 5.);
                FillHist(syst_name + "/" + pfx + "_WR_phi", Boost_SREEWRphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_leading_lep_eta", Boost_SREEleadlepeta, final_weight, 100, -2.5, 2.5);
                FillHist(syst_name + "/" + pfx + "_leading_lep_phi", Boost_SREEleadlepphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_subleading_lep_eta", Boost_SREEsubleadlepeta, final_weight, 100, -2.5, 2.5);
                FillHist(syst_name + "/" + pfx + "_subleading_lep_phi", Boost_SREEsubleadlepphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_fatjet_lsf3", Boost_SREEfatjet_lsf3, final_weight, 100, 0., 1.);
                FillHist(syst_name + "/" + pfx + "_subleadlep_miniiso", Boost_SREEsubleadlep_miniiso, final_weight, 200, 0., 1.);
                FillHist(syst_name + "/" + pfx + "_deltaR_leadlep_fatjet", Boost_SREEdeltaR_leadlep_fatjet, final_weight, 100, 0., 5.);
                FillHist(syst_name + "/" + pfx + "_dphi_leadlep_fatjet", Boost_SREEdphi_leadlep_fatjet, final_weight, 100, 0., 3.14);
                FillHist(syst_name + "/" + pfx + "_punum", Boost_SREEpileup_num, final_weight, 80, 0., 80.);
                FillHist(syst_name + "/" + pfx + "_jetnum", Boost_SREEjet_num, final_weight, 20, 0., 20.);
                FillHist(syst_name + "/" + pfx + "_pv", Boost_SREEpv, final_weight, 80, 0., 80.);
                FillHist(syst_name + "/" + pfx + "_pvgood", Boost_SREEpvgood, final_weight, 80, 0., 80.);
            };
            for (auto [cond, pfx] : std::initializer_list<std::pair<bool,const char*>>{
                {is_Boosted_SR_EE,             "SR_Boosted_EE"},
                {is_Boosted_SR_EE_SS,          "SR_Boosted_EE_SS"},
                {is_Boosted_SR_EE_SS_2e_tight, "SR_Boosted_EE_SS_2e_tight"},
                {is_Boosted_SR_EE_SS_1e_tight, "SR_Boosted_EE_SS_1e_tight"},
                {is_Boosted_SR_EE_SS_0e_tight, "SR_Boosted_EE_SS_0e_tight"},
                {is_Boosted_SR_EE_OS,          "SR_Boosted_EE_OS"},
                {is_Boosted_SR_EE_OS_2e_tight, "SR_Boosted_EE_OS_2e_tight"},
                {is_Boosted_SR_EE_OS_1e_tight, "SR_Boosted_EE_OS_1e_tight"},
                {is_Boosted_SR_EE_OS_0e_tight, "SR_Boosted_EE_OS_0e_tight"},
            }) { if (cond) fill(pfx); }
        }
        //SR MM
        {
            // R multiplies the fill weight, not `weight`: an event can be in
            // both a resolved and a boosted region, and they take different
            // factors. Non-DY and data have R = 1.
            const float final_weight = final_weight_noR * R_boo;
            auto fill = [&](const char* pfx) {
                FillHist(syst_name + "/" + pfx + "_ll_pt", Boost_SRMMpt, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/" + pfx + "_leading_fatjet_pt", Boost_SRMMfatjetpt, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/" + pfx + "_mlljj", Boost_SRMMmlljj, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/" + pfx + "_leading_lep_pt", Boost_SRMMleadleppt, final_weight, 2000, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_subleading_lep_pt", Boost_SRMMsubleadleppt, final_weight, 2000, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_mass", Boost_SRMMmass, final_weight, 2000, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_eta", Boost_SRMMeta, final_weight, 100, -5., 5.);
                FillHist(syst_name + "/" + pfx + "_phi", Boost_SRMMphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_fatjet_eta", Boost_SRMMfatjeteta, final_weight, 100, -2.5, 2.5);
                FillHist(syst_name + "/" + pfx + "_fatjet_phi", Boost_SRMMfatjetphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_fatjet_SDM", Boost_SRMMfatjetSDM, final_weight, 100, 0., 200.);
                FillHist(syst_name + "/" + pfx + "_mll", Boost_SRMMmll, final_weight, 2000, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_WR_pt", Boost_SRMMWRpt, final_weight, 800, 0., 8000.);
                FillHist(syst_name + "/" + pfx + "_WR_eta", Boost_SRMMWReta, final_weight, 100, -5., 5.);
                FillHist(syst_name + "/" + pfx + "_WR_phi", Boost_SRMMWRphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_leading_lep_eta", Boost_SRMMleadlepeta, final_weight, 100, -2.5, 2.5);
                FillHist(syst_name + "/" + pfx + "_leading_lep_phi", Boost_SRMMleadlepphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_subleading_lep_eta", Boost_SRMMsubleadlepeta, final_weight, 100, -2.5, 2.5);
                FillHist(syst_name + "/" + pfx + "_subleading_lep_phi", Boost_SRMMsubleadlepphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_fatjet_lsf3", Boost_SRMMfatjet_lsf3, final_weight, 100, 0., 1.);
                FillHist(syst_name + "/" + pfx + "_subleadlep_miniiso", Boost_SRMMsubleadlep_miniiso, final_weight, 200, 0., 1.);
                FillHist(syst_name + "/" + pfx + "_deltaR_leadlep_fatjet", Boost_SRMMdeltaR_leadlep_fatjet, final_weight, 100, 0., 5.);
                FillHist(syst_name + "/" + pfx + "_dphi_leadlep_fatjet", Boost_SRMMdphi_leadlep_fatjet, final_weight, 100, 0., 3.14);
                FillHist(syst_name + "/" + pfx + "_punum", Boost_SRMMpileup_num, final_weight, 80, 0., 80.);
                FillHist(syst_name + "/" + pfx + "_jetnum", Boost_SRMMjet_num, final_weight, 20, 0., 20.);
                FillHist(syst_name + "/" + pfx + "_pv", Boost_SRMMpv, final_weight, 80, 0., 80.);
                FillHist(syst_name + "/" + pfx + "_pvgood", Boost_SRMMpvgood, final_weight, 80, 0., 80.);
            };
            for (auto [cond, pfx] : std::initializer_list<std::pair<bool,const char*>>{
                {is_Boosted_SR_MM,              "SR_Boosted_MM"},
                {is_Boosted_SR_MM_SS,           "SR_Boosted_MM_SS"},
                {is_Boosted_SR_MM_SS_tight,     "SR_Boosted_MM_SS_tight"},
                {is_Boosted_SR_MM_SS_not_tight, "SR_Boosted_MM_SS_not_tight"},
                {is_Boosted_SR_MM_OS,           "SR_Boosted_MM_OS"},
                {is_Boosted_SR_MM_OS_tight,     "SR_Boosted_MM_OS_tight"},
                {is_Boosted_SR_MM_OS_not_tight, "SR_Boosted_MM_OS_not_tight"},
            }) { if (cond) fill(pfx); }
        }
        //Flav EMJ
        {
            // R multiplies the fill weight, not `weight`: an event can be in
            // both a resolved and a boosted region, and they take different
            // factors. Non-DY and data have R = 1.
            const float final_weight = final_weight_noR * R_boo;
            auto fill = [&](const char* pfx) {
                FillHist(syst_name + "/" + pfx + "_ll_pt", Boost_FlavEMJpt, final_weight, 1000, 0., 1000.);
                FillHist(syst_name + "/" + pfx + "_leading_fatjet_pt", Boost_FlavEMJleadfatjetpt, final_weight, 2000, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_mlljj", Boost_FlavEMJmlljj, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/" + pfx + "_fatjet_SDM", Boost_FlavEMJSDM, final_weight, 10000, 0., 10000.);
                FillHist(syst_name + "/" + pfx + "_leading_lep_pt", Boost_FlavEMJleadleppt, final_weight, 2000, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_subleading_lep_pt", Boost_FlavEMJsubleadleppt, final_weight, 2000, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_leading_lep_eta", Boost_FlavEMJleadlepeta, final_weight, 100, -2.5, 2.5);
                FillHist(syst_name + "/" + pfx + "_subleading_lep_eta", Boost_FlavEMJsubleadlepeta, final_weight, 100, -2.5, 2.5);
                FillHist(syst_name + "/" + pfx + "_fatjet_eta", Boost_FlavEMJfatjeteta, final_weight, 100, -2.5, 2.5);
                FillHist(syst_name + "/" + pfx + "_fatjet_phi", Boost_FlavEMJfatjetphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_deltaR_leadlep_fatjet", Boost_FlavEMJdeltaR_leadlep_fatjet, final_weight, 100, 0., 5.);
                FillHist(syst_name + "/" + pfx + "_leadlep_lsf", Boost_FlavEMJleadlep_lsf, final_weight, 100, 0., 1.);
                FillHist(syst_name + "/" + pfx + "_mll", Boost_FlavEMJmll, final_weight, 1000, 0., 1000.);
                FillHist(syst_name + "/" + pfx + "_fatjet_pt", Boost_FlavEMJfatjetpt, final_weight, 1000, 0., 1000.);
                FillHist(syst_name + "/" + pfx + "_WR_pt", Boost_FlavEMJWRpt, final_weight, 1000, 0., 1000.);
                FillHist(syst_name + "/" + pfx + "_mass", Boost_FlavEMJmass, final_weight, 2000, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_eta", Boost_FlavEMJeta, final_weight, 100, -5., 5.);
                FillHist(syst_name + "/" + pfx + "_phi", Boost_FlavEMJphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_WR_eta", Boost_FlavEMJWReta, final_weight, 100, -5., 5.);
                FillHist(syst_name + "/" + pfx + "_WR_phi", Boost_FlavEMJWRphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_leading_lep_phi", Boost_FlavEMJleadlepphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_subleading_lep_phi", Boost_FlavEMJsubleadlepphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_fatjet_lsf3", Boost_FlavEMJfatjet_lsf3, final_weight, 100, 0., 1.);
                FillHist(syst_name + "/" + pfx + "_subleadlep_miniiso", Boost_FlavEMJsubleadlep_miniiso, final_weight, 200, 0., 1.);
                FillHist(syst_name + "/" + pfx + "_dphi_leadlep_fatjet", Boost_FlavEMJdphi_leadlep_fatjet, final_weight, 100, 0., 3.14);
                FillHist(syst_name + "/" + pfx + "_punum", Boost_FlavEMJpileup_num, final_weight, 80, 0., 80.);
                FillHist(syst_name + "/" + pfx + "_pv", Boost_FlavEMJpv, final_weight, 80, 0., 80.);
                FillHist(syst_name + "/" + pfx + "_pvgood", Boost_FlavEMJpvgood, final_weight, 80, 0., 80.);
                FillHist(syst_name + "/" + pfx + "_jetnum", Boost_FlavEMJjet_num, final_weight, 20, 0., 20.);
            };
            for (auto [cond, pfx] : std::initializer_list<std::pair<bool,const char*>>{
                {is_Boosted_Flav_EMJ,                            "FlavCR_Boosted_EMJ"},
                {is_Boosted_Flav_EMJ_OS,                         "FlavCR_Boosted_EMJ_OS"},
                {is_Boosted_Flav_EMJ_OS_tight,                   "FlavCR_Boosted_EMJ_OS_tight"},
                {is_Boosted_Flav_EMJ_OS_not_tight,               "FlavCR_Boosted_EMJ_OS_not_tight"},
                {is_Boosted_Flav_EMJ_OS_M_tight_E_tight,         "FlavCR_Boosted_EMJ_OS_M_tight_E_tight"},
                {is_Boosted_Flav_EMJ_OS_M_tight_E_not_tight,     "FlavCR_Boosted_EMJ_OS_M_tight_E_not_tight"},
                {is_Boosted_Flav_EMJ_OS_M_not_tight_E_tight,     "FlavCR_Boosted_EMJ_OS_M_not_tight_E_tight"},
                {is_Boosted_Flav_EMJ_OS_M_not_tight_E_not_tight, "FlavCR_Boosted_EMJ_OS_M_not_tight_E_not_tight"},
                {is_Boosted_Flav_EMJ_SS,                         "FlavCR_Boosted_EMJ_SS"},
                {is_Boosted_Flav_EMJ_SS_tight,                   "FlavCR_Boosted_EMJ_SS_tight"},
                {is_Boosted_Flav_EMJ_SS_not_tight,               "FlavCR_Boosted_EMJ_SS_not_tight"},
                {is_Boosted_Flav_EMJ_SS_M_tight_E_tight,         "FlavCR_Boosted_EMJ_SS_M_tight_E_tight"},
                {is_Boosted_Flav_EMJ_SS_M_tight_E_not_tight,     "FlavCR_Boosted_EMJ_SS_M_tight_E_not_tight"},
                {is_Boosted_Flav_EMJ_SS_M_not_tight_E_tight,     "FlavCR_Boosted_EMJ_SS_M_not_tight_E_tight"},
                {is_Boosted_Flav_EMJ_SS_M_not_tight_E_not_tight, "FlavCR_Boosted_EMJ_SS_M_not_tight_E_not_tight"},
            }) { if (cond) fill(pfx); }
        }
        //Flav MEJ
        {
            // R multiplies the fill weight, not `weight`: an event can be in
            // both a resolved and a boosted region, and they take different
            // factors. Non-DY and data have R = 1.
            const float final_weight = final_weight_noR * R_boo;
            auto fill = [&](const char* pfx) {
                FillHist(syst_name + "/" + pfx + "_ll_pt", Boost_FlavMEJpt, final_weight, 1000, 0., 1000.);
                FillHist(syst_name + "/" + pfx + "_leading_fatjet_pt", Boost_FlavMEJleadfatjetpt, final_weight, 2000, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_mlljj", Boost_FlavMEJmlljj, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/" + pfx + "_fatjet_SDM", Boost_FlavMEJSDM, final_weight, 10000, 0., 10000.);
                FillHist(syst_name + "/" + pfx + "_leading_lep_pt", Boost_FlavMEJleadleppt, final_weight, 2000, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_subleading_lep_pt", Boost_FlavMEJsubleadleppt, final_weight, 2000, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_leading_lep_eta", Boost_FlavMEJleadlepeta, final_weight, 100, -2.5, 2.5);
                FillHist(syst_name + "/" + pfx + "_subleading_lep_eta", Boost_FlavMEJsubleadlepeta, final_weight, 100, -2.5, 2.5);
                FillHist(syst_name + "/" + pfx + "_fatjet_eta", Boost_FlavMEJfatjeteta, final_weight, 100, -2.5, 2.5);
                FillHist(syst_name + "/" + pfx + "_fatjet_phi", Boost_FlavMEJfatjetphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_deltaR_leadlep_fatjet", Boost_FlavMEJdeltaR_leadlep_fatjet, final_weight, 100, 0., 5.);
                FillHist(syst_name + "/" + pfx + "_leadlep_lsf", Boost_FlavMEJleadlep_lsf, final_weight, 100, 0., 1.);
                FillHist(syst_name + "/" + pfx + "_mll", Boost_FlavMEJmll, final_weight, 1000, 0., 1000.);
                FillHist(syst_name + "/" + pfx + "_fatjet_pt", Boost_FlavMEJfatjetpt, final_weight, 1000, 0., 1000.);
                FillHist(syst_name + "/" + pfx + "_WR_pt", Boost_FlavMEJWRpt, final_weight, 1000, 0., 1000.);
                FillHist(syst_name + "/" + pfx + "_mass", Boost_FlavMEJmass, final_weight, 2000, 0., 2000.);
                FillHist(syst_name + "/" + pfx + "_eta", Boost_FlavMEJeta, final_weight, 100, -5., 5.);
                FillHist(syst_name + "/" + pfx + "_phi", Boost_FlavMEJphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_WR_eta", Boost_FlavMEJWReta, final_weight, 100, -5., 5.);
                FillHist(syst_name + "/" + pfx + "_WR_phi", Boost_FlavMEJWRphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_leading_lep_phi", Boost_FlavMEJleadlepphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_subleading_lep_phi", Boost_FlavMEJsubleadlepphi, final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/" + pfx + "_fatjet_lsf3", Boost_FlavMEJfatjet_lsf3, final_weight, 100, 0., 1.);
                FillHist(syst_name + "/" + pfx + "_subleadlep_miniiso", Boost_FlavMEJsubleadlep_miniiso, final_weight, 200, 0., 1.);
                FillHist(syst_name + "/" + pfx + "_dphi_leadlep_fatjet", Boost_FlavMEJdphi_leadlep_fatjet, final_weight, 100, 0., 3.14);
                FillHist(syst_name + "/" + pfx + "_punum", Boost_FlavMEJpileup_num, final_weight, 80, 0., 80.);
                FillHist(syst_name + "/" + pfx + "_pv", Boost_FlavMEJpv, final_weight, 80, 0., 80.);
                FillHist(syst_name + "/" + pfx + "_pvgood", Boost_FlavMEJpvgood, final_weight, 80, 0., 80.);
                FillHist(syst_name + "/" + pfx + "_jetnum", Boost_FlavMEJjet_num, final_weight, 20, 0., 20.);
            };
            for (auto [cond, pfx] : std::initializer_list<std::pair<bool,const char*>>{
                {is_Boosted_Flav_MEJ,                            "FlavCR_Boosted_MEJ"},
                {is_Boosted_Flav_MEJ_SS,                         "FlavCR_Boosted_MEJ_SS"},
                {is_Boosted_Flav_MEJ_SS_tight,                   "FlavCR_Boosted_MEJ_SS_tight"},
                {is_Boosted_Flav_MEJ_SS_not_tight,               "FlavCR_Boosted_MEJ_SS_not_tight"},
                {is_Boosted_Flav_MEJ_SS_M_tight_E_tight,         "FlavCR_Boosted_MEJ_SS_M_tight_E_tight"},
                {is_Boosted_Flav_MEJ_SS_M_tight_E_not_tight,     "FlavCR_Boosted_MEJ_SS_M_tight_E_not_tight"},
                {is_Boosted_Flav_MEJ_SS_M_not_tight_E_tight,     "FlavCR_Boosted_MEJ_SS_M_not_tight_E_tight"},
                {is_Boosted_Flav_MEJ_SS_M_not_tight_E_not_tight, "FlavCR_Boosted_MEJ_SS_M_not_tight_E_not_tight"},
                {is_Boosted_Flav_MEJ_OS,                         "FlavCR_Boosted_MEJ_OS"},
                {is_Boosted_Flav_MEJ_OS_tight,                   "FlavCR_Boosted_MEJ_OS_tight"},
                {is_Boosted_Flav_MEJ_OS_not_tight,               "FlavCR_Boosted_MEJ_OS_not_tight"},
                {is_Boosted_Flav_MEJ_OS_M_tight_E_tight,         "FlavCR_Boosted_MEJ_OS_M_tight_E_tight"},
                {is_Boosted_Flav_MEJ_OS_M_tight_E_not_tight,     "FlavCR_Boosted_MEJ_OS_M_tight_E_not_tight"},
                {is_Boosted_Flav_MEJ_OS_M_not_tight_E_tight,     "FlavCR_Boosted_MEJ_OS_M_not_tight_E_tight"},
                {is_Boosted_Flav_MEJ_OS_M_not_tight_E_not_tight, "FlavCR_Boosted_MEJ_OS_M_not_tight_E_not_tight"},
            }) { if (cond) fill(pfx); }
        }
        // syst_name: "Central", "PU_Weight_Up" 등
        // sf_val: 해당 케이스의 SF 값 (예: 0.99)
        
        }

        // --- RunXsecSyst: per-PDF-member mlljj -------------------------------------
        // Central pass only. The member histograms are an input to the OFFLINE envelope
        // (see tables/06_systematics/theory_norm_split), not a nuisance themselves, so
        // they must not be crossed with the object-variation passes.
        //
        // Each member weight is divided by its own inclusive normalisation K_i exactly as
        // the PDF_Weight target is, which makes the envelope acceptance-only: summed over
        // all events every member reproduces the central yield, so what survives after a
        // selection is purely the change in selection efficiency.
        //
        // Member 0 is the central PDF and is written too -- the offline step needs N_0
        // from the same fills, and reading it from the Central directory instead would
        // fold in every other weight target.
        if (RunXsecSyst && !IsDATA && this_syst == "Central" && nLHEPdfWeight >= 103) {
            const float w_xs = weight * weight_map["Central"];
            for (int i = 0; i < 103; i++) {
                const float w_i = w_xs * LHEPdfWeight[i] / GetTheoryNormK(theoryK_pdf, i);
                const TString dir = TString::Format("PDFmem%03d/", i);
                for (auto [cond, pfx, mlljj, boosted, nbin] :
                     std::initializer_list<std::tuple<bool, const char*, float, bool, int>>{
            {is_Resolved_DY_EE, "DYCR_Resolved_EE", Resolve_DYCREEmlljj, false, 800},
            {is_Resolved_DY_MM, "DYCR_Resolved_MM", Resolve_DYCRMMmlljj, false, 800},
            {is_Resolved_SR_EE, "SR_Resolved_EE", Resolve_SREEmlljj, false, 8000},
            {is_Resolved_SR_MM, "SR_Resolved_MM", Resolve_SRMMmlljj, false, 8000},
            {is_Resolved_Flav_EM, "FlavCR_Resolved_EM", Resolve_FlavCRmlljj, false, 8000},
            {is_Boosted_DY_EE, "DYCR_Boosted_EE", Boost_DYCREEmlljj, true, 8000},
            {is_Boosted_DY_MM, "DYCR_Boosted_MM", Boost_DYCRMMmlljj, true, 8000},
            {is_Boosted_SR_EE, "SR_Boosted_EE", Boost_SREEmlljj, true, 8000},
            {is_Boosted_SR_MM, "SR_Boosted_MM", Boost_SRMMmlljj, true, 8000},
            {is_Boosted_Flav_EMJ, "FlavCR_Boosted_EMJ", Boost_FlavEMJmlljj, true, 8000},
            {is_Boosted_Flav_MEJ, "FlavCR_Boosted_MEJ", Boost_FlavMEJmlljj, true, 8000},
                     }) {
                    if (!cond) continue;
                    FillHist(dir + pfx + "_mlljj", mlljj,
                             w_i * (boosted ? R_boo_nom : R_res_nom), nbin, 0., 8000.);
                }
            }
        }
    }
}


    // double counting check?  # 1670

    // Higmass info # 1722 

    // veto HEM  ? # 1748 

    // Fill histograms for main variables # 1771 
    // end ## 1984
    

void HNWR_miniiso::SetSignalFlags() {
    sig_isSignal   = false;
    sig_isOffshell = false;
    sig_isOnshell  = false;
    sig_isTb       = false;

    if (IsDATA) return;

    // (1) Signal tagging: any gen particle with |PID| in {9900012 (N_e), 9900014 (N_mu), 34 (W_R)}
    for (const auto &g : gen_set.gens) {
        int apid = abs(g.PID());
        if (apid == 9900012 || apid == 9900014 || apid == 34) {
            sig_isSignal = true;
            break;
        }
    }

    if (!sig_isSignal) return;

    // (2) Off-shell / on-shell split by reconstructed WR(lljj) invariant mass at LHE level.
    //     Use status==1 (outgoing) LHE leptons+quarks if any exist, otherwise sum all of them.
    auto isLepOrQuark = [](int pid) {
        int a = abs(pid);
        return (a >= 1 && a <= 6) || a == 11 || a == 13 || a == 15;
    };
    bool hasStatus1 = false;
    for (const auto &p : lhe_set.lhe_parts) {
        if (isLepOrQuark(p.PdgId()) && p.Status() == 1) { hasStatus1 = true; break; }
    }
    Particle WRsys;
    for (const auto &p : lhe_set.lhe_parts) {
        if (!isLepOrQuark(p.PdgId())) continue;
        if (hasStatus1 && p.Status() != 1) continue;
        WRsys += p;
    }
    double wrMass = WRsys.M();

    // WR2000 -> no off/on-shell split (threshold < 0); WR4000/6000/8000 -> 2000/4000/5000
    double threshold = -1.;
    if      (MCSample.BeginsWith("WR2000")) threshold = -1.;
    else if (MCSample.BeginsWith("WR4000")) threshold = 2000.;
    else if (MCSample.BeginsWith("WR6000")) threshold = 4000.;
    else if (MCSample.BeginsWith("WR8000")) threshold = 5000.;

    if (threshold > 0.) {
        if (wrMass <= threshold) sig_isOffshell = true;
        else                     sig_isOnshell  = true;
    }

    // (3) tb tagging: signal event with an LHE particle of |PdgId| in {5 (b), 6 (t)}
    for (const auto &p : lhe_set.lhe_parts) {
        int apid = abs(p.PdgId());
        if (apid == 5 || apid == 6) { sig_isTb = true; break; }
    }
}

void HNWR_miniiso::FillSignalCutflow(const TString &this_syst, bool isResolved, double binN, float weight) {
    if (this_syst != "Central") return;
    const TString base  = isResolved ? "/Cutflow_for_reseolved_SR" : "/Cutflow_for_Boosted_SR";
    const int     nbins = isResolved ? 10  : 13;
    const double  xmax  = isResolved ? 10. : 13.;
    // Raw (unweighted) event count per cut stage. TH1 only stores sum(w) and
    // sum(w^2) per bin, so the actual number of events surviving each cut is
    // not recoverable from the weighted cutflow and needs its own histogram.
    FillHist(this_syst + base + "_raw", binN, 1.0, nbins, 0., xmax);
    if (sig_isOffshell) FillHist(this_syst + base + "_offshell", binN, weight, nbins, 0., xmax);
    if (sig_isOnshell)  FillHist(this_syst + base + "_onshell",  binN, weight, nbins, 0., xmax);
    if (sig_isTb)       FillHist(this_syst + base + "_tb",       binN, weight, nbins, 0., xmax);
    // light-quark (u/d/s/c) signal decays: complement of the tb tag among signal events
    if (sig_isSignal && !sig_isTb) FillHist(this_syst + base + "_light", binN, weight, nbins, 0., xmax);
    // all-signal denominator (for tb-fraction / normalization convenience)
    if (sig_isSignal)              FillHist(this_syst + base + "_signal", binN, weight, nbins, 0., xmax);
}

bool HNWR_miniiso::Electrons::isPassCustomTightID(const Electron& el, const HNWR_miniiso::Electrons& eset) const {
    if (fabs(el.scEta()) < 1.566) {
        return el.PassID(eset.Electron_Tight_ID[0]);
    }
    int heepbit = el.VidNestedWPBitmapHEEP();
    if ( !((heepbit & 3775) == 3775) ) return false;
    float scE =  (el.scEtOverPt()+1) * el.Pt();
    double cutValue_HoverE = ( -0.4 + 0.4 * fabs(el.scEta()) ) * el.rho() / scE + 0.05;
    if(! (el.hoe()<cutValue_HoverE) ) return false;

    //==== new EM+Had_depth1 cut
    //double cutValue_emhaddep1 = UncorrPt() > 50. ? 2.5 + 0.03 * (UncorrPt()-50.) +                        0.28 * Rho() : 2.5 +                        0.28 * Rho(); // original cut
    double cutValue_emhaddep1 = el.Pt() > 50. ? 2.5 + 0.03 * (el.Pt()-50.) + (0.15 + 0.07*fabs(el.scEta())) * el.rho() : 2.5 + (0.15 + 0.07*fabs(el.scEta())) * el.rho();
    if(! ( el.dr03EcalRecHitSumEt() + el.dr03HcalDepth1TowerSumEt() < cutValue_emhaddep1 ) ) return false;
    return true;
}

bool HNWR_miniiso::Electrons::isPassCustomLooseID(const Electron& el) const {
    //if (!(el.hoe() < 0.5)) return false;

    if (fabs(el.scEta()) <= 1.479){
    
        if(!(el.hoe() < 0.05 + 1.28/el.E() + 0.0422*el.rho()/el.E())) return false;
        if (!(el.sieie() < 0.00107)) return false;
        if (!(fabs(el.deltaEtaInSeed()) < 0.00691)) return false;
        if (!(fabs(el.deltaPhiInSC()) < 0.175)) return false;
        if (!(fabs(el.eInvMinusPInv()) < 0.138)) return false;
        if (!(el.LostHits() <= 1)) return false;
        if (!(el.ConvVeto())) return false;
        return true;
    }
    else {
        if(!(el.hoe() < 0.05 + 2.3/el.E() + 0.262*el.rho()/el.E())) return false;
        if (!(el.sieie() < 0.0275)) return false;
        if (!(fabs(el.deltaEtaInSeed()) < 0.0121)) return false;
        if (!(fabs(el.deltaPhiInSC()) < 0.228)) return false;
        if (!(fabs(el.eInvMinusPInv()) < 0.127)) return false;
        if (!(el.LostHits() <= 1)) return false;
        if (!(el.ConvVeto())) return false;
        return true;
    }
    return true ;
}

bool HNWR_miniiso::Electrons::isPassLooseNoIso(const Electron& el) const {
    // Matches Python selectLooseElectrons logic:
    // Evaluate vidNestedWPBitmap with id_level=2 (Loose WP), ignoring isolation (cut index 7)
    //
    // vidNestedWPBitmap contains 10 cuts, each encoded in 3 bits:
    //   0: MinPtCut
    //   1: GsfEleSCEtaMultiRangeCut
    //   2: GsfEleDEtaInSeedCut
    //   3: GsfEleDPhiInCut
    //   4: GsfEleFull5x5SigmaIEtaIEtaCut
    //   5: GsfEleHadronicOverEMEnergyScaledCut
    //   6: GsfEleEInverseMinusPInverseCut
    //   7: GsfEleRelPFIsoScaledCut (ISOLATION - ignored)
    //   8: GsfEleConversionVetoCut
    //   9: GsfEleMissingHitsCut
    //
    // Each 3-bit value: 0=fail, 1=Veto, 2=Loose, 3=Medium, 4=Tight

    int bitmap = el.VidNestedWPBitmap();

    const int n_cuts = 10;
    const int bits_per_cut = 3;
    const int ignore_cut = 7;      // Isolation cut index
    const int id_level = 2;        // Loose working point
    const int mask = (1 << bits_per_cut) - 1;  // 0b111 = 7

    for (int cut_nr = 0; cut_nr < n_cuts; cut_nr++) {
        if (cut_nr == ignore_cut) continue;  // Skip isolation

        int value = (bitmap >> (cut_nr * bits_per_cut)) & mask;
        if (value < id_level) return false;
    }
    return true;
}

RVec<FatJet> HNWR_miniiso::Clean_Fatjet_with_tight_leptons(const RVec<FatJet> & fatjets, const RVec<Lepton *> & tight_leps) {
    RVec<FatJet> cleanedfatjets;
    for (unsigned int i=0 ; i< fatjets.size(); i ++) {
        FatJet fatjet = fatjets.at(i);
        bool isDRtoLepton(false);
        for (unsigned int j=0 ; j< tight_leps.size(); j ++) {
            Lepton * lep = tight_leps.at(j);
            if ( fatjet.DeltaR(*lep) < 0.4 ) {
                isDRtoLepton = true;
                break;
            }
        }
        if (!isDRtoLepton) {
            cleanedfatjets.push_back(fatjet);
        }
    }
    return cleanedfatjets;
}

RVec<Jet> HNWR_miniiso::Clean_jet_with_loose_leptons(const RVec<Jet> & jets, const RVec<Lepton *> & loose_leps) {
    RVec<Jet> cleanedjets;
    for (unsigned int i=0 ; i< jets.size(); i ++) {
        Jet jet = jets.at(i);
        bool isDRtoLepton(false);
        for (unsigned int j=0 ; j< loose_leps.size(); j ++) {
            Lepton * lep = loose_leps.at(j);
            if ( jet.DeltaR(*lep) < 0.4 ) {
                isDRtoLepton = true;
                break;
            }
        }
        if (!isDRtoLepton) {
            cleanedjets.push_back(jet);
        }
    }
    return cleanedjets;
}

RVec<Jet> HNWR_miniiso::Clean_LSF_FatJet_with_jets(const RVec<FatJet> & fatjets, const RVec<Jet> & jets) {
    RVec<Jet> cleanedjets;
    for (unsigned int i=0 ; i< jets.size(); i ++) {
        Jet jet = jets.at(i);
        bool isDRtoFatJet(false);
        for (unsigned int j=0 ; j< fatjets.size(); j ++) {
            FatJet fatjet = fatjets.at(j);
            if ( fatjet.DeltaR(jet) < 0.8 ) {
                isDRtoFatJet = true;
                break;
            }
        }
        if (!isDRtoFatJet) {
            cleanedjets.push_back(jet);
        }
    }
    return cleanedjets;
}

RVec<FatJet> HNWR_miniiso::Clean_Jets_with_fatjets(const RVec<Jet> & jets, const RVec<FatJet> & fatjets) {
    RVec<FatJet> cleanedfatjets;
    for (unsigned int i=0 ; i< fatjets.size(); i ++) {
        FatJet fatjet = fatjets.at(i);
        bool isDRtoJet(false);
        for (unsigned int j=0 ; j< jets.size(); j ++) {
            Jet jet = jets.at(j);
            if ( fatjet.DeltaR(jet) < 0.8 ) {
                isDRtoJet = true;
                break;
            }
        }
        if (!isDRtoJet) {
            cleanedfatjets.push_back(fatjet);
        }
    }
    return cleanedfatjets;
}



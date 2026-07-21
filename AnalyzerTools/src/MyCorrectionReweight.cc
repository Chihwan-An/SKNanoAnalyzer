#include "MyCorrection.h"
#include "MLHelper.h"
#include <TLorentzVector.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <numeric>
#include <unordered_map>
#include <vector>

float MyCorrection::GetTopPtReweight(
    const TLorentzVector &LastCopyTop,
    const TLorentzVector &LastCopyAntiTop) const {

  const float t_pt = LastCopyTop.Pt();
  const float t_y = LastCopyTop.Rapidity();
  const float t_phi = LastCopyTop.Phi();
  const float t_mass = LastCopyTop.M();

  const float at_pt = LastCopyAntiTop.Pt();
  const float at_y = LastCopyAntiTop.Rapidity();
  const float at_phi = LastCopyAntiTop.Phi();
  const float at_mass = LastCopyAntiTop.M();

  const TLorentzVector tt = LastCopyTop + LastCopyAntiTop;
  const float tt_pt = tt.Pt();
  const float tt_y = tt.Rapidity();
  const float tt_phi = tt.Phi();
  const float tt_mass = tt.M();

  float feats[12] = {tt_pt, tt_y,   tt_phi, tt_mass, t_pt,   t_y,
                     t_phi, t_mass, at_pt,  at_y,    at_phi, at_mass};

  struct NormSpec {
    double mean;
    double std;
    bool use_log;
  };

  static const NormSpec nrm_tt[4] = {
      {3.6520673599656903, 1.0123402362573612, true},
      {0.0001718810581680775, 1.0362455506718102, false},
      {2.8943571877384285e-05, 1.8139038706413384, false},
      {6.21729978047307, 0.2771419580231537, true},
  };
  static const NormSpec nrm_top[4] = {
      {4.595855742518925, 0.7101176940989488, true},
      {0.00022746366634849002, 1.213207643109532, false},
      {-0.00028213870737636996, 1.8136544140703632, false},
      {171.93706459943778, 6.9652037622153, false},
  };
  static const NormSpec nrm_atop[4] = {
      {4.5986175957604045, 0.7103218938891299, true},
      {0.00011712322394057398, 1.2076422016031159, false},
      {0.0003628069129526392, 1.8139415747773364, false},
      {171.93691192651536, 6.9500586980501575, false},
  };

  auto normalize_block = [](float *x, const NormSpec *specs) {
    for (int j = 0; j < 4; ++j) {
      double v = static_cast<double>(x[j]);
      if (specs[j].use_log) {
        v = std::log(std::max(v, 1e-6));
      }
      v -= specs[j].mean;
      if (specs[j].std >= 1e-2) {
        v /= specs[j].std;
      }
      x[j] = static_cast<float>(v);
    }
  };

  normalize_block(&feats[0], nrm_tt);
  normalize_block(&feats[4], nrm_top);
  normalize_block(&feats[8], nrm_atop);

  std::array<float, 15> input_minnlo;
  // tt
  input_minnlo[0] = feats[0];
  input_minnlo[1] = feats[1];
  input_minnlo[2] = feats[2];
  input_minnlo[3] = feats[3];
  input_minnlo[4] = 0.0f;
  // top
  input_minnlo[5] = feats[4];
  input_minnlo[6] = feats[5];
  input_minnlo[7] = feats[6];
  input_minnlo[8] = feats[7];
  input_minnlo[9] = 0.6f;
  // antitop
  input_minnlo[10] = feats[8];
  input_minnlo[11] = feats[9];
  input_minnlo[12] = feats[10];
  input_minnlo[13] = feats[11];
  input_minnlo[14] = -0.6f;

  std::unordered_map<std::string, VariousArray> inputDataMap;
  static const std::unordered_map<std::string, IntArray> inputShapeMap_toppt = {
      {"input", IntArray{1, 3, 5}}};

  inputDataMap["input"] = FloatArray(input_minnlo.data(),
                                     input_minnlo.data() + input_minnlo.size());
  auto outputDataMap =
      MLHelper_TopPtReweight->Run_ONNX_Model(inputDataMap, inputShapeMap_toppt);
  return outputDataMap["activation_6"][1] / outputDataMap["activation_6"][0];
}

float MyCorrection::GethDampReweight(const TLorentzVector &FirstCopyTop,
                                     const TLorentzVector &FirstCopyAntiTop,
                                     const variation &syst) const {
  if (syst == variation::nom)
    return 1.f;

  const float t_pt = FirstCopyTop.Pt();
  const float t_y = FirstCopyTop.Rapidity();
  const float t_phi = FirstCopyTop.Phi();
  const float t_mass = FirstCopyTop.M();

  const float at_pt = FirstCopyAntiTop.Pt();
  const float at_y = FirstCopyAntiTop.Rapidity();
  const float at_phi = FirstCopyAntiTop.Phi();
  const float at_mass = FirstCopyAntiTop.M();

  const TLorentzVector tt = FirstCopyTop + FirstCopyAntiTop;
  const float tt_pt = tt.Pt();
  if (tt_pt > 1000.f)
    return 1.f;

  FloatArray input_hdamp(12);
  input_hdamp = {
      std::log10(t_pt),  t_y,  t_phi,  t_mass / 243.95f,  0.1, 1.379,
      std::log10(at_pt), at_y, at_phi, at_mass / 243.95f, 0.2, 1.379};

  std::unordered_map<std::string, VariousArray> inputDataMap;
  static const std::unordered_map<std::string, IntArray> inputShapeMap_hdamp = {
      {"input", IntArray{1, 2, 6}}};

  inputDataMap["input"] = input_hdamp;

  MLHelper *this_model = (syst == variation::up) ? MLHelper_hDampUp.get()
                                                 : MLHelper_hDampDown.get();

  auto outputDataMap =
      this_model->Run_ONNX_Model(inputDataMap, inputShapeMap_hdamp);
  return outputDataMap["activation_6"][0] / outputDataMap["activation_6"][1];
}

float MyCorrection::GetBFragReweight(
    const TLorentzVector &LastCopyTop, const TLorentzVector &LastCopyAntiTop,
    const TLorentzVector &LastCopyWPlus, const TLorentzVector &LastCopyWMinus,
    const TLorentzVector &FirstCopyBHadronFromTop,
    const TLorentzVector &FirstCopyBHadronFromAntiTop,
    const variation &syst) const {
  if (syst == variation::down)
    return 1.f;
  const float x_e_top =
      2 * FirstCopyBHadronFromTop * LastCopyTop / LastCopyTop.M2();
  const float x_e_antitop =
      2 * FirstCopyBHadronFromAntiTop * LastCopyAntiTop / LastCopyAntiTop.M2();
  const float w_top = LastCopyWPlus.M2() / LastCopyTop.M2();
  const float w_antitop = LastCopyWMinus.M2() / LastCopyAntiTop.M2();
  const float clip_value = 1.2f;
  const float x_b_top = std::min(x_e_top / (1 - w_top), clip_value);
  const float x_b_antitop = std::min(x_e_antitop / (1 - w_antitop), clip_value);
  FloatArray input_bfrag(4);
  MLHelper *this_model =
      (syst == variation::up) ? MLHelper_rBUp.get() : MLHelper_rBnom.get();
  input_bfrag = {x_b_top, 0.855, x_b_antitop, 0.855};

  std::unordered_map<std::string, VariousArray> inputDataMap;
  static const std::unordered_map<std::string, IntArray> inputShapeMap_bfrag = {
      {"input", IntArray{1, 2, 2}}};

  inputDataMap["input"] = input_bfrag;
  auto outputDataMap =
      this_model->Run_ONNX_Model(inputDataMap, inputShapeMap_bfrag);
  return outputDataMap["activation_6"][0] / outputDataMap["activation_6"][1];
}

std::array<std::size_t, 6>
MyCorrection::GetGenIdxofTopDecayProducts(const GenViewCollection &gens) const {
  constexpr std::size_t npos = std::numeric_limits<std::size_t>::max();
  std::array<std::size_t, 6> out{npos, npos, npos, npos, npos, npos};

  const auto &storage = gens.storage();
  if (!storage)
    return out;

  const std::size_t n = storage->size();
  if (n == 0)
    return out;

  const auto &pt = storage->pt;
  const auto &eta = storage->eta;
  const auto &phi = storage->phi;
  const auto &mass = storage->mass;
  const auto &pdgId = storage->pdgId;
  const auto &statusFlags = storage->statusFlags;
  const auto &motherIdx = storage->motherIdx;

  constexpr unsigned short FIRST_COPY = 1u << 12;
  constexpr unsigned short LAST_COPY = 1u << 13;

  auto isBottomHadron = [](int pdg) {
    int ap = std::abs(pdg);
    if (ap < 500)
      return false;
    int hundreds = (ap / 100) % 10;
    int thousands = (ap / 1000) % 10;
    return (hundreds == 5) || (thousands == 5);
  };

  std::size_t topIdx = npos;
  std::size_t antiTopIdx = npos;
  std::size_t WTopIdx = npos;
  std::size_t WAntiTopIdx = npos;
  std::size_t bFromTopIdx = npos;
  std::size_t bFromAntiTopIdx = npos;

  std::vector<std::size_t> Bcands;
  Bcands.reserve(8);

  for (std::size_t i = 0; i < n; ++i) {
    const int id = pdgId[i];
    const unsigned short flg = statusFlags[i];

    if (flg & LAST_COPY) {
      if (id == 6 && topIdx == npos)
        topIdx = i;
      else if (id == -6 && antiTopIdx == npos)
        antiTopIdx = i;
      else if (id == 24 && WTopIdx == npos)
        WTopIdx = i;
      else if (id == -24 && WAntiTopIdx == npos)
        WAntiTopIdx = i;
      else if (id == 5 && bFromTopIdx == npos)
        bFromTopIdx = i;
      else if (id == -5 && bFromAntiTopIdx == npos)
        bFromAntiTopIdx = i;
    }

    if ((flg & FIRST_COPY) && isBottomHadron(id)) {
      Bcands.push_back(i);
    }
  }

  if (topIdx == npos || antiTopIdx == npos || WTopIdx == npos ||
      WAntiTopIdx == npos) {
    throw runtime_error("[MyCorrection::GetGenIdxofTopDecayProducts] Unable to "
                        "find top/anti-top or W bosons in the event.");
  }

  if (Bcands.size() < 2) {
    throw runtime_error("[MyCorrection::GetGenIdxofTopDecayProducts] Unable to "
                        "find sufficient b-hadron candidates in the event.");
  }

  auto matchBHad = [&](std::size_t bIdx, bool isTopLeg) -> std::size_t {
    if (bIdx == npos || Bcands.empty())
      return npos;

    const float eta_b = eta[bIdx];
    const float phi_b = phi[bIdx];

    double bestDR2 = 1e9;
    std::size_t best = npos;
    const double maxDR2 = 0.2 * 0.2; // DR < 0.2

    for (auto cand : Bcands) {
      const int pid = pdgId[cand];

      // PDG sign 패턴: top vs anti-top
      const int pid100 = (pid / 100) % 10;
      const int pid1000 = (pid / 1000) % 10;

      bool signOK = false;
      if (isTopLeg) {
        // ((pid/100)%10 == -5 || (pid/1000)%10 == 5)
        signOK = (pid100 == -5) || (pid1000 == 5);
      } else {
        // ((pid/100)%10 == 5 || (pid/1000)%10 == -5)
        signOK = (pid100 == 5) || (pid1000 == -5);
      }
      if (!signOK)
        continue;

      // ΔR<0.2 조건
      const double dEta = static_cast<double>(eta[cand]) - eta_b;
      double dPhi = static_cast<double>(phi[cand]) - phi_b;
      if (dPhi > M_PI)
        dPhi -= 2 * M_PI;
      else if (dPhi < -M_PI)
        dPhi += 2 * M_PI;

      const double dr2 = dEta * dEta + dPhi * dPhi;
      if (dr2 < bestDR2 && dr2 < maxDR2) {
        bestDR2 = dr2;
        best = cand;
      }
    }
    return best;
  };

  std::size_t BHadTopIdx = matchBHad(bFromTopIdx, true);
  std::size_t BHadAntiIdx = matchBHad(bFromAntiTopIdx, false);

  out[0] = topIdx;
  out[1] = WTopIdx;
  out[2] = BHadTopIdx;
  out[3] = antiTopIdx;
  out[4] = WAntiTopIdx;
  out[5] = BHadAntiIdx;

  return out;
}

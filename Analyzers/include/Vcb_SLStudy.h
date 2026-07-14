#ifndef Vcb_SLStudy_h
#define Vcb_SLStudy_h

#include <string>
#include <vector>

#include "Vcb_SL.h"

class Vcb_SLStudy : public Vcb_SL {
public:
  Vcb_SLStudy();
  ~Vcb_SLStudy() override = default;

  void initializeAnalyzer() override;
  bool PassBaseLineSelection(bool remove_flavtagging_cut = false,
                             bool loose_cut = false) override;

private:
  enum class JetCleaningMode { TightLeptons, LooseLeptons };
  enum class LooseMuonIsolationMode { Tight, Loose, None };

  struct StudyConfig {
    MuonView::MuonID looseMuonID = Muon_Veto_ID;
    LooseMuonIsolationMode looseMuonIso = LooseMuonIsolationMode::Tight;
    JetCleaningMode jetCleaningMode = JetCleaningMode::TightLeptons;
    JetView::JetID jetID = Jet_ID;
    bool requireMET20 = false;
    std::string looseMuonLabel = "TightID+TightIso";
    std::string jetCleaningLabel = "TightLeptons";
    std::string jetIDLabel = "Tight";
    std::string metLabel = "NoMET20";
  };

  StudyConfig studyConfig_;
  bool studyConfigInitialized_ = false;

  void ConfigureStudyFlags();
  std::vector<std::size_t> BuildLooseMuonCleaningIndices(
      const std::vector<std::size_t> &signal_muon_indices) const;
  std::vector<std::size_t> BuildElectronCleaningIndices(
      const std::vector<std::size_t> &signal_electron_indices,
      const std::vector<std::size_t> &loose_electron_indices) const;
};

#endif

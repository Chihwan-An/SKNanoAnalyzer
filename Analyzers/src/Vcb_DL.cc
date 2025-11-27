#include "Vcb_DL.h"

Vcb_DL::Vcb_DL() {}

bool Vcb_DL::PassBaseLineSelection(bool remove_flavtagging_cut, bool loose_cut)
{
    FillCutFlow(0);
    // if (!IsDATA)
    // {
    //     try
    //     {
    //         myCorr->METXYCorrection(MET, RunNumber, ev.nPVsGood(), Correction::XYCorrection_MetType::Type1PuppiMET);
    //     }
    //     catch (const std::exception &e)
    //     {
    //         std::cerr << e.what() << '\n';
    //     }
    // }
    bool passMuTrig = ev.PassTrigger(Mu_Trigger[DataEra.Data()]);
    bool passElTrig = ev.PassTrigger(El_Trigger[DataEra.Data()]);

    switch (channel)
    {
    case Channel::MM:
        if (!(passMuTrig))
            return false;
        break;
    case Channel::EE:
        if (!(passElTrig))
            return false;
        break;
    case Channel::ME:
        // subtract passMuTrig && passElTrig from electron datastream
        if (IsDATA && (DataStream.Contains("EG") || DataStream.Contains("Electron")) && (passMuTrig && passElTrig))
            return false;
        if (!(passMuTrig || passElTrig))
            return false;
        break;
    default:
        break;
    }
    FillCutFlow(1);
    if (!PassJetVetoMap(AllJetViews, AllMuonViews))
        return false;
    RVec<Jet> eep_veto_jets = SelectJets(AllJetViews, Jet::JetID::NOCUT, 30., INFINITY);
    if (DataEra == "2022EE" && !PassJetVetoMap(eep_veto_jets, AllMuonViews, "jetvetomap_eep"))
        return false;

    FillCutFlow(2);
    if (!PassMetFilter(AllJetViews, ev))
        return false;
    FillCutFlow(3);

    auto jetIndices = SelectJetIndices(AllJetViews, Jet_ID, DL_Jet_Pt_cut, Jet_Eta_cut);
    if (jetIndices.size() < 4)
        return false;
    Jets = MaterializeJets(AllJetViews, jetIndices);
    MET = ev.GetMETVector(Event::MET_Type::PUPPI);

  TLorentzVector p4_nominal(0, 0, 0, 0);
  TLorentzVector p4_shifted(0, 0, 0, 0);
  for (const auto &jetView : AllJetViews) {
    TLorentzVector v;
    v.SetPtEtaPhiM(jetView.Pt(), jetView.Eta(), jetView.Phi(), jetView.Mass());
    p4_nominal += v;
  }

  if (systHelper->getCurrentIterSysTarget().find("Jet_En") !=
      std::string::npos) {
    const bool doBreakdown = HasFlag("doBreakdown");
    const TString srcT = systHelper->getCurrentIterSysSource();
    if (doBreakdown) {
      if (srcT.EqualTo("total", TString::kIgnoreCase))
        return false;
      ApplyJetScaleVariation(AllJetViews, srcT);
    } else {
      if (!srcT.EqualTo("total", TString::kIgnoreCase))
        return false;
      ApplyJetScaleVariation(AllJetViews, "total");
    }

    if (systHelper->getCurrentIterVariation() == MyCorrection::variation::up) {
      for (const auto &jetView : AllJetViews) {
        TLorentzVector v;
        v.SetPtEtaPhiM(jetView.JesPtUp(), jetView.Eta(), jetView.Phi(),
                       jetView.JesMassUp());
        p4_shifted += v;
      }
    } else if (systHelper->getCurrentIterVariation() ==
               MyCorrection::variation::down) {
      for (const auto &jetView : AllJetViews) {
        TLorentzVector v;
        v.SetPtEtaPhiM(jetView.JesPtDown(), jetView.Eta(), jetView.Phi(),
                       jetView.JesMassDown());
        p4_shifted += v;
      }
    }
  } else if (systHelper->getCurrentIterSysTarget() == "Jet_Res") {
    MET = ev.GetMETVector(Event::MET_Type::PUPPI,
                          systHelper->getCurrentIterVariation(),
                          Event::MET_Syst::JER);
    p4_shifted = p4_nominal; // 안 흔들리게
  } else if (systHelper->getCurrentIterSysTarget() == "UE") {
    MET = ev.GetMETVector(Event::MET_Type::PUPPI,
                          systHelper->getCurrentIterVariation(),
                          Event::MET_Syst::UE);
    p4_shifted = p4_nominal;
  } else {
    for (const auto &jetView : AllJetViews) {
      TLorentzVector v;
      v.SetPtEtaPhiM(jetView.SmearedPtNominal(), jetView.Eta(), jetView.Phi(),
                     jetView.SmearedMassNominal());
      p4_shifted += v;
    }
  }

  {
    TLorentzVector delta = p4_shifted - p4_nominal;
    MET.SetXYZM(MET.Px() - delta.Px(), MET.Py() - delta.Py(), 0., 0.);
  }
    Jets = SelectJets(Jets, Jet_ID, DL_Jet_Pt_cut, Jet_Eta_cut);

    Muons_Veto = SelectMuons(AllMuonViews, Muon_Veto_ID, Muon_Veto_Pt, Muon_Veto_Eta);
    Muons_Veto = SelectMuons(Muons_Veto, Muon_Veto_Iso, Muon_Veto_Pt, Muon_Veto_Eta);
    Muons = SelectMuons(AllMuonViews, Muon_Tight_ID, Muon_Tight_Pt[DataEra.Data()], Muon_Tight_Eta);
    Muons = SelectMuons(Muons, Muon_Tight_Iso, Muon_Tight_Pt[DataEra.Data()], Muon_Tight_Eta);
    Electrons_Veto = SelectElectrons(AllElectronViews, Electron_Veto_ID, Electron_Veto_Pt, Electron_Veto_Eta);
    Electrons = SelectElectrons(AllElectronViews, Electron_Tight_ID, Electron_Tight_Pt[DataEra.Data()], Electron_Tight_Eta);
    Jets = JetsVetoLeptonInside(Jets, Electrons_Veto, Muons_Veto, Jet_Veto_DR);
    Jets = SelectJets(Jets, Jet_PUID, DL_Jet_Pt_cut, Jet_Eta_cut);

    HT = GetHT(Jets);
    n_jets = Jets.size();

    if (n_jets < 4)
        return false;
    FillCutFlow(4);
    FillCutFlow(5);
    for (const auto &jet : Jets)
    {
        if (GetPassedBTaggingWP(jet) >= 2)
            n_b_tagged_jets++;
        if (GetPassedCTaggingWP(jet) >= 2)
            n_c_tagged_jets++;
        if (!IsDATA)
        {
            if (abs(jet.partonFlavour()) == 5)
                n_partonFlav_b_jets++;
            if (abs(jet.partonFlavour()) == 4)
                n_partonFlav_c_jets++;
        }
    }

    if (channel == Channel::MM)
    {
        if (!(Muons.size() == 2 && Electrons_Veto.size() == 0 && Muons_Veto.size() == 2))
            return false;
        leptons.push_back(Muons[0]);
        leptons.push_back(Muons[1]);
    }
    else if (channel == Channel::ME)
    {
        if (!(Electrons.size() == 1 && Muons_Veto.size() == 1 && Electrons_Veto.size() == 1 && Muons.size() == 1))
            return false;
        leptons.push_back(Muons[0]);
        leptons.push_back(Electrons[0]);
    }
    else if (channel == Channel::EE)
    {
        if (!(Electrons.size() == 2 && Muons_Veto.size() == 0 && Electrons_Veto.size() == 2))
            return false;
        leptons.push_back(Electrons[0]);
        leptons.push_back(Electrons[1]);
    }
    Particle ZCand = leptons[0] + leptons[1];
    if (channel == Channel::MM || channel == Channel::EE)
    {
        if (fabs(ZCand.M() - Z_MASS) < 15.)
            return false;
        if (fabs(ZCand.M()) < 15.)
            return false;
    }
    if (MET.Pt() < 40.)
        return false;
    if (leptons[0].Charge() * leptons[1].Charge() > 0)
        return false;
    FillCutFlow(6);
    if (n_b_tagged_jets < 2 && !remove_flavtagging_cut)
        return false;
    FillCutFlow(7);
    // order Jets by btagging
    std::sort(Jets.begin(), Jets.end(), [this](const Jet &a, const Jet &b)
              { return JetBScore(a) > JetBScore(b); });
    if (Jets[0].Pt() < 30. && !remove_flavtagging_cut)
        return false;
    if (Jets[1].Pt() < 30. && !remove_flavtagging_cut)
        return false;

    // re-order Jets by Pt
    std::sort(Jets.begin(), Jets.end(), PtComparing);
    SetSystematicLambda();
    SetTTbarId();
    return true;
}

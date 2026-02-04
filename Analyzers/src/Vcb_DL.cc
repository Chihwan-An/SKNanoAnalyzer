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

    std::vector<std::size_t> Muons_Veto_indices = SelectMuonIndices(
        AllMuonViews, Muon_Veto_ID, Muon_Veto_Pt, Muon_Veto_Eta);
    Muons_Veto_indices =
        SelectMuonIndices(AllMuonViews, Muons_Veto_indices, Muon_Veto_Iso,
                          Muon_Veto_Pt, Muon_Veto_Eta);

    std::vector<std::size_t> Muons_indices;
    if (HasFlag("Skim"))
    {
        Muons_indices.clear();
    }
    else
    {
        Muons_indices =
            SelectMuonIndices(AllMuonViews, Muons_Veto_indices, Muon_Tight_ID,
                              Muon_Tight_Pt[DataEra.Data()], Muon_Tight_Eta);
    }

    Muons_indices =
        SelectMuonIndices(AllMuonViews, Muons_indices, Muon_Tight_Iso,
                          Muon_Tight_Pt[DataEra.Data()], Muon_Tight_Eta);

    std::vector<std::size_t> Electron_Veto_indices = SelectElectronIndices(
        AllElectronViews, Electron_Veto_ID, Electron_Veto_Pt, Electron_Veto_Eta);

    std::vector<std::size_t> Electrons_indices;
    if (HasFlag("Skim"))
    {
        Electrons_indices.clear();
    }
    else
    {
        Electrons_indices = SelectElectronIndices(
            AllElectronViews, Electron_Veto_indices, Electron_Tight_ID,
            Electron_Tight_Pt[DataEra.Data()], Electron_Tight_Eta);
    }

    if (HasFlag("Skim"))
    {
        auto select_tight_muons = [&](Muon::MuonID id, bool require_iso)
        {
            std::vector<std::size_t> indices = SelectMuonIndices(
                AllMuonViews, id, Muon_Tight_Pt[DataEra.Data()], Muon_Tight_Eta);
            if (require_iso)
            {
                indices =
                    SelectMuonIndices(AllMuonViews, indices, Muon_Tight_Iso,
                                      Muon_Tight_Pt[DataEra.Data()], Muon_Tight_Eta);
            }
            return indices;
        };

        auto select_tight_electrons = [&](Electron::ElectronID id)
        {
            return SelectElectronIndices(AllElectronViews, id,
                                         Electron_Tight_Pt[DataEra.Data()],
                                         Electron_Tight_Eta);
        };

        auto count_extra_loose = [](const std::vector<std::size_t> &loose,
                                    const std::vector<std::size_t> &tight)
        {
            std::size_t count = 0;
            for (const auto idx : loose)
            {
                if (std::find(tight.begin(), tight.end(), idx) == tight.end())
                    ++count;
            }
            return count;
        };

        const std::vector<std::size_t> mu_tight_pog =
            select_tight_muons(Muon::MuonID::POG_TIGHT, true);
        const std::vector<std::size_t> mu_tight_prompt =
            select_tight_muons(Muon::MuonID::POG_PROMPTMVA_WP0p64, false);
        const std::vector<std::size_t> el_tight_wp80 =
            select_tight_electrons(Electron::ElectronID::POG_MVAISO_WP80);
        const std::vector<std::size_t> el_tight_prompt =
            select_tight_electrons(Electron::ElectronID::POG_PROMPTMVA_MEDIUM);

        auto pass_mm_case = [&](const std::vector<std::size_t> &mu_tight)
        {
            if (mu_tight.size() != 2)
                return false;
            if (count_extra_loose(Muons_Veto_indices, mu_tight) != 0)
                return false;
            if (!Electron_Veto_indices.empty())
                return false;
            return true;
        };

        auto pass_ee_case = [&](const std::vector<std::size_t> &el_tight)
        {
            if (el_tight.size() != 2)
                return false;
            if (count_extra_loose(Electron_Veto_indices, el_tight) != 0)
                return false;
            if (!Muons_Veto_indices.empty())
                return false;
            return true;
        };

        auto pass_me_case = [&](const std::vector<std::size_t> &mu_tight,
                                const std::vector<std::size_t> &el_tight)
        {
            if (mu_tight.size() != 1)
                return false;
            if (el_tight.size() != 1)
                return false;
            if (count_extra_loose(Muons_Veto_indices, mu_tight) != 0)
                return false;
            if (count_extra_loose(Electron_Veto_indices, el_tight) != 0)
                return false;
            return true;
        };

        if (channel == Channel::MM)
        {
            if (pass_mm_case(mu_tight_pog))
            {
                Muons_indices = mu_tight_pog;
                Electrons_indices.clear();
            }
            else if (pass_mm_case(mu_tight_prompt))
            {
                Muons_indices = mu_tight_prompt;
                Electrons_indices.clear();
            }
            else
            {
                return false;
            }
        }
        else if (channel == Channel::EE)
        {
            if (pass_ee_case(el_tight_wp80))
            {
                Muons_indices.clear();
                Electrons_indices = el_tight_wp80;
            }
            else if (pass_ee_case(el_tight_prompt))
            {
                Muons_indices.clear();
                Electrons_indices = el_tight_prompt;
            }
            else
            {
                return false;
            }
        }
        else if (channel == Channel::ME)
        {
            if (pass_me_case(mu_tight_pog, el_tight_wp80))
            {
                Muons_indices = mu_tight_pog;
                Electrons_indices = el_tight_wp80;
            }
            else if (pass_me_case(mu_tight_prompt, el_tight_prompt))
            {
                Muons_indices = mu_tight_prompt;
                Electrons_indices = el_tight_prompt;
            }
            else
            {
                return false;
            }
        }
    }

    Muons_Veto = MaterializeMuons(AllMuonViews, Muons_Veto_indices);
    Electrons_Veto = MaterializeElectrons(AllElectronViews, Electron_Veto_indices);
    Muons = MaterializeMuons(AllMuonViews, Muons_indices);
    Electrons = MaterializeElectrons(AllElectronViews, Electrons_indices);

    MyCorrection::variation jesVar = MyCorrection::variation::nom;
    MyCorrection::variation jerVar = MyCorrection::variation::nom;
    if (!PropagateJetSystToMET(AllJetViews, *systHelper, ev, MET, jesVar,
                               jerVar))
        return false;

    std::vector<std::size_t> jetIndices = SelectJetIndices(
        AllJetViews, Jet_ID, DL_Jet_Pt_cut, Jet_Eta_cut, jesVar, jerVar);
    jetIndices = JetsVetoLeptonInside(AllJetViews, jetIndices, AllElectronViews,
                                      Electron_Veto_indices, AllMuonViews,
                                      Muons_Veto_indices, Jet_Veto_DR);
    if (jetIndices.size() < 4)
        return false;
    Jets = MaterializeJets(AllJetViews, jetIndices, jesVar, jerVar);

    HT = GetHT(Jets);
    n_jets = Jets.size();

    if (n_jets < 4)
        return false;
    FillCutFlow(4);
    FillCutFlow(5);
    for (const auto &jet : Jets)
    {
        if (GetPassedBTaggingWP(jet) >= 1)
            n_b_tagged_jets++;
        if (GetPassedCTaggingWP(jet) >= 1)
            n_c_tagged_jets++;
        if (!IsDATA)
        {
            if (abs(jet.hadronFlavour()) == 5)
                n_hadronFlav_b_jets++;
            if (abs(jet.hadronFlavour()) == 4)
                n_hadronFlav_c_jets++;
        }
    }

    if (channel == Channel::MM)
    {
        if (Muons.size() != 2)
            return false;
        if (!HasFlag("Skim"))
        {
            if (!(Electrons_Veto.size() == 0 && Muons_Veto.size() == 2))
                return false;
        }
        leptons.push_back(Muons[0]);
        leptons.push_back(Muons[1]);
    }
    else if (channel == Channel::ME)
    {
        if (!(Electrons.size() == 1 && Muons.size() == 1))
            return false;
        if (!HasFlag("Skim"))
        {
            if (!(Muons_Veto.size() == 1 && Electrons_Veto.size() == 1))
                return false;
        }
        leptons.push_back(Muons[0]);
        leptons.push_back(Electrons[0]);
    }
    else if (channel == Channel::EE)
    {
        if (Electrons.size() != 2)
            return false;
        if (!HasFlag("Skim"))
        {
            if (!(Muons_Veto.size() == 0 && Electrons_Veto.size() == 2))
                return false;
        }
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
    // re-order Jets by Pt
    std::sort(Jets.begin(), Jets.end(), PtComparing);
    SetSystematicLambda();
    SetTTbarId();
    return true;
}

void Vcb_DL::FillHistogramsAtThisPoint(std::string_view histPrefix,
                                       float weight)
{
    Vcb::FillHistogramsAtThisPoint(histPrefix, weight);

    if (Jets.size() < 4)
        return;

    std::vector<std::size_t> probb_order;
    probb_order.reserve(Jets.size());
    for (std::size_t i = 0; i < Jets.size(); ++i)
    {
        probb_order.push_back(i);
    }
    std::sort(probb_order.begin(), probb_order.end(),
              [&](std::size_t a, std::size_t b)
              { return JetProbBScore(Jets[a]) > JetProbBScore(Jets[b]); });

    const Jet &jet3 = Jets[probb_order[2]];
    const Jet &jet4 = Jets[probb_order[3]];

    const int cat3 = static_cast<int>(JetCategory(jet3));
    const int cat4 = static_cast<int>(JetCategory(jet4));
    const int cat_min = std::min(cat3, cat4);
    const int cat_max = std::max(cat3, cat4);
    constexpr int kCatBins = 12;
    constexpr int kMergedBins = kCatBins * (kCatBins + 1) / 2;
    // Merge (3rd,4th) and (4th,3rd) into the same bin (upper-triangle index).
    const int unrolled = cat_min + (cat_max * (cat_max + 1)) / 2;

    std::string name(histPrefix);
    name.append("/Jet34_Cat_Unrolled_Probb_Merged");
    FillHist(name, unrolled, weight, kMergedBins, 0.f,
             static_cast<float>(kMergedBins));
}

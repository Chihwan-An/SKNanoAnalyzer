#ifndef FATJETVIEW_H
#define FATJETVIEW_H

#include <memory>
#include <cstddef>
#include <vector>

#include "TLorentzVector.h"
#include "ViewColumns.h"



struct FatJetSoA {
    ColumnView<float> area ;
    ColumnView<float> chEmEF ;
    ColumnView<float> chHEF ;
    ColumnView<short> chMultiplicity ;
    ColumnView<short> electronIdx3SJ ;
    ColumnView<float> eta ;
    ColumnView<short> genJetAK8Idx ;
    ColumnView<float> globalParT3_QCD ;
    ColumnView<float> globalParT3_TopbWev ;
    ColumnView<float> globalParT3_TopbWmv ;
    ColumnView<float> globalParT3_TopbWq ;  
    ColumnView<float> globalParT3_TopbWqq ;
    ColumnView<float> globalParT3_TopbWtauhv ;
    ColumnView<float> globalParT3_WvsQCD ;
    ColumnView<float> globalParT3_XWW3q ;
    ColumnView<float> globalParT3_XWW4q ;
    ColumnView<float> globalParT3_XWWqqev ;
    ColumnView<float> globalParT3_XWWqqmv ;
    ColumnView<float> globalParT3_Xbb ;
    ColumnView<float> globalParT3_Xcc ;
    ColumnView<float> globalParT3_Xcs ;
    ColumnView<float> globalParT3_Xqq ;
    ColumnView<float> globalParT3_Xtauhtaue ;
    ColumnView<float> globalParT3_Xtauhtauh ;
    ColumnView<float> globalParT3_Xtauhtaum ;
    ColumnView<float> globalParT3_massCorrGeneric ;
    ColumnView<float> globalParT3_massCorrX2p ;
    ColumnView<float> globalParT3_withMassTopvsQCD ;
    ColumnView<float> globalParT3_withMassWvsQCD ;
    ColumnView<float> globalParT3_withMassZvsQCD ;
    ColumnView<unsigned char> hadronFlavour ;
    ColumnView<float> hfEmEF ;
    ColumnView<float> hfHEF ;
    ColumnView<float> lsf3 ;
    ColumnView<float> mass ;
    ColumnView<float> msoftdrop ;
    ColumnView<float> muEF ;
    ColumnView<short> muonIdx3SJ ;
    ColumnView<float> n2b1 ;
    ColumnView<float> n3b1 ;
    ColumnView<unsigned char> nConstituents ;
    ColumnView<float> neEmEF ;
    ColumnView<float> neHEF ;
    ColumnView<short> neMultiplicity ;
    ColumnView<float> particleNetLegacy_QCD ;
    ColumnView<float> particleNetLegacy_Xbb ;
    ColumnView<float> particleNetLegacy_Xcc ;
    ColumnView<float> particleNetLegacy_Xqq ;
    ColumnView<float> particleNetLegacy_mass ;
    ColumnView<float> particleNetWithMass_H4qvsQCD ;
    ColumnView<float> particleNetWithMass_HbbvsQCD ;
    ColumnView<float> particleNetWithMass_HccvsQCD ;
    ColumnView<float> particleNetWithMass_QCD ;
    ColumnView<float> particleNetWithMass_TvsQCD ;
    ColumnView<float> particleNetWithMass_WvsQCD ;
    ColumnView<float> particleNetWithMass_ZvsQCD ;
    ColumnView<float> particleNet_QCD ;
    ColumnView<float> particleNet_QCD0HF ;
    ColumnView<float> particleNet_QCD1HF ;
    ColumnView<float> particleNet_QCD2HF ;
    ColumnView<float> particleNet_WVsQCD ;
    ColumnView<float> particleNet_XbbVsQCD ;
    ColumnView<float> particleNet_XccVsQCD ;
    ColumnView<float> particleNet_XggVsQCD ;
    ColumnView<float> particleNet_XqqVsQCD ;
    ColumnView<float> particleNet_XteVsQCD ;
    ColumnView<float> particleNet_XtmVsQCD ;
    ColumnView<float> particleNet_XttVsQCD ;
    ColumnView<float> particleNet_massCorr ;
    ColumnView<float> phi ;
    ColumnView<float> pt ;
    ColumnView<float> rawFactor ;   
    ColumnView<short> subJetIdx1 ;
    ColumnView<short> subJetIdx2 ;
    ColumnView<float> tau1 ;
    ColumnView<float> tau2 ;
    ColumnView<float> tau3 ;
    ColumnView<float> tau4 ;
    
    ColumnView<float> massCorrGeneric ;
    ColumnView<float> massCorrX2p ;
    ColumnView<float> massCorr ;
    
    std::vector<float> jecFactor;
    std::vector<float> correctedPt;
    std::vector<float> correctedMass;
    std::vector<float> smearedPtNominal;
    std::vector<float> smearedPtUp;
    std::vector<float> smearedPtDown;
    std::vector<float> smearedMassNominal;
    std::vector<float> smearedMassUp;
    std::vector<float> smearedMassDown;
    std::vector<float> jesPtUp;
    std::vector<float> jesPtDown;
    std::vector<float> jesMassUp;
    std::vector<float> jesMassDown;

    std::size_t size() const { return pt.size(); }
};

class FatJetView {
public:
    FatJetView() = default;
    FatJetView(std::shared_ptr<const FatJetSoA> storage, std::size_t index)
        : store(std::move(storage)), idx(index) {}

    bool valid() const { return static_cast<bool>(store) && idx < store->size(); }

    float Area() const {return store-> area[idx];}
    float ChEmEF() const {return store-> chEmEF[idx];}
    float ChHEF() const {return store-> chHEF[idx];}
    short ChMultiplicity() const {return store-> chMultiplicity[idx];}
    short Electronidx3Sj() const {return store-> electronIdx3SJ[idx];}
    float Eta() const {return store-> eta[idx];}
    short Genjetak8Idx() const {return store-> genJetAK8Idx[idx];}
    float Globalpart3_Qcd() const {return store-> globalParT3_QCD[idx];}
    float Globalpart3_Topbwev() const {return store-> globalParT3_TopbWev[idx];}
    float Globalpart3_Topbwmv() const {return store-> globalParT3_TopbWmv[idx];}
    float Globalpart3_Topbwq() const {return store-> globalParT3_TopbWq[idx];}
    float Globalpart3_Topbwqq() const {return store-> globalParT3_TopbWqq[idx];}
    float Globalpart3_Topbwtauhv() const {return store-> globalParT3_TopbWtauhv[idx];}
    float Globalpart3_Wvsqcd() const {return store-> globalParT3_WvsQCD[idx];}
    float Globalpart3_Xww3Q() const {return store-> globalParT3_XWW3q[idx];}
    float Globalpart3_Xww4Q() const {return store-> globalParT3_XWW4q[idx];}
    float Globalpart3_Xwwqqev() const {return store-> globalParT3_XWWqqev[idx];}
    float Globalpart3_Xwwqqmv() const {return store-> globalParT3_XWWqqmv[idx];}
    float Globalpart3_Xbb() const {return store-> globalParT3_Xbb[idx];}
    float Globalpart3_Xcc() const {return store-> globalParT3_Xcc[idx];}
    float Globalpart3_Xcs() const {return store-> globalParT3_Xcs[idx];}
    float Globalpart3_Xqq() const {return store-> globalParT3_Xqq[idx];}
    float Globalpart3_Xtauhtaue() const {return store-> globalParT3_Xtauhtaue[idx];}
    float Globalpart3_Xtauhtauh() const {return store-> globalParT3_Xtauhtauh[idx];}
    float Globalpart3_Xtauhtaum() const {return store-> globalParT3_Xtauhtaum[idx];}
    float Globalpart3_Masscorrgeneric() const {return store-> globalParT3_massCorrGeneric[idx];}
    float Globalpart3_Masscorrx2P() const {return store-> globalParT3_massCorrX2p[idx];}
    float Globalpart3_Withmasstopvsqcd() const {return store-> globalParT3_withMassTopvsQCD[idx];}
    float Globalpart3_Withmasswvsqcd() const {return store-> globalParT3_withMassWvsQCD[idx];}
    float Globalpart3_Withmasszvsqcd() const {return store-> globalParT3_withMassZvsQCD[idx];}
    unsigned char HadronFlavour() const {return store-> hadronFlavour[idx];}
    float Hfemef() const {return store-> hfEmEF[idx];}
    float Hfhef() const {return store-> hfHEF[idx];}
    float Lsf3() const {return store-> lsf3[idx];}
    float Mass() const {return store-> mass[idx];}
    float Msoftdrop() const {return store-> msoftdrop[idx];}
    float MuEF() const {return store-> muEF[idx];}
    short Muonidx3Sj() const {return store-> muonIdx3SJ[idx];}
    float N2B1() const {return store-> n2b1[idx];}
    float N3B1() const {return store-> n3b1[idx];}
    unsigned char NConstituents() const {return store-> nConstituents[idx];}
    float NeEmEF() const {return store-> neEmEF[idx];}
    float NeHEF() const {return store-> neHEF[idx];}
    short NeMultiplicity() const {return store-> neMultiplicity[idx];}
    float Particlenetlegacy_Qcd() const {return store-> particleNetLegacy_QCD[idx];}
    float Particlenetlegacy_Xbb() const {return store-> particleNetLegacy_Xbb[idx];}
    float Particlenetlegacy_Xcc() const {return store-> particleNetLegacy_Xcc[idx];}
    float Particlenetlegacy_Xqq() const {return store-> particleNetLegacy_Xqq[idx];}
    float Particlenetlegacy_Mass() const {return store-> particleNetLegacy_mass[idx];}
    float Particlenetwithmass_H4Qvsqcd() const {return store-> particleNetWithMass_H4qvsQCD[idx];}
    float Particlenetwithmass_Hbbvsqcd() const {return store-> particleNetWithMass_HbbvsQCD[idx];}
    float Particlenetwithmass_Hccvsqcd() const {return store-> particleNetWithMass_HccvsQCD[idx];}
    float Particlenetwithmass_Qcd() const {return store-> particleNetWithMass_QCD[idx];}
    float Particlenetwithmass_Tvsqcd() const {return store-> particleNetWithMass_TvsQCD[idx];}
    float Particlenetwithmass_Wvsqcd() const {return store-> particleNetWithMass_WvsQCD[idx];}
    float Particlenetwithmass_Zvsqcd() const {return store-> particleNetWithMass_ZvsQCD[idx];}
    float Particlenet_Qcd() const {return store-> particleNet_QCD[idx];}
    float Particlenet_Qcd0Hf() const {return store-> particleNet_QCD0HF[idx];}
    float Particlenet_Qcd1Hf() const {return store-> particleNet_QCD1HF[idx];}
    float Particlenet_Qcd2Hf() const {return store-> particleNet_QCD2HF[idx];}
    float Particlenet_Wvsqcd() const {return store-> particleNet_WVsQCD[idx];}
    float Particlenet_Xbbvsqcd() const {return store-> particleNet_XbbVsQCD[idx];}
    float Particlenet_Xccvsqcd() const {return store-> particleNet_XccVsQCD[idx];}
    float Particlenet_Xggvsqcd() const {return store-> particleNet_XggVsQCD[idx];}
    float Particlenet_Xqqvsqcd() const {return store-> particleNet_XqqVsQCD[idx];}
    float Particlenet_Xtevsqcd() const {return store-> particleNet_XteVsQCD[idx];}
    float Particlenet_Xtmvsqcd() const {return store-> particleNet_XtmVsQCD[idx];}
    float Particlenet_Xttvsqcd() const {return store-> particleNet_XttVsQCD[idx];}
    float Particlenet_Masscorr() const {return store-> particleNet_massCorr[idx];}
    float Phi() const {return store-> phi[idx];}
    float Pt() const {return store-> pt[idx];}
    float RawFactor() const {return store-> rawFactor[idx];}
    short Subjetidx1() const {return store-> subJetIdx1[idx];}
    short Subjetidx2() const {return store-> subJetIdx2[idx];}
    float Tau1() const {return store-> tau1[idx];}
    float Tau2() const {return store-> tau2[idx];}
    float Tau3() const {return store-> tau3[idx];}
    float Tau4() const {return store-> tau4[idx];}

    float MassCorrGeneric() const {return store-> massCorrGeneric[idx];}
    float MassCorrX2p() const {return store-> massCorrX2p[idx];}
    float MassCorr() const {return store-> massCorr[idx];}

    float JECFactor() const { return idx < store->jecFactor.size() ? store->jecFactor[idx] : 1.f; }
    float CorrectedPt() const { return idx < store->correctedPt.size() ? store->correctedPt[idx] : Pt(); }
    float CorrectedMass() const { return idx < store->correctedMass.size() ? store->correctedMass[idx] : Mass(); }
    float SmearedPtNominal() const { return idx < store->smearedPtNominal.size() ? store->smearedPtNominal[idx] : CorrectedPt(); }
    float SmearedPtUp() const { return idx < store->smearedPtUp.size() ? store->smearedPtUp[idx] : CorrectedPt(); }
    float SmearedPtDown() const { return idx < store->smearedPtDown.size() ? store->smearedPtDown[idx] : CorrectedPt(); }
    float SmearedMassNominal() const { return idx < store->smearedMassNominal.size() ? store->smearedMassNominal[idx] : CorrectedMass(); }
    float SmearedMassUp() const { return idx < store->smearedMassUp.size() ? store->smearedMassUp[idx] : CorrectedMass(); }
    float SmearedMassDown() const { return idx < store->smearedMassDown.size() ? store->smearedMassDown[idx] : CorrectedMass(); }
    float JesPtUp() const { return idx < store->jesPtUp.size() ? store->jesPtUp[idx] : SmearedPtNominal(); }
    float JesPtDown() const { return idx < store->jesPtDown.size() ? store->jesPtDown[idx] : SmearedPtNominal(); }
    float JesMassUp() const { return idx < store->jesMassUp.size() ? store->jesMassUp[idx] : SmearedMassNominal(); }
    float JesMassDown() const { return idx < store->jesMassDown.size() ? store->jesMassDown[idx] : SmearedMassNominal(); }
    TLorentzVector P4() const {
        TLorentzVector v;
        v.SetPtEtaPhiM(Pt(), Eta(), Phi(), Mass());
        return v;
    }

private:
    std::shared_ptr<const FatJetSoA> store;
    std::size_t idx = 0;
};

class FatJetViewCollection {
public:
    FatJetViewCollection() = default;
    explicit FatJetViewCollection(std::shared_ptr<FatJetSoA> payload)
        : storage_(std::move(payload)) {
        if (storage_) {
            const std::size_t n = storage_->size();
            views.reserve(n);
            for (std::size_t i = 0; i < n; ++i) {
                views.emplace_back(storage_, i);
            }
        }
    }

    const FatJetView &operator[](std::size_t index) const { return views[index]; }
    std::size_t size() const { return views.size(); }
    bool empty() const { return views.empty(); }

    auto begin() const { return views.begin(); }
    auto end() const { return views.end(); }

    const std::shared_ptr<FatJetSoA> &storage() const { return storage_; }

private:
    std::shared_ptr<FatJetSoA> storage_;
    std::vector<FatJetView> views;
};

#endif // FATJETVIEW_H
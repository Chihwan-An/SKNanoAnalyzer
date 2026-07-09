#ifndef FatJet_h
#define FatJet_h

#include <array>
#include <bitset>
#include <memory>
#include <limits>

#include "Particle.h"
#include "JetTaggingParameter.h"
#include "JetConstituent.h"
#include "FatJetView.h"

class AnalyzerCore;

class FatJet : public Particle
{

public:
    FatJet();
    FatJet(std::shared_ptr<const FatJetSoA> storage, std::size_t index);
    ~FatJet();

    enum class Property
    {
        Part3QCD,
        Part3TopbWev,
        Part3TopbWmv,
        Part3TopbWq,
        Part3TopbWqq,
        Part3TopbWtauhv,
        Part3WvsQCD,
        Part3Xww3Q,
        Part3Xww4Q,
        Part3Xwwqqev,
        Part3Xwwqqmv,
        Part3Xbb,
        Part3Xcc,
        Part3Xcs,
        Part3Xqq,
        Part3Xtauhtaue,
        Part3Xtauhtauh,
        Part3Xtauhtaum,
        Part3WithMassTvsQcd,
        Part3WithMassWvsQcd,
        Part3WithMassZvsQcd,
        PNetLegacyQcd,
        PNetLegacyXbb,
        PNetLegacyXcc,
        PNetLegacyXqq,
        PNetLegacyMass,
        PNetWithmassH4Qvsqcd,
        PNetWithmassHbbvsqcd,
        PNetWithmassHccvsqcd,
        PNetWithmassQcd,
        PNetWithMassTvsQcd,
        PNetWithMassWvsQcd,
        PNetWithMassZvsQcd,
        PNetQcd,
        PNetQcd0Hf,
        PNetQcd1Hf,
        PNetQcd2Hf,
        PNetWvsQcd,
        PNetXbbvsQcd,
        PNetXccvsQcd,
        PNetXggvsQcd,
        PNetXqqvsQcd,
        PNetXtevsQcd,
        PNetXtmvsQcd,
        PNetXttvsQcd,
        Count
    };

    using EnsureCallback = void (*)(void*, FatJet&, Property);
    void AttachLazyPayload(void *context, EnsureCallback callback, int index) const;
    void DetachLazyPayload() const;
    bool HasLazyPayload() const { return static_cast<bool>(lazy_); }

    enum class FatJetID
    {
        NOCUT,
        TIGHT,
        TIGHTLEPVETO
    };

    inline void SetRawPt(float pt) { fj_rawPt = pt; };
    inline void SetOriginalPt(float pt) { fj_originalPt = pt; };
    float GetRawPt() const { return fj_rawPt; };
    float GetOriginalPt() const { return fj_originalPt; };

    inline void SetArea(double area) { fj_area = area; };
    inline void SetJetFlavours(unsigned char hf)
    {
    fj_hadronFlavour = static_cast<short>(hf);
    };
    inline unsigned char hadronFlavour() const { return fj_hadronFlavour; };
    
    void SetTaggerScore(JetTagging::FatJetTagger tagger, JetTagging::FatJetTaggerScoreType scoreType, float value);
    inline void SetTaggerResults(RVec<float> parT,RVec<float> parTWithMass,RVec<float> pnet, RVec<float> pnetWithMass)
    {
        SetTaggerScore(JetTagging::FatJetTagger::ParT, JetTagging::FatJetTaggerScoreType::QCD, parT[0]);
        SetTaggerScore(JetTagging::FatJetTagger::ParT, JetTagging::FatJetTaggerScoreType::TopbWev, parT[1]);
        SetTaggerScore(JetTagging::FatJetTagger::ParT, JetTagging::FatJetTaggerScoreType::TopbWmv, parT[2]);
        SetTaggerScore(JetTagging::FatJetTagger::ParT, JetTagging::FatJetTaggerScoreType::TopbWq, parT[3]);
        SetTaggerScore(JetTagging::FatJetTagger::ParT, JetTagging::FatJetTaggerScoreType::TopbWqq, parT[4]);
        SetTaggerScore(JetTagging::FatJetTagger::ParT, JetTagging::FatJetTaggerScoreType::TopbWtauhv, parT[5]);
        SetTaggerScore(JetTagging::FatJetTagger::ParT, JetTagging::FatJetTaggerScoreType::WvsQCD, parT[6]);
        SetTaggerScore(JetTagging::FatJetTagger::ParT, JetTagging::FatJetTaggerScoreType::XWW3q, parT[7]);
        SetTaggerScore(JetTagging::FatJetTagger::ParT, JetTagging::FatJetTaggerScoreType::XWW4q, parT[8]);
        SetTaggerScore(JetTagging::FatJetTagger::ParT, JetTagging::FatJetTaggerScoreType::XWWqqev, parT[9]);
        SetTaggerScore(JetTagging::FatJetTagger::ParT, JetTagging::FatJetTaggerScoreType::XWWqqmv, parT[10]);
        SetTaggerScore(JetTagging::FatJetTagger::ParT, JetTagging::FatJetTaggerScoreType::Xbb, parT[11]);
        SetTaggerScore(JetTagging::FatJetTagger::ParT, JetTagging::FatJetTaggerScoreType::Xcc, parT[12]);
        SetTaggerScore(JetTagging::FatJetTagger::ParT, JetTagging::FatJetTaggerScoreType::Xcs, parT[13]);
        SetTaggerScore(JetTagging::FatJetTagger::ParT, JetTagging::FatJetTaggerScoreType::Xqq, parT[14]);
        SetTaggerScore(JetTagging::FatJetTagger::ParT, JetTagging::FatJetTaggerScoreType::Xtauhtaue, parT[15]);
        SetTaggerScore(JetTagging::FatJetTagger::ParT, JetTagging::FatJetTaggerScoreType::Xtauhtauh, parT[16]);
        SetTaggerScore(JetTagging::FatJetTagger::ParT, JetTagging::FatJetTaggerScoreType::Xtauhtaum, parT[17]);
        
        SetTaggerScore(JetTagging::FatJetTagger::ParTWithMass, JetTagging::FatJetTaggerScoreType::TvsQCD, parTWithMass[0]);
        SetTaggerScore(JetTagging::FatJetTagger::ParTWithMass, JetTagging::FatJetTaggerScoreType::WvsQCD, parTWithMass[1]);
        SetTaggerScore(JetTagging::FatJetTagger::ParTWithMass, JetTagging::FatJetTaggerScoreType::ZvsQCD, parTWithMass[2]);

        SetTaggerScore(JetTagging::FatJetTagger::ParticleNet, JetTagging::FatJetTaggerScoreType::LegacyQCD, pnet[0]);
        SetTaggerScore(JetTagging::FatJetTagger::ParticleNet, JetTagging::FatJetTaggerScoreType::LegacyXbb, pnet[1]);
        SetTaggerScore(JetTagging::FatJetTagger::ParticleNet, JetTagging::FatJetTaggerScoreType::LegacyXcc, pnet[2]);
        SetTaggerScore(JetTagging::FatJetTagger::ParticleNet, JetTagging::FatJetTaggerScoreType::LegacyXqq, pnet[3]);
        SetTaggerScore(JetTagging::FatJetTagger::ParticleNet, JetTagging::FatJetTaggerScoreType::LegacyMass, pnet[4]);
        SetTaggerScore(JetTagging::FatJetTagger::ParticleNet, JetTagging::FatJetTaggerScoreType::QCD, pnet[5]);
        SetTaggerScore(JetTagging::FatJetTagger::ParticleNet, JetTagging::FatJetTaggerScoreType::QCD0HF, pnet[6]);
        SetTaggerScore(JetTagging::FatJetTagger::ParticleNet, JetTagging::FatJetTaggerScoreType::QCD1HF, pnet[7]);
        SetTaggerScore(JetTagging::FatJetTagger::ParticleNet, JetTagging::FatJetTaggerScoreType::QCD2HF, pnet[8]);
        SetTaggerScore(JetTagging::FatJetTagger::ParticleNet, JetTagging::FatJetTaggerScoreType::WvsQCD, pnet[9]);
        SetTaggerScore(JetTagging::FatJetTagger::ParticleNet, JetTagging::FatJetTaggerScoreType::XbbVsQCD, pnet[10]);
        SetTaggerScore(JetTagging::FatJetTagger::ParticleNet, JetTagging::FatJetTaggerScoreType::XccVsQCD, pnet[11]);
        SetTaggerScore(JetTagging::FatJetTagger::ParticleNet, JetTagging::FatJetTaggerScoreType::XqqVsQCD, pnet[12]);
        SetTaggerScore(JetTagging::FatJetTagger::ParticleNet, JetTagging::FatJetTaggerScoreType::XggVsQCD, pnet[13]);
        SetTaggerScore(JetTagging::FatJetTagger::ParticleNet, JetTagging::FatJetTaggerScoreType::XteVsQCD, pnet[14]);
        SetTaggerScore(JetTagging::FatJetTagger::ParticleNet, JetTagging::FatJetTaggerScoreType::XtmVsQCD, pnet[15]);
        SetTaggerScore(JetTagging::FatJetTagger::ParticleNet, JetTagging::FatJetTaggerScoreType::XttVsQCD, pnet[16]);
        

        SetTaggerScore(JetTagging::FatJetTagger::ParticleNetWithMass, JetTagging::FatJetTaggerScoreType::QCD, pnetWithMass[0]);
        SetTaggerScore(JetTagging::FatJetTagger::ParticleNetWithMass, JetTagging::FatJetTaggerScoreType::TvsQCD, pnetWithMass[1]);
        SetTaggerScore(JetTagging::FatJetTagger::ParticleNetWithMass, JetTagging::FatJetTaggerScoreType::WvsQCD, pnetWithMass[2]);
        SetTaggerScore(JetTagging::FatJetTagger::ParticleNetWithMass, JetTagging::FatJetTaggerScoreType::ZvsQCD, pnetWithMass[3]);
        SetTaggerScore(JetTagging::FatJetTagger::ParticleNetWithMass, JetTagging::FatJetTaggerScoreType::H4qvsQCD, pnetWithMass[4]);
        SetTaggerScore(JetTagging::FatJetTagger::ParticleNetWithMass, JetTagging::FatJetTaggerScoreType::HbbvsQCD, pnetWithMass[5]);
        SetTaggerScore(JetTagging::FatJetTagger::ParticleNetWithMass, JetTagging::FatJetTaggerScoreType::HccvsQCD, pnetWithMass[6]);
        };
    
    inline void SetEnergyFractions(float cH, float nH, float nEM, float cEM, float muE)
    {
        fj_chHEF = cH;
        fj_neHEF = nH;
        fj_neEmEF = nEM;
        fj_chEmEF = cEM;
        fj_muEF = muE;
    };

    inline float chHEF() const { return fj_chHEF; }
    inline float neHEF() const { return fj_neHEF; }
    inline float neEmEF() const { return fj_neEmEF; }
    inline float chEmEF() const { return fj_chEmEF; }
    inline float muEF() const { return fj_muEF; }
    inline float EMFraction() const { return fj_chEmEF + fj_neEmEF; }

    inline void SetMultiplicities(unsigned char nC){fj_nConstituents = short(nC);};
    inline short nConstituents() const { return fj_nConstituents; }

    inline void SetHadronMultiplicities(unsigned char chMult , unsigned char neMult)
    {
        fj_chMultiplicity = chMult;
        fj_neMultiplicity = neMult;
    };

    inline unsigned char chMultiplicity() const { return fj_chMultiplicity; }
    inline unsigned char neMultiplicity() const { return fj_neMultiplicity; }

    inline void SetMatchingIndices(short e1,  short m1,  short sv1, short sv2, short gj8)
    {
        fj_electronIdx3SJ = e1;
        fj_muonIdx3SJ = m1;
        fj_subJetIdx1 = sv1;
        fj_subJetIdx2 = sv2;
        fj_genJetAK8Idx = gj8;
    };
    inline void SetMatchingIndices(short e1,  short m1,  short sv1, short sv2)
    {
        fj_electronIdx3SJ = e1;
        fj_muonIdx3SJ = m1;
        fj_subJetIdx1 = sv1;
        fj_subJetIdx2 = sv2;
    };

    inline short electronIdx3SJ() const { return fj_electronIdx3SJ; }
    inline short muonIdx3SJ() const { return fj_muonIdx3SJ; }
    inline short subJetIdx1() const { return fj_subJetIdx1; }
    inline short subJetIdx2() const { return fj_subJetIdx2; }
    inline short genJetAK8Idx() const { return fj_genJetAK8Idx; }

    inline void SetOriginalIndex(int idx) { fj_originalIndex = idx; };
    inline int OriginalIndex() const { return fj_originalIndex; };

    inline void SetCorrection(RVec<float> corrs)
    {
        fj_massCorrGeneric = corrs[0];
        fj_massCorrX2p = corrs[1];
        fj_massCorr = corrs[2];
        fj_rawFactor = corrs[3];
    };

    inline float MassCorrGeneric() const { return fj_massCorrGeneric; };
    inline float MassCorrX2p() const { return fj_massCorrX2p; };
    inline float MassCorr() const { return fj_massCorr; };
    inline float SetRawFactor() const { return fj_rawFactor; };

    inline void SetM(double fatjet_m){fj_mass = fatjet_m;};
    inline void SetSDM(double fatjet_msoftdrop){fj_msoftdrop = fatjet_msoftdrop;};
    inline double GetM() const { return fj_mass; };
    inline double GetSDM() const { return fj_msoftdrop; };

    inline void SetUnsmearedP4 (FatJet fatjet ){fj_unsmearedP4 = fatjet;};
    
    void SetPFConstituents(RVec<JetConstituent> constituents){fj_constituents = std::move(constituents);}
    const RVec<JetConstituent> &PFConstituents() const noexcept{return fj_constituents;}

    float GetTaggerResult(JetTagging::FatJetTagger tagger, JetTagging::FatJetTaggerScoreType) const;
    TLorentzVector GetUnsmearedP4() const;

    inline void SetLSF3(float lsf3) { fj_lsf3 = lsf3; };
    inline float LSF3() const { return fj_lsf3; };

    inline void SetSubjettiness(float tau1, float tau2, float tau3, float n2b1, float n3b1)
    {
        fj_tau1 = tau1;
        fj_tau2 = tau2;
        fj_tau3 = tau3;
        fj_n2b1 = n2b1;
        fj_n3b1 = n3b1;
    };
    inline float Tau1() const { return fj_tau1; };
    inline float Tau2() const { return fj_tau2; };
    inline float Tau3() const { return fj_tau3; };
    inline float N2b1() const { return fj_n2b1; };
    inline float N3b1() const { return fj_n3b1; };




    


private:

    void ensure(Property property) const;
    void markLoaded(Property property) const;
    void ensureScore(JetTagging::FatJetTagger tagger, JetTagging::FatJetTaggerScoreType scoreType) const;
    static Property propertyFor(JetTagging::FatJetTagger tagger, JetTagging::FatJetTaggerScoreType scoreType);

    void initializeMembers();
    void materialize() const;
    void loadFromStorage(Property property) const;

    struct LazyPayload {
        void *context = nullptr;
        EnsureCallback callback = nullptr;
        int index = -1;
        mutable std::bitset<static_cast<std::size_t>(Property::Count)> loaded;
    };

    static constexpr float kInvalidTagScore = -1.f;

    std::shared_ptr<const FatJetSoA> storage_;
    std::size_t index_ = std::numeric_limits<std::size_t>::max();

    mutable std::bitset<static_cast<std::size_t>(Property::Count)> cachedProperties_;
    mutable std::shared_ptr<LazyPayload> lazy_;

    // ───── Basic Jet Properties ─────────────
    float fj_pt, fj_eta, fj_phi, fj_mass, fj_rawFactor, fj_area;

    // ───── Energy Fractions ────────────────
    float fj_chEmEF, fj_chHEF, fj_hfEmEF, fj_hfHEF, fj_muEF, fj_neEmEF, fj_neHEF;

    // ───── Multiplicities ──────────────────
    short fj_chMultiplicity, fj_neMultiplicity;
    unsigned char fj_nConstituents;
    // ───── Indices and Flavors ─────────────
    short fj_genJetAK8Idx, fj_subJetIdx1, fj_subJetIdx2;
    short fj_muonIdx3SJ, fj_electronIdx3SJ;
    unsigned char fj_hadronFlavour;

    // ───── Jet Substructure ────────────────
    float fj_msoftdrop, fj_n2b1, fj_n3b1, fj_lsf3;
    float fj_tau1, fj_tau2, fj_tau3, fj_tau4;
    // ───── ParticleNet WithMass ────────────
    float fj_particleNetWithMass_H4qvsQCD, fj_particleNetWithMass_HbbvsQCD, fj_particleNetWithMass_HccvsQCD;
    float fj_particleNetWithMass_QCD, fj_particleNetWithMass_TvsQCD, fj_particleNetWithMass_WvsQCD, fj_particleNetWithMass_ZvsQCD;
    std::array<float, JetTagging::FatJetFlavTaggerScoreCount> fj_particleNetWithMass;

    // ───── ParticleNet W/O mass ───────────
    float fj_particleNet_QCD, fj_particleNet_QCD0HF, fj_particleNet_QCD1HF, fj_particleNet_QCD2HF;
    float fj_particleNet_WVsQCD, fj_particleNet_XbbVsQCD, fj_particleNet_XccVsQCD;
    float fj_particleNet_XggVsQCD, fj_particleNet_XqqVsQCD, fj_particleNet_XteVsQCD, fj_particleNet_XtmVsQCD, fj_particleNet_XttVsQCD;
    float fj_particleNet_massCorr;
    std::array<float, JetTagging::FatJetFlavTaggerScoreCount> fj_particleNet;
    // ───── parT Taggers ─────────────
    float fj_globalParT3_QCD, fj_globalParT3_TopbWev, fj_globalParT3_TopbWmv, fj_globalParT3_TopbWq, fj_globalParT3_TopbWqq, fj_globalParT3_TopbWtauhv;
    float fj_globalParT3_WvsQCD;
    float fj_globalParT3_XWW3q, fj_globalParT3_XWW4q, fj_globalParT3_XWWqqev, fj_globalParT3_XWWqqmv;
    float fj_globalParT3_Xbb, fj_globalParT3_Xcc, fj_globalParT3_Xcs, fj_globalParT3_Xqq;
    float fj_globalParT3_Xtauhtaue, fj_globalParT3_Xtauhtauh, fj_globalParT3_Xtauhtaum;
    float fj_globalParT3_massCorrGeneric, fj_globalParT3_massCorrX2p;
    float fj_globalParT3_withMassTvsQCD, fj_globalParT3_withMassWvsQCD, fj_globalParT3_withMassZvsQCD;
    std::array<float, JetTagging::FatJetFlavTaggerScoreCount> fj_globalParT3;
    std::array<float, JetTagging::FatJetFlavTaggerScoreCount> fj_globalParT3WithMass;

    // ───── PF constituents ──────────────────────────
    RVec<JetConstituent> fj_constituents;

    // ───── Matching Information ────────────────────
    size_t fj_originalIndex; // original index in the event

    float fj_massCorrGeneric, fj_massCorrX2p, fj_massCorr;

    float fj_rawPt;
    float fj_originalPt;

    TLorentzVector fj_unsmearedP4;
    friend class AnalyzerCore;
    ClassDef(FatJet, 1)
};

#endif

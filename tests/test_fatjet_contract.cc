#include <cmath>
#include <cstdint>
#include <exception>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

#include "FatJetView.h"

namespace {

template <typename T>
class FakeColumn final : public SKNano::ColumnSource<T> {
public:
    T get(std::size_t index) const override { return values.at(index); }
    std::size_t size() const override { return values.size(); }
    bool valid() const override { return available; }
    std::uint64_t epoch() const override { return currentEpoch; }
    SKNano::ContiguousView<T> snapshot() const override {
        return {this, values.data(), values.size(), currentEpoch};
    }

    std::vector<T> values;
    bool available = true;
    std::uint64_t currentEpoch = 1;
};

void require(bool condition, const char *message) {
    if (!condition)
        throw std::runtime_error(message);
}

template <typename Function>
bool throwsLogicError(Function &&function) {
    try {
        function();
    } catch (const SKNano::LogicError &) {
        return true;
    } catch (...) {
    }
    return false;
}

void testTaggerScoreRouting() {
    using Tagger = JetTagging::FatJetTagger;
    using Score = JetTagging::FatJetTaggerScoreType;

    FakeColumn<float> pt;
    FakeColumn<float> particleNetQcd;
    FakeColumn<float> particleNetMassCorr;
    FakeColumn<float> particleNetWithMassH4q;
    FakeColumn<float> globalParTMassCorrGeneric;
    FakeColumn<float> globalParTWithMassTop;
    pt.values = {300.f};
    particleNetQcd.values = {1.f};
    particleNetMassCorr.values = {13.f};
    particleNetWithMassH4q.values = {101.f};
    globalParTMassCorrGeneric.values = {222.f};
    globalParTWithMassTop.values = {219.f};

    auto storage = std::make_shared<FatJetSoA>();
    storage->pt.bind(&pt);
    storage->score(Tagger::ParticleNet, Score::QCD).bind(&particleNetQcd);
    storage->score(Tagger::ParticleNet, Score::MassCorr)
        .bind(&particleNetMassCorr);
    storage->score(Tagger::ParticleNetWithMass, Score::H4qvsQCD)
        .bind(&particleNetWithMassH4q);
    storage->score(Tagger::ParT, Score::MassCorrGeneric)
        .bind(&globalParTMassCorrGeneric);
    storage->score(Tagger::ParTWithMass, Score::TopvsQCD)
        .bind(&globalParTWithMassTop);

    const FatJetView jet(storage.get(), 0);
    require(jet.GetTaggerResult(Tagger::ParticleNet, Score::QCD) == 1.f,
            "ParticleNet QCD must use its own column");
    require(jet.GetTaggerResult(Tagger::ParticleNet, Score::MassCorr) == 13.f,
            "ParticleNet mass correction must use its own column");
    require(jet.GetTaggerResult(Tagger::ParticleNetWithMass,
                                Score::H4qvsQCD) == 101.f,
            "ParticleNetWithMass must not fall through to another tagger");
    require(jet.GetTaggerResult(Tagger::ParT,
                                Score::MassCorrGeneric) == 222.f,
            "GlobalParT generic mass correction must preserve routing");
    require(jet.GetTaggerResult(Tagger::ParTWithMass,
                                Score::TopvsQCD) == 219.f,
            "GlobalParT-with-mass top score must use its dedicated column");
    require(throwsLogicError([&] {
                static_cast<void>(jet.GetTaggerResult(Tagger::ParT,
                                                      Score::WvsQCD));
            }),
            "an unavailable score column must fail explicitly");
}

void testConstituentAndMultiplicityContract() {
    FakeColumn<float> pt;
    FakeColumn<short> charged;
    FakeColumn<short> neutral;
    FakeColumn<unsigned char> total;
    FakeColumn<int> associationJet;
    FakeColumn<int> associationPFCand;
    FakeColumn<float> pfPt;
    FakeColumn<float> pfEta;
    FakeColumn<float> pfPhi;
    FakeColumn<float> pfMass;
    FakeColumn<int> pfPdgId;
    FakeColumn<float> pfPuppiWeight;

    pt.values = {300.f};
    charged.values = {4};
    neutral.values = {7};
    total.values = {11};
    associationJet.values = {0, 0};
    associationPFCand.values = {0, 1};
    pfPt.values = {45.f, 20.f};
    pfEta.values = {0.1f, -0.2f};
    pfPhi.values = {1.f, -1.f};
    pfMass.values = {0.14f, 0.f};
    pfPdgId.values = {211, 22};
    pfPuppiWeight.values = {0.75f, 0.25f};

    auto storage = std::make_shared<FatJetSoA>();
    storage->pt.bind(&pt);
    storage->chMultiplicity.bind(&charged);
    storage->neMultiplicity.bind(&neutral);
    storage->nConstituents.bind(&total);
    storage->constituentJetIdx.bind(&associationJet);
    storage->constituentPFCandIdx.bind(&associationPFCand);
    storage->pfCandPt.bind(&pfPt);
    storage->pfCandEta.bind(&pfEta);
    storage->pfCandPhi.bind(&pfPhi);
    storage->pfCandMass.bind(&pfMass);
    storage->pfCandPdgId.bind(&pfPdgId);
    storage->pfCandPuppiWeight.bind(&pfPuppiWeight);

    const FatJetView jet(storage.get(), 0);
    require(jet.chMultiplicity() == 4 && jet.neMultiplicity() == 7 &&
                jet.nConstituents() == 11,
            "fat-jet multiplicity columns must preserve charged/neutral/total order");

    const auto constituents = jet.PFConstituents();
    require(constituents.size() == 2 && constituents[0].PdgId() == 211 &&
                constituents[1].PdgId() == 22 &&
                std::abs(constituents[0].PUPPIWeight() - 0.75f) < 1.e-6f,
            "fat-jet constituent refs must resolve complete PFCand content");
}

} // namespace

int main() {
    try {
        testTaggerScoreRouting();
        testConstituentAndMultiplicityContract();
    } catch (const std::exception &error) {
        std::cerr << "test_fatjet_contract: " << error.what() << '\n';
        return 1;
    }
    return 0;
}

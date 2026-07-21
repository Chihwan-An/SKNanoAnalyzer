#include <cmath>
#include <cstdint>
#include <exception>
#include <iostream>
#include <memory>
#include <unordered_set>
#include <vector>

#include "AnalysisException.h"
#include "BranchManager.h"
#include "ElectronView.h"
#include "EventRange.h"
#include "FatJetView.h"
#include "JetView.h"
#include "MuonView.h"
#include "SelectedJetView.h"
#include "ViewColumns.h"

namespace {

template <typename T>
class FakeColumnSource final : public SKNano::ColumnSource<T> {
public:
    std::vector<T> values;
    bool isValid = true;
    std::uint64_t currentEpoch = 0;
    mutable std::size_t getCalls = 0;
    mutable std::size_t sizeCalls = 0;

    T get(std::size_t index) const override { ++getCalls; return values.at(index); }
    std::size_t size() const override { ++sizeCalls; return values.size(); }
    bool valid() const override { return isValid; }
    std::uint64_t epoch() const override { return currentEpoch; }
    SKNano::ContiguousView<T> snapshot() const override {
        return SKNano::ContiguousView<T>(this, values.data(), values.size(),
                                         currentEpoch);
    }
};

template <>
class FakeColumnSource<bool> final : public SKNano::ColumnSource<bool> {
public:
    std::vector<std::uint8_t> values;
    bool isValid = true;
    std::uint64_t currentEpoch = 0;
    mutable std::size_t getCalls = 0;
    mutable std::size_t sizeCalls = 0;

    bool get(std::size_t index) const override { ++getCalls; return values.at(index) != 0; }
    std::size_t size() const override { ++sizeCalls; return values.size(); }
    bool valid() const override { return isValid; }
    std::uint64_t epoch() const override { return currentEpoch; }
    SKNano::ByteContiguousView snapshot() const override {
        return SKNano::ByteContiguousView(this, values.data(), values.size(),
                                           currentEpoch);
    }
};

template <typename Exception, typename Function>
bool throws(Function &&function) {
    try {
        function();
    } catch (const Exception &) {
        return true;
    } catch (...) {
    }
    return false;
}

void require(bool condition, const char *message) {
    if (!condition)
        throw std::runtime_error(message);
}

void testViewEpochGuards() {
    ColumnView<float> unboundFloatView;
    require(throws<SKNano::ConfigError>([&] {
                static_cast<void>(unboundFloatView.size());
            }),
            "unbound float view access must fail explicitly");
    BoolColumnView unboundBoolView;
    require(throws<SKNano::ConfigError>([&] {
                static_cast<void>(unboundBoolView.snapshot());
            }),
            "unbound bool view access must fail explicitly");

    FakeColumnSource<float> floats;
    floats.values = {1.5f, 2.5f};
    floats.currentEpoch = 41;
    ColumnView<float> floatView;
    floatView.bind(&floats);
    const ColumnView<float> copiedFloatView = floatView;

    require(floatView.size() == 2, "float view size must be available in its bound event");
    require(floatView[1] == 2.5f, "float view value must be available in its bound event");
    const auto floatSnapshot = floatView.snapshot();
    require(floatSnapshot.size() == 2 && floatSnapshot.data()[0] == 1.5f,
            "snapshot must directly expose the current contiguous column buffer");

    ++floats.currentEpoch;
    require(throws<std::logic_error>([&] { static_cast<void>(floatSnapshot.size()); }),
            "stale contiguous float snapshot access must fail in every build");
    require(throws<SKNano::LogicError>([&] { static_cast<void>(floatView[0]); }),
            "stale float view access must fail");
    require(throws<SKNano::LogicError>([&] { static_cast<void>(copiedFloatView.size()); }),
            "copied stale float view access must fail");
    ++floats.currentEpoch;
    require(throws<SKNano::LogicError>([&] { static_cast<void>(floatView.size()); }),
            "a view stale by two events must fail");

    floatView.bind(&floats);
    require(floatView[0] == 1.5f, "rebinding must make the current event readable");
    floats.isValid = false;
    require(throws<SKNano::ConfigError>([&] {
                static_cast<void>(floatView[0]);
            }),
            "an unavailable float column must not look like a zero value");
    floats.isValid = true;

    FakeColumnSource<bool> bools;
    bools.values = {1, 0};
    bools.currentEpoch = 8;
    BoolColumnView boolView;
    boolView.bind(&bools);
    require(boolView[0] && !boolView[1], "bool view values must be available in their bound event");
    const auto boolSnapshot = boolView.snapshot();
    require(boolSnapshot.size() == 2 && boolSnapshot[0] && !boolSnapshot[1],
            "bool snapshot must expose a byte-backed contiguous buffer");
    bools.isValid = false;
    require(throws<SKNano::ConfigError>([&] {
                static_cast<void>(boolView[0]);
            }),
            "an unavailable bool column must not look like false");
    bools.isValid = true;

    ++bools.currentEpoch;
    require(throws<std::logic_error>([&] { static_cast<void>(boolSnapshot.data()); }),
            "stale byte snapshot access must fail in every build");
    require(throws<SKNano::LogicError>([&] { static_cast<void>(boolView.size()); }),
            "stale bool view access must fail");
}

struct FakeObjectStorage {
    ColumnView<float> pt;
    std::vector<float> correctedPt;
    std::size_t size() const { return pt.size(); }
};

class FakeObjectRef {
public:
    FakeObjectRef() = default;
    FakeObjectRef(const FakeObjectStorage *storage, std::size_t index)
        : storage_(storage), index_(index) {}

    float Pt() const { return storage_->pt[index_]; }
    float CorrectedPt() const {
        static_cast<void>(storage_->size());
        return storage_->correctedPt[index_];
    }

private:
    const FakeObjectStorage *storage_ = nullptr;
    std::size_t index_ = 0;
};

void testEventRangeContract() {
    using Range = EventRange<FakeObjectStorage, FakeObjectRef>;

    Range empty;
    require(empty.empty(), "a default event range must be empty");

    FakeColumnSource<float> source;
    source.values = {10.f, 20.f, 30.f};
    source.currentEpoch = 4;
    auto storage = std::make_shared<FakeObjectStorage>();
    storage->pt.bind(&source);
    storage->correctedPt = {11.f, 22.f, 33.f};

    Range all(storage);
    require(source.sizeCalls == 0 && source.getCalls == 0,
            "constructing an event range must not activate its primary column");
    require(all.size() == 3 && all[1].Pt() == 20.f &&
                all[1].CorrectedPt() == 22.f,
            "an identity event range must create refs on demand");

    float sum = 0.f;
    for (const auto object : all)
        sum += object.Pt();
    require(sum == 60.f, "event range iteration must preserve object order");

    Range selected(storage, {2, 0});
    require(selected.size() == 2 && selected.rawIndex(0) == 2 &&
                selected[0].Pt() == 30.f && selected[1].Pt() == 10.f,
            "a selected event range must retain only raw indices");

    const auto escapedRef = all[0];
    ++source.currentEpoch;
    require(throws<SKNano::LogicError>([&] { static_cast<void>(all.size()); }),
            "a stale event range must fail before exposing its cardinality");
    require(throws<SKNano::LogicError>(
                [&] { static_cast<void>(escapedRef.CorrectedPt()); }),
            "a stale ref must not expose derived event lanes");
}

void testFatJetDemandDrivenFields() {
    using Tagger = JetTagging::FatJetTagger;
    using Score = JetTagging::FatJetTaggerScoreType;

    FakeColumnSource<float> pt;
    FakeColumnSource<float> pnetQcd;
    FakeColumnSource<float> partQcd;
    FakeColumnSource<int> associationJet;
    FakeColumnSource<int> associationPFCand;
    FakeColumnSource<float> pfPt;
    pt.values = {300.f};
    pnetQcd.values = {0.2f};
    partQcd.values = {0.4f};
    associationJet.values = {0};
    associationPFCand.values = {0};
    pfPt.values = {45.f};

    auto storage = std::make_shared<FatJetSoA>();
    storage->pt.bind(&pt);
    storage->score(Tagger::ParticleNet, Score::QCD).bind(&pnetQcd);
    storage->score(Tagger::ParT, Score::QCD).bind(&partQcd);
    storage->constituentJetIdx.bind(&associationJet);
    storage->constituentPFCandIdx.bind(&associationPFCand);
    storage->pfCandPt.bind(&pfPt);

    FatJetViewCollection jets(storage);
    require(pt.sizeCalls == 0 && pnetQcd.getCalls == 0 &&
                partQcd.getCalls == 0 && associationJet.sizeCalls == 0,
            "binding a fat-jet range must not read kinematics, scores, or constituents");

    const auto jet = jets[0];
    require(jet.GetTaggerResult(Tagger::ParticleNet, Score::QCD) == 0.2f,
            "the requested fat-jet tagger score must be readable");
    require(pnetQcd.getCalls == 1 && partQcd.getCalls == 0,
            "requesting one tagger score must not read another tagger branch");
    require(associationJet.sizeCalls == 0,
            "tagger access must not inspect constituent associations");

    const auto constituents = jet.PFConstituents();
    require(associationJet.sizeCalls == 0,
            "constructing a constituent range must remain lazy");
    require(constituents.size() == 1 && constituents[0].Pt() == 45.f,
            "constituent access must resolve the matching PFCand on demand");
}

void testMuonDemandMomentumLane() {
    FakeColumnSource<float> pt;
    FakeColumnSource<float> eta;
    pt.values = {25.f};
    eta.values = {1.2f};
    auto storage = std::make_shared<MuonSoA>();
    storage->pt.bind(&pt);
    storage->eta.bind(&eta);
    int populateCalls = 0;
    storage->populateMomentum = [target = storage.get(), &populateCalls] {
        ++populateCalls;
        target->correctedPt = {26.f};
        target->miniAODPt = {25.f};
        target->momentumScaleUp = {27.f};
        target->momentumScaleDown = {25.f};
        target->momentumReady = true;
    };

    MuonViewCollection muons(storage);
    require(muons.size() == 1 && populateCalls == 0,
            "muon cardinality must not compute the corrected momentum lane");
    const auto muon = muons[0];
    require(muon.Eta() == 1.2f && populateCalls == 0,
            "an unrelated muon field must not compute momentum corrections");
    require(muon.Pt() == 26.f && muon.MiniAODPt() == 25.f &&
                populateCalls == 1,
            "the first corrected momentum getter must publish the lane once");
    require(muon.MomentumScaleUp() == 27.f && populateCalls == 1,
            "subsequent momentum getters must reuse the demand lane");

    ++pt.currentEpoch;
    ++eta.currentEpoch;
    require(throws<SKNano::LogicError>([&] { static_cast<void>(muon.Pt()); }),
            "a cached momentum lane must still reject stale-event access");
}

void testJetDemandNominalLane() {
    FakeColumnSource<float> pt;
    FakeColumnSource<float> eta;
    pt.values = {100.f};
    eta.values = {0.4f};
    auto storage = std::make_shared<JetSoA>();
    storage->pt.bind(&pt);
    storage->eta.bind(&eta);
    int populateCalls = 0;
    storage->populateNominal = [target = storage.get(), &populateCalls] {
        ++populateCalls;
        target->jecFactor = {1.1f};
        target->correctedPt = {110.f};
        target->correctedMass = {11.f};
        target->smearedPtNominal = {108.f};
        target->smearedMassNominal = {10.8f};
    };

    JetViewCollection jets(storage);
    require(jets.size() == 1 && populateCalls == 0,
            "jet cardinality must not compute the nominal correction lane");
    const auto jet = jets[0];
    require(jet.Pt() == 100.f && jet.Eta() == 0.4f && populateCalls == 0,
            "raw jet fields must not compute JEC/JER");
    require(jet.CorrectedPt() == 110.f && jet.JECFactor() == 1.1f &&
                populateCalls == 1,
            "the first corrected jet getter must publish the nominal lane once");
    require(jet.SmearedPtNominal() == 108.f && populateCalls == 1,
            "subsequent corrected jet getters must reuse the demand lane");

    ++pt.currentEpoch;
    ++eta.currentEpoch;
    require(throws<SKNano::LogicError>(
                [&] { static_cast<void>(jet.CorrectedPt()); }),
            "a cached jet correction lane must reject stale-event access");
}

void testJetDemandJerVariationLane() {
    FakeColumnSource<float> pt;
    FakeColumnSource<float> eta;
    pt.values = {100.f};
    eta.values = {0.4f};
    auto storage = std::make_shared<JetSoA>();
    storage->pt.bind(&pt);
    storage->eta.bind(&eta);
    int nominalCalls = 0;
    int variationCalls = 0;
    storage->populateNominal = [target = storage.get(), &nominalCalls] {
        ++nominalCalls;
        target->correctedPt = {110.f};
        target->correctedMass = {11.f};
        target->smearedPtNominal = {108.f};
        target->smearedMassNominal = {10.8f};
    };
    storage->populateJerVariations =
        [target = storage.get(), &variationCalls] {
            ++variationCalls;
            target->smearedPtUp = {112.f};
            target->smearedPtDown = {104.f};
            target->smearedMassUp = {11.2f};
            target->smearedMassDown = {10.4f};
        };

    JetViewCollection jets(storage);
    const auto jet = jets[0];
    require(jet.SmearedPtNominal() == 108.f && nominalCalls == 1 &&
                variationCalls == 0,
            "nominal JER access must not materialize up/down lanes");
    require(jet.SmearedPtUp() == 112.f && nominalCalls == 1 &&
                variationCalls == 1,
            "the first JER variation access must materialize the lane once");
    require(jet.SmearedPtDown() == 104.f &&
                jet.SmearedMassUp() == 11.2f &&
                jet.SmearedMassDown() == 10.4f && variationCalls == 1,
            "all JER variation getters must reuse the materialized lanes");

    ++pt.currentEpoch;
    ++eta.currentEpoch;
    require(throws<SKNano::LogicError>(
                [&] { static_cast<void>(jet.SmearedPtUp()); }),
            "a cached JER variation lane must reject stale-event access");
}

void testSelectedJetProjection() {
    FakeColumnSource<float> pt;
    FakeColumnSource<float> eta;
    FakeColumnSource<float> phi;
    FakeColumnSource<float> mass;
    FakeColumnSource<float> deepFlavB;
    FakeColumnSource<float> pnetB;
    pt.values = {100.f, 80.f};
    eta.values = {0.4f, -1.2f};
    phi.values = {0.1f, -0.3f};
    mass.values = {10.f, 8.f};
    deepFlavB.values = {0.2f, 0.3f};
    pnetB.values = {0.7f, 0.8f};

    auto storage = std::make_shared<JetSoA>();
    storage->pt.bind(&pt);
    storage->eta.bind(&eta);
    storage->phi.bind(&phi);
    storage->mass.bind(&mass);
    storage->deepFlavB.bind(&deepFlavB);
    storage->pnetB.bind(&pnetB);
    int populateCalls = 0;
    storage->populateNominal = [target = storage.get(), &populateCalls] {
        ++populateCalls;
        target->jesPtUp = {115.f, 92.f};
        target->jesMassUp = {11.5f, 9.2f};
    };

    SelectedJetViewCollection jets(
        storage, {1, 0}, JetKinematicProjection::JesUp);
    require(jets.size() == 2 && populateCalls == 0,
            "selected jet construction and cardinality must remain lazy");
    require(jets.rawIndex(0) == 1 && jets[0].OriginalIndex() == 1,
            "a selected jet must preserve its raw input index");
    require(jets[0].Eta() == -1.2f && populateCalls == 0,
            "raw selected-jet fields must not compute projected kinematics");
    require(jets[0].GetTaggerResult(JetTagging::JetFlavTagger::ParticleNet,
                                    JetTagging::JetFlavTaggerScoreType::B) ==
                0.8f &&
                pnetB.getCalls == 1 && deepFlavB.getCalls == 0,
            "selected tag access must read only the requested score column");

    const auto projected = jets[0].P4();
    require(std::abs(projected.Pt() - 92.f) < 1.e-4f &&
                std::abs(projected.M() - 9.2f) < 1.e-4f &&
                populateCalls == 1,
            "the first projected four-vector access must populate corrections once");
    require(jets[1].Pt() == 115.f && populateCalls == 1,
            "all selected refs must share the populated correction lane");

    ++pt.currentEpoch;
    require(throws<SKNano::LogicError>([&] {
                static_cast<void>(jets[0].Pt());
            }),
            "a selected jet projection must reject stale-event access");
}

void testElectronIdentityRangeAndDemandRho() {
    FakeColumnSource<float> pt;
    FakeColumnSource<float> scEta;
    pt.values = {35.f, 40.f};
    scEta.values = {0.5f, 1.5f};
    auto storage = std::make_shared<ElectronSoA>();
    storage->pt.bind(&pt);
    storage->scEta.bind(&scEta);
    int rhoReads = 0;
    storage->readRho = [&rhoReads] {
        ++rhoReads;
        return 22.f;
    };

    ElectronViewCollection electrons(storage);
    require(pt.sizeCalls == 0 && scEta.getCalls == 0 && rhoReads == 0,
            "constructing the all-electron range must not read any branch");
    require(electrons.size() == 2 && rhoReads == 0,
            "electron cardinality must not read event rho");
    require(electrons[0].ScEta() == 0.5f && rhoReads == 0,
            "electron kinematics must not read rho");
    require(electrons[0].Rho() == 22.f && electrons[1].Rho() == 22.f &&
                rhoReads == 1,
            "the first rho getter must read and cache the event scalar once");

}

void testCanonicalBranchRegistryAndTreeTransition() {
    BranchManager manager;
    BranchScalar<Bool_t> generatedHLT("HLT_Test");
    manager.registerScalar(generatedHLT);

    auto &runtimeHLT = manager.getOrCreateScalar<Bool_t>("HLT_Test");
    require(&runtimeHLT == &generatedHLT,
            "runtime HLT resolution must reuse the generated canonical wrapper");
    require(throws<SKNano::ConfigError>([&] {
                static_cast<void>(manager.getOrCreateScalar<int>("HLT_Test"));
            }),
            "a conflicting scalar type must fail before a second wrapper is created");

    BranchScalar<int> firstTreeValue("schemaDependent");
    manager.registerScalar(firstTreeValue);

    int inputValue = 17;
    TTree firstTree("Events", "Events");
    firstTree.Branch("schemaDependent", &inputValue);
    firstTree.Fill();
    manager.attachTree(&firstTree);
    require(firstTreeValue.get(0) == 17,
            "the registered wrapper must read the value from the first tree");

    TTree secondTree("Events", "Events");
    secondTree.Fill();
    manager.attachTree(&secondTree);
    require(!firstTreeValue.valid(),
            "a branch absent from the next tree must be marked unavailable");
    require(throws<SKNano::ConfigError>([&] {
                static_cast<void>(firstTreeValue.get(0));
            }),
            "access to a missing branch after a tree transition must fail");
}

void testTypedColumnHandles() {
    BranchManager manager;
    Long64_t entry = 0;
    std::uint64_t epoch = 1;
    manager.bindEntrySource(&entry);
    manager.bindEpochSource(&epoch);
    std::unordered_set<std::string> activated;
    manager.setActivationCallback(
        [&](const std::string &name) { activated.insert(name); });

    int nJet = 2;
    float jetScore[4] = {0.25f, 0.75f, 0.f, 0.f};
    int htxsNjets = 3;
    TTree firstTree("Events", "Events");
    firstTree.Branch("nJet", &nJet, "nJet/I");
    firstTree.Branch("Jet_myScore", jetScore, "Jet_myScore[nJet]/F");
    firstTree.Branch("HTXS_njets30", &htxsNjets, "HTXS_njets30/I");
    firstTree.Fill();
    manager.attachTree(&firstTree);

    auto score = manager.getColumnHandle<float>(
        "Jet_myScore", ColumnRequirement::Required);
    auto scoreAgain = manager.getColumnHandle<float>(
        "Jet_myScore", ColumnRequirement::Required);
    require(score.source() == scoreAgain.source(),
            "duplicate typed vector resolution must reuse one canonical source");
    require(score.size() == 2 && score[0] == 0.25f && score[1] == 0.75f,
            "typed vector handle must expose current entry values");
    require(activated.count("Jet_myScore") == 1 && activated.count("nJet") == 1,
            "dynamic vector and count activation must reach the cache callback");

    auto scalar = manager.getScalarHandle<int>(
        "HTXS_njets30", ColumnRequirement::Required);
    require(scalar.get() == 3,
            "typed scalar handle must expose the current entry value");

    auto optional = manager.getColumnHandle<float>(
        "Jet_optionalScore", ColumnRequirement::Optional);
    require(!optional.available(),
            "missing optional vector handle must report unavailable");
    require(throws<SKNano::ConfigError>([&] {
                static_cast<void>(optional[0]);
            }),
            "unchecked optional vector access must fail explicitly");

    require(throws<SKNano::ConfigError>([&] {
                static_cast<void>(manager.getColumnHandle<int>(
                    "Jet_myScore", ColumnRequirement::Required));
            }),
            "typed handle with the wrong element type must fail");
    require(throws<SKNano::ConfigError>([&] {
                static_cast<void>(manager.getScalarHandle<float>(
                    "Jet_myScore", ColumnRequirement::Required));
            }),
            "scalar handle for a vector branch must fail");
    require(throws<SKNano::ConfigError>([&] {
                static_cast<void>(manager.getColumnHandle<int>(
                    "HTXS_njets30", ColumnRequirement::Required));
            }),
            "vector handle for a scalar branch must fail");
    BranchManager missingManager;
    missingManager.attachTree(&firstTree);
    require(throws<SKNano::ConfigError>([&] {
                static_cast<void>(missingManager.getScalarHandle<float>(
                    "requiredMissing", ColumnRequirement::Required));
            }),
            "missing required scalar handle must fail at resolution");

    int secondNJet = 1;
    float secondScore[2] = {0.5f, 0.f};
    int secondHtxsNjets = 1;
    TTree secondTree("Events", "Events");
    secondTree.Branch("nJet", &secondNJet, "nJet/I");
    secondTree.Branch("Jet_myScore", secondScore, "Jet_myScore[nJet]/F");
    secondTree.Branch("HTXS_njets30", &secondHtxsNjets, "HTXS_njets30/I");
    secondTree.Fill();
    ++epoch;
    manager.attachTree(&secondTree);
    require(score.size() == 1 && score[0] == 0.5f && scalar.get() == 1,
            "typed handles must rebind address and value after a tree transition");

    TTree missingTree("Events", "Events");
    missingTree.Fill();
    require(throws<SKNano::ConfigError>([&] {
                manager.attachTree(&missingTree);
            }),
            "tree transition must reject a missing required handle branch");

    BranchManager generatedManager;
    generatedManager.bindEntrySource(&entry);
    generatedManager.bindEpochSource(&epoch);
    BranchScalar<int> generatedCount("nJet");
    BranchVector<float, int> generatedScore("Jet_myScore", generatedCount);
    generatedManager.registerScalar(generatedCount);
    generatedManager.registerVector(generatedScore);
    generatedManager.attachTree(&firstTree);
    auto generatedHandle = generatedManager.getColumnHandle<float>(
        "Jet_myScore", ColumnRequirement::Required);
    require(generatedHandle.source() == &generatedScore,
            "runtime resolution must reuse a matching generated vector wrapper");

    BranchManager lateOptionalManager;
    lateOptionalManager.bindEntrySource(&entry);
    lateOptionalManager.bindEpochSource(&epoch);
    lateOptionalManager.attachTree(&missingTree);
    auto lateOptional = lateOptionalManager.getColumnHandle<float>(
        "Jet_myScore", ColumnRequirement::Optional);
    require(!lateOptional.available(),
            "optional handle may be created before a branch appears");
    lateOptionalManager.attachTree(&firstTree);
    require(lateOptional.available() && lateOptional.size() == 2 &&
                lateOptional[1] == 0.75f,
            "optional handle must resolve when a later tree provides the branch");

    BranchManager wrongCountManager;
    BranchScalar<unsigned int> wrongGeneratedCount("nJet");
    wrongCountManager.registerScalar(wrongGeneratedCount);
    require(throws<SKNano::ConfigError>([&] {
                wrongCountManager.attachTree(&firstTree);
            }),
            "a generated count wrapper with the wrong type must fail before reading");
}

} // namespace

int main() {
    try {
        testViewEpochGuards();
        testEventRangeContract();
        testFatJetDemandDrivenFields();
        testMuonDemandMomentumLane();
        testJetDemandNominalLane();
        testJetDemandJerVariationLane();
        testSelectedJetProjection();
        testElectronIdentityRangeAndDemandRho();
        testCanonicalBranchRegistryAndTreeTransition();
        testTypedColumnHandles();
    } catch (const std::exception &error) {
        std::cerr << "test_view_and_branch_lifetime: " << error.what() << '\n';
        return 1;
    }
    return 0;
}

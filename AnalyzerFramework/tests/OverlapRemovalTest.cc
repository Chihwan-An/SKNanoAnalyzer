#include <gtest/gtest.h>

#include <memory>
#include <vector>

#include <AnalyzerFramework/AnalyzerCore.h>

#include "TauTestSupport.h"

namespace {

using sknano_test::MakeTauCollection;
using sknano_test::MakeTaus;
using sknano_test::VectorColumn;

// Minimal jet fixture. JetView::Eta()/Phi() are plain column reads, so only
// the four kinematic columns need binding for overlap geometry.
struct JetFixture {
    VectorColumn<float> pt, eta, phi, mass;
    JetSoA soa;
};

std::shared_ptr<JetFixture> MakeJets(const std::vector<float> &etas,
                                     const std::vector<float> &phis) {
    auto f = std::make_shared<JetFixture>();
    const std::size_t n = etas.size();
    f->pt = VectorColumn<float>(std::vector<float>(n, 50.f));
    f->eta = VectorColumn<float>(etas);
    f->phi = VectorColumn<float>(phis);
    f->mass = VectorColumn<float>(std::vector<float>(n, 5.f));
    f->soa.pt.bind(&f->pt);
    f->soa.eta.bind(&f->eta);
    f->soa.phi.bind(&f->phi);
    f->soa.mass.bind(&f->mass);
    return f;
}

JetViewCollection MakeJetCollection(const std::shared_ptr<JetFixture> &f) {
    return JetViewCollection(std::shared_ptr<JetSoA>(f, &f->soa));
}

// One tau at (eta, phi) = (0.05, 0.05), used as the veto object.
std::shared_ptr<sknano_test::TauFixture> OneTauAt(float eta, float phi) {
    auto t = MakeTaus({100.f}, {eta}, {6}, {6}, {4});
    t->phi = VectorColumn<float>({phi});
    t->soa.phi.bind(&t->phi);
    return t;
}

TEST(RemoveOverlapIndices, DropsJetsInsideTheConeOfATau) {
    auto jets = MakeJets({0.0f, 1.0f}, {0.0f, 1.0f});
    auto taus = OneTauAt(0.05f, 0.05f);
    AnalyzerCore core;

    const auto result = core.RemoveOverlapIndices(
        MakeJetCollection(jets), std::vector<std::size_t>{0, 1},
        MakeTauCollection(taus), std::vector<std::size_t>{0}, 0.4f);

    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0], 1u);
}

TEST(RemoveOverlapIndices, KeepsEverythingWhenVetoListIsEmpty) {
    auto jets = MakeJets({0.0f, 1.0f}, {0.0f, 1.0f});
    auto taus = OneTauAt(0.0f, 0.0f);
    AnalyzerCore core;

    const auto result = core.RemoveOverlapIndices(
        MakeJetCollection(jets), std::vector<std::size_t>{0, 1},
        MakeTauCollection(taus), std::vector<std::size_t>{}, 0.4f);

    EXPECT_EQ(result.size(), 2u);
}

// Separation exactly equal to dRmin is not an overlap.
TEST(RemoveOverlapIndices, BoundaryDistanceIsKept) {
    auto jets = MakeJets({0.0f}, {0.4f});
    auto taus = OneTauAt(0.0f, 0.0f);
    AnalyzerCore core;

    const auto result = core.RemoveOverlapIndices(
        MakeJetCollection(jets), std::vector<std::size_t>{0},
        MakeTauCollection(taus), std::vector<std::size_t>{0}, 0.4f);

    EXPECT_EQ(result.size(), 1u);
}

// dPhi between +3.10 and -3.10 is about 0.083, an overlap, not 6.2.
TEST(RemoveOverlapIndices, HandlesPhiWrapAround) {
    auto jets = MakeJets({0.0f}, {3.10f});
    auto taus = OneTauAt(0.0f, -3.10f);
    AnalyzerCore core;

    const auto result = core.RemoveOverlapIndices(
        MakeJetCollection(jets), std::vector<std::size_t>{0},
        MakeTauCollection(taus), std::vector<std::size_t>{0}, 0.4f);

    EXPECT_TRUE(result.empty());
}

TEST(RemoveOverlapIndices, ConeSizeIsRespected) {
    auto jets = MakeJets({0.0f}, {0.6f});
    auto taus = OneTauAt(0.0f, 0.0f);
    AnalyzerCore core;

    EXPECT_EQ(core.RemoveOverlapIndices(
                  MakeJetCollection(jets), std::vector<std::size_t>{0},
                  MakeTauCollection(taus), std::vector<std::size_t>{0}, 0.4f)
                  .size(),
              1u);
    EXPECT_TRUE(core.RemoveOverlapIndices(
                    MakeJetCollection(jets), std::vector<std::size_t>{0},
                    MakeTauCollection(taus), std::vector<std::size_t>{0}, 0.8f)
                    .empty());
}

}
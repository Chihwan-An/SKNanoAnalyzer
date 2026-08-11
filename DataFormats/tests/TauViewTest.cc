#include <gtest/gtest.h>

#include "TauTestSupport.h"

namespace {

using sknano_test::MakeTaus;

TEST(TauViewWorkingPoints, TightTauPassesLooserJetWorkingPoints) {
    auto f = MakeTaus({100.f}, {0.5f}, {6}, {6}, {4});
    const TauView tau = f->at(0);

    EXPECT_TRUE(tau.passTIDvJet());
    EXPECT_TRUE(tau.passMIDvJet());
    EXPECT_TRUE(tau.passLIDvJet());
    EXPECT_TRUE(tau.passVLIDvJet());
    EXPECT_TRUE(tau.passVVLIDvJet());
    EXPECT_TRUE(tau.passVVVLIDvJet());
}

TEST(TauViewWorkingPoints, TightTauFailsTighterJetWorkingPoints) {
    auto f = MakeTaus({100.f}, {0.5f}, {6}, {6}, {4});
    const TauView tau = f->at(0);

    EXPECT_FALSE(tau.passVTIDvJet());
    EXPECT_FALSE(tau.passVVTIDvJet());
}

TEST(TauViewWorkingPoints, ElectronAxisIsAlsoCumulative) {
    auto f = MakeTaus({100.f}, {0.5f}, {6}, {5}, {4});
    const TauView tau = f->at(0);

    EXPECT_TRUE(tau.passMIDvEl());
    EXPECT_TRUE(tau.passLIDvEl());
    EXPECT_TRUE(tau.passVVVLIDvEl());
    EXPECT_FALSE(tau.passTIDvEl());
}

TEST(TauViewWorkingPoints, MuonAxisUsesFourPointScale) {
    auto f = MakeTaus({100.f}, {0.5f}, {6}, {6}, {3});
    const TauView tau = f->at(0);

    EXPECT_TRUE(tau.passMIDvMu());
    EXPECT_TRUE(tau.passLIDvMu());
    EXPECT_TRUE(tau.passVLIDvMu());
    EXPECT_FALSE(tau.passTIDvMu());
}

TEST(TauViewWorkingPoints, UnidentifiedTauFailsEveryWorkingPoint) {
    auto f = MakeTaus({100.f}, {0.5f}, {0}, {0}, {0});
    const TauView tau = f->at(0);

    EXPECT_FALSE(tau.passVVVLIDvJet());
    EXPECT_FALSE(tau.passVVVLIDvEl());
    EXPECT_FALSE(tau.passVLIDvMu());
}

// test raw discrimator 

TEST(TauViewRawScores, RawDiscriminatorsAreReadable) {
    auto f = MakeTaus({100.f}, {0.5f}, {6}, {6}, {4});
    f->rawVsJet = sknano_test::VectorColumn<float>({0.97f});
    f->rawVsE = sknano_test::VectorColumn<float>({0.88f});
    f->rawVsMu = sknano_test::VectorColumn<float>({0.51f});
    f->soa.rawDeepTau2018v2p5VSjet.bind(&f->rawVsJet);
    f->soa.rawDeepTau2018v2p5VSe.bind(&f->rawVsE);
    f->soa.rawDeepTau2018v2p5VSmu.bind(&f->rawVsMu);

    const TauView tau = f->at(0);
    EXPECT_FLOAT_EQ(tau.RawVsJet(), 0.97f);
    EXPECT_FLOAT_EQ(tau.RawVsE(), 0.88f);
    EXPECT_FLOAT_EQ(tau.RawVsMu(), 0.51f);
}

TEST(TauViewRawScores, UnboundRawDiscriminatorsReturnSentinel) {
    auto f = MakeTaus({100.f}, {0.5f}, {6}, {6}, {4});
    const TauView tau = f->at(0);

    EXPECT_FLOAT_EQ(tau.RawVsJet(), -1.f);
    EXPECT_FLOAT_EQ(tau.RawVsE(), -1.f);
    EXPECT_FLOAT_EQ(tau.RawVsMu(), -1.f);
}

TEST(TauViewID, AcceptsTauMeetingEveryAxis) {
    auto f = MakeTaus({100.f}, {0.5f}, {6}, {6}, {4});
    const TauView::ID id{TauWP::Medium, TauWP::VVLoose, TauWPvsMu::Loose,
                         true, 0.2f};
    EXPECT_TRUE(f->at(0).PassID(id));
}

TEST(TauViewID, RejectsTauFailingTheJetAxis) {
    auto f = MakeTaus({100.f}, {0.5f}, {4}, {6}, {4});
    const TauView::ID id{TauWP::Medium, TauWP::VVLoose, TauWPvsMu::Loose,
                         true, 0.2f};
    EXPECT_FALSE(f->at(0).PassID(id));
}

TEST(TauViewID, RejectsTauFailingTheMuonAxis) {
    auto f = MakeTaus({100.f}, {0.5f}, {6}, {6}, {1});
    const TauView::ID id{TauWP::Medium, TauWP::VVLoose, TauWPvsMu::Tight,
                         true, 0.2f};
    EXPECT_FALSE(f->at(0).PassID(id));
}

TEST(TauViewID, NoneWorkingPointDisablesThatAxis) {
    auto f = MakeTaus({100.f}, {0.5f}, {6}, {0}, {0});
    const TauView::ID id{TauWP::Tight, TauWP::None, TauWPvsMu::None,
                         true, 0.2f};
    EXPECT_TRUE(f->at(0).PassID(id));
}

TEST(TauViewID, RejectsTauFailingTheImpactParameterCut) {
    auto f = MakeTaus({100.f}, {0.5f}, {6}, {6}, {4}, {0.5f});
    const TauView::ID id{TauWP::Medium, TauWP::VVLoose, TauWPvsMu::Loose,
                         true, 0.2f};
    EXPECT_FALSE(f->at(0).PassID(id));
}

TEST(TauViewID, WorkingPointEnumMapsToPogStrings) {
    EXPECT_STREQ(ToCorrectionString(TauWP::Medium), "Medium");
    EXPECT_STREQ(ToCorrectionString(TauWP::VVLoose), "VVLoose");
    EXPECT_STREQ(ToCorrectionString(TauWP::VVTight), "VVTight");
    EXPECT_STREQ(ToCorrectionString(TauWPvsMu::Tight), "Tight");
    EXPECT_STREQ(ToCorrectionString(TauWPvsMu::VLoose), "VLoose");
}

}
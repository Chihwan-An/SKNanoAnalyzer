#include <gtest/gtest.h>

#include <cstdlib>
#include <exception>
#include <memory>
#include <string>

#include "MyCorrection.h"
#include "TauTestSupport.h"

namespace {

const TauView::TauID kTestID{LeptonID::TauWorkingPoint::MEDIUM,
                             LeptonID::TauWorkingPoint::TIGHT,
                             LeptonID::TauWorkingPointVsMu::TIGHT, true, 0.2f};

// MyCorrection opens every correction file the era declares, so it is built
// once for the whole suite rather than per test.
//
// Which files exist is decided by the era yml, not by this file, so the suite
// probes the object instead of guessing a path: a TAU set that is absent for
// this era surfaces as an exception on first use and skips the suite.
class TauCorrection : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        if (!std::getenv("JSONPOG_REPO_PATH") || !std::getenv("SKNANO_DATA") ||
            !std::getenv("ROCCOR_PATH")) {
            skip_ = "correction environment not set; source setup.sh first";
            return;
        }
        try {
            mc_ = std::make_unique<MyCorrection>("2024", "CDE", "TTLJ_powheg",
                                                 false);
            data_ = std::make_unique<MyCorrection>("2024", "CDE", "Muon", true);
            mc_->GetTauIDSF_vsJetRaw(kTestID, 100.f, 0, 5);
        } catch (const std::exception &e) {
            skip_ = std::string("tau corrections unavailable: ") + e.what();
            mc_.reset();
            data_.reset();
        }
    }

    static void TearDownTestSuite() {
        mc_.reset();
        data_.reset();
    }

    void SetUp() override {
        if (!mc_)
            GTEST_SKIP() << skip_;
    }

    static std::unique_ptr<MyCorrection> mc_;
    static std::unique_ptr<MyCorrection> data_;
    static std::string skip_;
};

std::unique_ptr<MyCorrection> TauCorrection::mc_;
std::unique_ptr<MyCorrection> TauCorrection::data_;
std::string TauCorrection::skip_;

TEST_F(TauCorrection, IdentificationScaleFactorIsPhysical) {
    const float sf = mc_->GetTauIDSF_vsJetRaw(kTestID, 100.f, 0, 5,
                                              MyCorrection::variation::nom);
    EXPECT_GT(sf, 0.5f);
    EXPECT_LT(sf, 1.5f);
}

TEST_F(TauCorrection, SystematicVariationsStraddleTheNominal) {
    const float nom = mc_->GetTauIDSF_vsJetRaw(kTestID, 100.f, 0, 5,
                                               MyCorrection::variation::nom);
    const float up = mc_->GetTauIDSF_vsJetRaw(kTestID, 100.f, 0, 5,
                                              MyCorrection::variation::up);
    const float down = mc_->GetTauIDSF_vsJetRaw(kTestID, 100.f, 0, 5,
                                                MyCorrection::variation::down);
    EXPECT_GE(up, nom);
    EXPECT_LE(down, nom);
    EXPECT_GT(up, down);
}

TEST_F(TauCorrection, ElectronAndMuonAxesReturnPhysicalScaleFactors) {
    const float vsE = mc_->GetTauIDSF_vsERaw(kTestID, 0.5f, 0, 5,
                                             MyCorrection::variation::nom);
    const float vsMu = mc_->GetTauIDSF_vsMuRaw(kTestID, 0.5f, 5,
                                               MyCorrection::variation::nom);
    EXPECT_GT(vsE, 0.5f);
    EXPECT_LT(vsE, 1.5f);
    EXPECT_GT(vsMu, 0.5f);
    EXPECT_LT(vsMu, 1.5f);
}

TEST_F(TauCorrection, DataReturnsUnity) {
    EXPECT_FLOAT_EQ(data_->GetTauIDSF_vsJetRaw(kTestID, 100.f, 0, 5,
                                               MyCorrection::variation::nom),
                    1.f);
}

// The collection forms multiply over the selection, so an index that does not
// address a tau must be reported rather than silently weighting the event with
// whatever sits at that slot.
TEST_F(TauCorrection, CollectionFormRejectsOutOfRangeIndices) {
    const auto fixture =
        sknano_test::MakeTaus({100.f}, {0.5f}, {6}, {6}, {4});
    const auto taus = sknano_test::MakeTauCollection(fixture);

    EXPECT_NO_THROW(mc_->GetTauIDSF_vsJet(kTestID, taus, {0}));
    EXPECT_THROW(mc_->GetTauIDSF_vsJet(kTestID, taus, {1}), SKNano::LogicError);
}

} // namespace

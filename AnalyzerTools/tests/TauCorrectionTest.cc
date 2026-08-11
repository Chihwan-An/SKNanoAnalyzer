#include <gtest/gtest.h>

#include <cstdlib>
#include <string>
#include <sys/stat.h>

#include "MyCorrection.h"

namespace {

bool TauJsonAvailable() {
    const char *repo = std::getenv("JSONPOG_REPO_PATH");
    if (!repo)
        return false;
    const std::string path =
        std::string(repo) +
        "/TAU/Run3-24CDEReprocessingFGHIPrompt-Summer24-NanoAODv15/latest/"
        "tau.json.gz";
    struct stat sb;
    return stat(path.c_str(), &sb) == 0;
}

const TauView::ID kTestID{TauWP::Medium, TauWP::Tight, TauWPvsMu::Tight, true,
                          0.2f};

TEST(TauCorrection, IdentificationScaleFactorIsPhysical) {
    if (!TauJsonAvailable())
        GTEST_SKIP() << "tau.json.gz not available";

    MyCorrection corr("2024", "CDE", "TTLJ_powheg", false);

    const float sf = corr.GetTauIDSF_vsJetRaw(kTestID, 100.f, 0, 5,
                                              MyCorrection::variation::nom);
    EXPECT_GT(sf, 0.5f);
    EXPECT_LT(sf, 1.5f);
}

TEST(TauCorrection, SystematicVariationsStraddleTheNominal) {
    if (!TauJsonAvailable())
        GTEST_SKIP() << "tau.json.gz not available";

    MyCorrection corr("2024", "CDE", "TTLJ_powheg", false);

    const float nom = corr.GetTauIDSF_vsJetRaw(kTestID, 100.f, 0, 5,
                                               MyCorrection::variation::nom);
    const float up = corr.GetTauIDSF_vsJetRaw(kTestID, 100.f, 0, 5,
                                              MyCorrection::variation::up);
    const float down = corr.GetTauIDSF_vsJetRaw(kTestID, 100.f, 0, 5,
                                                MyCorrection::variation::down);
    EXPECT_GE(up, nom);
    EXPECT_LE(down, nom);
    EXPECT_GT(up, down);
}

TEST(TauCorrection, ElectronAndMuonAxesReturnPhysicalScaleFactors) {
    if (!TauJsonAvailable())
        GTEST_SKIP() << "tau.json.gz not available";

    MyCorrection corr("2024", "CDE", "TTLJ_powheg", false);

    const float vsE = corr.GetTauIDSF_vsERaw(kTestID, 0.5f, 0, 5,
                                             MyCorrection::variation::nom);
    const float vsMu = corr.GetTauIDSF_vsMuRaw(kTestID, 0.5f, 5,
                                               MyCorrection::variation::nom);
    EXPECT_GT(vsE, 0.5f);
    EXPECT_LT(vsE, 1.5f);
    EXPECT_GT(vsMu, 0.5f);
    EXPECT_LT(vsMu, 1.5f);
}

TEST(TauCorrection, DataReturnsUnity) {
    if (!TauJsonAvailable())
        GTEST_SKIP() << "tau.json.gz not available";

    MyCorrection corr("2024", "CDE", "Muon", true);
    EXPECT_FLOAT_EQ(corr.GetTauIDSF_vsJetRaw(kTestID, 100.f, 0, 5,
                                             MyCorrection::variation::nom),
                    1.f);
}

}
#include <cmath>
#include <cstdint>
#include <exception>
#include <iostream>
#include <stdexcept>

#include "AnalyzerCore.h"

namespace {

void require(bool condition, const char *message) {
    if (!condition)
        throw std::runtime_error(message);
}

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

void testDataEventDoesNotRequirePileupBranches() {
    AnalyzerCore analyzer;
    analyzer.IsDATA = true;
    analyzer.SetEra("2024");
    analyzer.branchManager.bindEntrySource(&analyzer.currentLocalEntry);
    analyzer.branchManager.bindEpochSource(&analyzer.eventEpoch);
    analyzer.RegisterBranches();

    UInt_t run = 380567;
    UInt_t lumi = 659;
    ULong64_t event = 1079959517;
    UChar_t goodVertices = 25;
    float metPt = 42.f, metPhi = .5f;
    float metPtUp = 43.f, metPtDown = 41.f;
    float metPhiUp = .51f, metPhiDown = .49f;
    float rho = 18.f;
    TTree tree("Events", "Events");
    tree.Branch("run", &run);
    tree.Branch("luminosityBlock", &lumi);
    tree.Branch("event", &event);
    tree.Branch("PV_npvsGood", &goodVertices);
    tree.Branch("PuppiMET_pt", &metPt);
    tree.Branch("PuppiMET_phi", &metPhi);
    tree.Branch("PuppiMET_ptUnclusteredUp", &metPtUp);
    tree.Branch("PuppiMET_ptUnclusteredDown", &metPtDown);
    tree.Branch("PuppiMET_phiUnclusteredUp", &metPhiUp);
    tree.Branch("PuppiMET_phiUnclusteredDown", &metPhiDown);
    tree.Branch("Rho_fixedGridRhoFastjetAll", &rho);
    tree.Fill();
    tree.SetBranchStatus("*", 0);

    analyzer.currentLocalEntry = 0;
    analyzer.currentEntry = 0;
    analyzer.eventEpoch = 1;
    analyzer.branchManager.attachTree(&tree);
    const Event result = analyzer.GetEvent();
    require(result.run() == static_cast<int>(run) &&
                result.nPVsGood() == goodVertices &&
                result.nTrueInt() == -999.f,
            "data Event must not activate absent Pileup branches");
}

} // namespace

int main() {
    try {
        testDataEventDoesNotRequirePileupBranches();
        SKNanoLoader loader;
        loader.branchManager.bindEntrySource(&loader.currentLocalEntry);
        loader.branchManager.bindEpochSource(&loader.eventEpoch);
        loader.RegisterBranches();

        int nJet = 2;
        float pt[4] = {45.f, 80.f, 0.f, 0.f};
        float eta[4] = {0.2f, -1.3f, 0.f, 0.f};
        float phi[4] = {1.f, -2.f, 0.f, 0.f};
        float mass[4] = {8.f, 12.f, 0.f, 0.f};
        TTree tree("Events", "Events");
        tree.Branch("nJet", &nJet, "nJet/I");
        tree.Branch("Jet_pt", pt, "Jet_pt[nJet]/F");
        tree.Branch("Jet_eta", eta, "Jet_eta[nJet]/F");
        tree.Branch("Jet_phi", phi, "Jet_phi[nJet]/F");
        tree.Branch("Jet_mass", mass, "Jet_mass[nJet]/F");
        tree.Fill();
        tree.SetBranchStatus("*", 0);

        loader.currentLocalEntry = 0;
        loader.eventEpoch = 1;
        loader.branchManager.attachTree(&tree);
        auto columns = loader.GetJetInputColumns();
        require(columns.ptRequired && columns.ptAvailable(),
                "required generated Jet_pt metadata must match availability");
        require(!columns.btagDeepFlavBRequired &&
                    !columns.btagDeepFlavBAvailable(),
                "missing optional generated score must remain explicitly unavailable");
        const auto ptSnapshot = columns.pt.snapshot();
        require(ptSnapshot.size() == 2 && std::abs(ptSnapshot[1] - 80.f) < 1e-6f,
                "generated input composition must bind the canonical branch buffer");

        ++loader.eventEpoch;
        bool staleFailed = false;
        try {
            static_cast<void>(ptSnapshot.size());
        } catch (const std::logic_error &) {
            staleFailed = true;
        }
        require(staleFailed,
                "generated input snapshot must retain the event epoch guard");

    } catch (const std::exception &error) {
        std::cerr << "test_generated_input_columns: " << error.what() << '\n';
        return 1;
    }
    return 0;
}

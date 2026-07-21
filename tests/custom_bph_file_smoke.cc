#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

#include "SKNanoLoader.h"

namespace {

class CustomBPHSmoke final : public SKNanoLoader {
public:
    long long d0Count = 0;
    long long dstarCount = 0;
    long long genJetCount = 0;
    long long genHadronCount = 0;
    long long hadronTrackCount = 0;
    long long lambdaCount = 0;
    long long lambdaCCount = 0;
    double checksum = 0.;

    void executeEvent() override {
        auto d0 = GetAllD0ToKPiViews();
        auto dstar = GetAllDstarToD0PiViews();
        auto genJet = GetAllGenJetBPHViews();
        auto genHadron = GetAllGenJetHadronViews();
        auto hadronTrack = GetAllHadronTrackViews();
        auto lambda = GetAllLambdaViews();
        auto lambdaC = GetAllLambdaCToPKPiViews();
        if (!d0.available() || !dstar.available() || !genJet.available() ||
            !genHadron.available() || !hadronTrack.available() || !lambda.available() ||
            !lambdaC.available()) {
            throw std::runtime_error(
                "a CustomBPH collection is unavailable:" +
                std::string(d0.available() ? "" : " D0ToKPi") +
                std::string(dstar.available() ? "" : " DstarToD0Pi") +
                std::string(genJet.available() ? "" : " GenJet") +
                std::string(genHadron.available() ? "" : " GenJetHadron") +
                std::string(hadronTrack.available() ? "" : " HadronTrack") +
                std::string(lambda.available() ? "" : " Lambda") +
                std::string(lambdaC.available() ? "" : " LambdaCToPKPi") +
                " [manager/count/pt=" +
                std::to_string(branchManager.available("nD0ToKPi")) + "/" +
                std::to_string(nD0ToKPi.valid()) + "/" +
                std::to_string(D0ToKPi_pt.valid()) + "]");
        }

        d0Count += d0.size();
        dstarCount += dstar.size();
        genJetCount += genJet.size();
        genHadronCount += genHadron.size();
        hadronTrackCount += hadronTrack.size();
        lambdaCount += lambda.size();
        lambdaCCount += lambdaC.size();
        if (!d0.empty()) checksum += d0[0].pt();
        if (!dstar.empty()) checksum += dstar[0].pt();
        if (!genJet.empty()) checksum += genJet[0].nD0();
        if (!genHadron.empty()) checksum += genHadron[0].pt();
        if (!hadronTrack.empty()) checksum += hadronTrack[0].pt();
        if (!lambda.empty()) checksum += lambda[0].pt();
        if (!lambdaC.empty()) checksum += lambdaC[0].pt();
    }
};

} // namespace

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "usage: custom_bph_file_smoke INPUT.root [INPUT.root ...]\n";
        return 2;
    }

    try {
        CustomBPHSmoke analyzer;
        analyzer.SetInputFormat("auto");
        analyzer.SetTreeName("Events");
        for (int i = 1; i < argc; ++i) {
            if (analyzer.AddFile(argv[i]) != 1)
                throw std::runtime_error("failed to add custom NanoAOD file");
        }
        analyzer.SetEra("2024");
        analyzer.MaxEvent = 10000;
        if (const char *limit = std::getenv("SKNANO_SMOKE_MAX_EVENTS"))
            analyzer.MaxEvent = std::stol(limit);
        analyzer.Init();
        analyzer.Loop();

        const auto &active = analyzer.branchManager.getActiveBranches();
        if (active.count("D0ToKPi_charge") != 0)
            throw std::runtime_error("unused D0ToKPi_charge was activated");
        if (active.count("nD0ToKPi") == 0 ||
            active.count("D0ToKPi_pt") == 0)
            throw std::runtime_error(
                "selected D0ToKPi lazy branches were not activated");

        std::cout << "CUSTOM_BPH_SMOKE_OK"
                  << " d0=" << analyzer.d0Count
                  << " dstar=" << analyzer.dstarCount
                  << " genJet=" << analyzer.genJetCount
                  << " genHadron=" << analyzer.genHadronCount
                  << " hadronTrack=" << analyzer.hadronTrackCount
                  << " lambda=" << analyzer.lambdaCount
                  << " lambdaC=" << analyzer.lambdaCCount
                  << " active=" << active.size()
                  << " checksum=" << analyzer.checksum << '\n';
    } catch (const std::exception &error) {
        std::cerr << "custom_bph_file_smoke: " << error.what() << '\n';
        return 1;
    }
    return 0;
}

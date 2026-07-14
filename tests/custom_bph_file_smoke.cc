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
    long long lambdaCount = 0;
    long long lambdaCCount = 0;
    double checksum = 0.;

    void executeEvent() override {
        auto d0 = GetAllD0ToKPiViews();
        auto dstar = GetAllDstarToD0PiViews();
        auto genJet = GetAllGenJetBPHViews();
        auto genHadron = GetAllGenJetHadronViews();
        auto lambda = GetAllLambdaViews();
        auto lambdaC = GetAllLambdaCToPKPiViews();
        if (!d0.available() || !dstar.available() || !genJet.available() ||
            !genHadron.available() || !lambda.available() ||
            !lambdaC.available())
            throw std::runtime_error("a CustomBPH collection is unavailable");

        d0Count += d0.size();
        dstarCount += dstar.size();
        genJetCount += genJet.size();
        genHadronCount += genHadron.size();
        lambdaCount += lambda.size();
        lambdaCCount += lambdaC.size();
        if (!d0.empty()) checksum += d0[0].pt();
        if (!dstar.empty()) checksum += dstar[0].pt();
        if (!genJet.empty()) checksum += genJet[0].nD0();
        if (!genHadron.empty()) checksum += genHadron[0].pt();
        if (!lambda.empty()) checksum += lambda[0].pt();
        if (!lambdaC.empty()) checksum += lambdaC[0].pt();
    }
};

} // namespace

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "usage: custom_bph_file_smoke INPUT.root\n";
        return 2;
    }

    try {
        CustomBPHSmoke analyzer;
        analyzer.SetTreeName("Events");
        if (analyzer.AddFile(argv[1]) != 1)
            throw std::runtime_error("failed to add custom NanoAOD file");
        analyzer.SetEra("2024");
        analyzer.MaxEvent = 10000;
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

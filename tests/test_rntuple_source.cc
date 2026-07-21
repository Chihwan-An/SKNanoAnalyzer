#include "BranchManager.h"
#include "RNTupleSource.h"

#include <cmath>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "usage: test_rntuple_source INPUT.rntuple.root\n";
        return 2;
    }

    try {
        SKNano::RNTupleSource source;
        source.open("Events", argv[1], true, true);
        if (source.entries() == 0 || !source.hasField("Jet_pt"))
            throw std::runtime_error("missing expected NanoAOD jet data");

        Long64_t entry = -1;
        std::uint64_t epoch = 0;
        BranchManager manager;
        manager.bindEntrySource(&entry);
        manager.bindEpochSource(&epoch);

        BranchScalar<unsigned int> run{"run"};
        BranchScalar<int> nJet{"nJet"};
        BranchVector<float, int> jetPt{"Jet_pt", nJet};
        manager.registerScalar(run);
        manager.registerScalar(nJet);
        manager.registerVector(jetPt);
        manager.attachRNTuple(&source);

        double checksum = 0.;
        std::size_t jets = 0;
        const auto limit = std::min<std::uint64_t>(source.entries(), 1000);
        for (std::uint64_t i = 0; i < limit; ++i) {
            entry = static_cast<Long64_t>(i);
            ++epoch;
            const auto pt = jetPt.snapshot();
            const auto count = static_cast<std::size_t>(nJet.get());
            if (pt.size() != count)
                throw std::runtime_error("cardinality/vector size mismatch");
            jets += pt.size();
            if (!pt.empty())
                checksum += pt[0];
            checksum += run.get() * 1.e-12;
        }
        if (!std::isfinite(checksum))
            throw std::runtime_error("non-finite checksum");
        std::cout << "RNTUPLE_SOURCE_OK entries=" << limit
                  << " jets=" << jets
                  << " active=" << manager.getActiveBranches().size()
                  << " checksum=" << checksum << '\n';
    } catch (const std::exception &error) {
        std::cerr << "test_rntuple_source: " << error.what() << '\n';
        return 1;
    }
    return 0;
}

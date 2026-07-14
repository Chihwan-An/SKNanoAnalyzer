#include <array>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "AnalyzerCore.h"

namespace {

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

} // namespace

int main() {
  try {
    AnalyzerCore analyzer;
    unsigned int run = 1;
    unsigned long long event = 1ULL << 40;
    std::array<float, 4> jets{100.f, 80.f, 60.f, 40.f};
    std::vector<int> hadrons{511, 421};

    auto tree = analyzer.BookTree("Events");
    tree.Branch("run", run)
        .Branch("event", event)
        .Branch("Jet_pt", jets)
        .Branch("Hadron_pdgId", hadrons);
    tree.Fill();

    run = 2;
    event += 1;
    jets[0] = 120.f;
    hadrons = {4122};
    tree.Fill();

    require(tree.get()->GetEntries() == 2,
            "TreeHandle::Fill must write every event");
    require(std::string(tree.get()->GetLeaf("run")->GetTypeName()) ==
                "UInt_t",
            "unsigned scalar ROOT type mismatch");
    require(std::string(tree.get()->GetLeaf("event")->GetTypeName()) ==
                "ULong64_t",
            "64-bit event ROOT type mismatch");
    require(tree.get()->GetLeaf("Jet_pt")->GetLenStatic() == 4,
            "std::array branch length mismatch");
    require(tree.get()->GetBranch("Hadron_pdgId") != nullptr,
            "std::vector branch was not created");

    tree.Branch("run", run);
    unsigned int anotherRun = 3;
    require(throws<SKNano::ConfigError>(
                [&] { tree.Branch("run", anotherRun); }),
            "same branch with another address must fail");
    std::vector<float> wrongType;
    require(throws<SKNano::ConfigError>(
                [&] { tree.Branch("Hadron_pdgId", wrongType); }),
            "same vector branch with another type must fail");

    const auto group = analyzer.Hists("Analysis/Jet0");
    const auto multiplicity = group.Book1D("Multiplicity", 10, 0., 10.);
    multiplicity.Fill(2., 1.5);
    require(analyzer.GetHist1D("Analysis/Jet0/Multiplicity")->GetEntries() ==
                1.,
            "HistogramGroup prefix or fill mismatch");

    analyzer.BookTree("Collision");
    require(throws<SKNano::ConfigError>([&] {
              analyzer.Hists("Collision").Book1D("Nested", 2, 0., 2.);
            }),
            "tree/histogram output path collision must fail");
  } catch (const std::exception &error) {
    std::cerr << "test_output_handles: " << error.what() << '\n';
    return 1;
  }
  return 0;
}

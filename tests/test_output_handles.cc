#include <array>
#include <exception>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <ROOT/RNTupleReader.hxx>
#include <TFile.h>
#include "AnalyzerCore.h"

namespace {

class SkimProbe : public AnalyzerCore {
public:
  using AnalyzerCore::SnapshotSelectedInput;
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

} // namespace

int main() {
  try {
    AnalyzerCore analyzer;
    const auto defaultOutputPath =
        std::filesystem::temp_directory_path() /
        "sknano_test_default_output_rntuple.root";
    std::filesystem::remove(defaultOutputPath);
    std::filesystem::remove(defaultOutputPath.string() + ".partial");
    analyzer.SetOutfilePath(defaultOutputPath.string());
    unsigned int run = 1;
    unsigned long long event = 1ULL << 40;
    std::array<float, 4> jets{100.f, 80.f, 60.f, 40.f};
    std::vector<int> hadrons{511, 421};

    auto tree = analyzer.Output().Book("Events");
    auto weight = tree.MakeField<float>("weight");
    tree.Field("run", run)
        .Field("event", event)
        .Field("Jet_pt", jets)
        .Field("Hadron_pdgId", hadrons);
    tree.Fill();

    run = 2;
    event += 1;
    jets[0] = 120.f;
    hadrons = {4122};
    weight = 2.5f;
    tree.Fill();

    require(tree.GetEntries() == 2,
            "RNTupleHandle::Fill must write every event");

    tree.Field("run", run);
    unsigned int anotherRun = 3;
    require(throws<SKNano::ConfigError>(
                [&] { tree.Field("run", anotherRun); }),
            "same branch with another address must fail");
    std::vector<float> wrongType;
    require(throws<SKNano::ConfigError>(
                [&] { tree.Field("Hadron_pdgId", wrongType); }),
            "same vector branch with another type must fail");

    const auto group = analyzer.Hists("Analysis/Jet0");
    const auto multiplicity = group.Book1D("Multiplicity", 10, 0., 10.);
    multiplicity.Fill(2., 1.5);
    require(analyzer.GetHist1D("Analysis/Jet0/Multiplicity")->GetEntries() ==
                1.,
            "HistogramGroup prefix or fill mismatch");

    require(throws<SKNano::ConfigError>([&] {
              analyzer.Hists("Events").Book1D("Nested", 2, 0., 2.);
            }),
            "tree/histogram output path collision must fail");
    analyzer.WriteHist();
    require(std::filesystem::exists(defaultOutputPath),
            "final output was not atomically published");
    require(!std::filesystem::exists(defaultOutputPath.string() + ".partial"),
            "partial output remained after successful publish");
    {
      auto reader =
          ROOT::RNTupleReader::Open("Events", defaultOutputPath.string());
      require(reader->GetNEntries() == 2,
              "default RNTuple entry count mismatch");
      auto eventView = reader->GetView<unsigned long long>("event");
      auto jetView = reader->GetView<std::array<float, 4>>("Jet_pt");
      require(eventView(1) == (1ULL << 40) + 1 && jetView(1)[0] == 120.f,
              "default RNTuple values mismatch");
      auto weightView = reader->GetView<float>("weight");
      require(weightView(1) == 2.5f,
              "owned RNTuple output field value mismatch");
    }
    std::filesystem::remove(defaultOutputPath);

    const auto outputPath =
        std::filesystem::temp_directory_path() /
        "sknano_test_output_handles_rntuple.root";
    std::filesystem::remove(outputPath);
    {
      AnalyzerCore outputAnalyzer;
      outputAnalyzer.SetOutfilePath(outputPath.string());
      outputAnalyzer.Output().Book("UnusedCategory",
                                   AnalyzerCore::RNTupleOutputProfile::Sparse);
      unsigned int outputRun = 10;
      std::array<float, 2> outputJetPt{100.f, 80.f};
      std::vector<int> outputHadrons{511, 421};
      auto ntuple = outputAnalyzer.Output().Book("BPHEvents");
      ntuple.Field("run", outputRun)
          .Field("Jet_pt", outputJetPt)
          .Field("Hadron_pdgId", outputHadrons);
      ntuple.Fill();
      outputRun = 11;
      outputHadrons = {4122};
      ntuple.Fill();
      require(ntuple.GetEntries() == 2,
              "RNTupleHandle::Fill must write every event");
      unsigned int otherOutputRun = 12;
      require(throws<SKNano::ConfigError>(
                  [&] { ntuple.Field("run", otherOutputRun); }),
              "same RNTuple field with another address must fail");
      outputAnalyzer.WriteHist();
    }
    {
      auto reader = ROOT::RNTupleReader::Open("BPHEvents", outputPath.string());
      require(reader->GetNEntries() == 2,
              "persisted RNTuple entry count mismatch");
      auto runView = reader->GetView<unsigned int>("run");
      auto jetPtView = reader->GetView<std::array<float, 2>>("Jet_pt");
      auto hadronView = reader->GetView<std::vector<int>>("Hadron_pdgId");
      require(runView(0) == 10 && runView(1) == 11,
              "persisted RNTuple scalar values mismatch");
      require(jetPtView(0)[0] == 100.f && jetPtView(0)[1] == 80.f,
              "persisted RNTuple fixed-array values mismatch");
      require(hadronView(1).size() == 1 && hadronView(1)[0] == 4122,
              "persisted RNTuple vector values mismatch");
    }
    std::filesystem::remove(outputPath);

    const auto skimInputPath =
        std::filesystem::temp_directory_path() /
        "sknano_test_rntuple_skim_input.root";
    const auto skimOutputPath =
        std::filesystem::temp_directory_path() /
        "sknano_test_rntuple_skim_output.root";
    std::filesystem::remove(skimInputPath);
    std::filesystem::remove(skimOutputPath);
    {
      AnalyzerCore inputWriter;
      inputWriter.SetOutfilePath(skimInputPath.string());
      auto input = inputWriter.Output().Book("Events");
      auto run = input.MakeField<unsigned int>("run");
      auto values = input.MakeField<std::vector<float>>("values");
      for (unsigned int index = 0; index < 3; ++index) {
        run = 100 + index;
        values = std::vector<float>{static_cast<float>(index), 2.f};
        input.Fill();
      }
      inputWriter.WriteHist();
    }
    {
      SkimProbe skim;
      skim.SetTreeName("Events");
      require(skim.AddFile(skimInputPath.string()) == 1,
              "RNTuple skim input could not be added");
      skim.SetOutfilePath(skimOutputPath.string());
      skim.SnapshotSelectedInput({2, 0, 2});
      skim.WriteHist();
    }
    {
      auto reader = ROOT::RNTupleReader::Open("Events", skimOutputPath.string());
      require(reader->GetNEntries() == 2,
              "RNTuple skim selection or deduplication mismatch");
      auto run = reader->GetView<unsigned int>("run");
      require(run(0) == 100 && run(1) == 102,
              "RNTuple skim did not preserve selected input rows");
    }
    std::filesystem::remove(skimInputPath);
    std::filesystem::remove(skimOutputPath);
  } catch (const std::exception &error) {
    std::cerr << "test_output_handles: " << error.what() << '\n';
    return 1;
  }
  return 0;
}

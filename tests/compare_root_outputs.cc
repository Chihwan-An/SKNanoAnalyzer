#include <TArrayD.h>
#include <TAxis.h>
#include <TBranch.h>
#include <TDirectory.h>
#include <TFile.h>
#include <TH1.h>
#include <TKey.h>
#include <TTree.h>

#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

bool sameDouble(double lhs, double rhs) {
  std::uint64_t lhsBits = 0;
  std::uint64_t rhsBits = 0;
  std::memcpy(&lhsBits, &lhs, sizeof(lhs));
  std::memcpy(&rhsBits, &rhs, sizeof(rhs));
  return lhsBits == rhsBits;
}

void require(bool condition, const std::string &message) {
  if (!condition)
    throw std::runtime_error(message);
}

void compareAxis(const TAxis &lhs, const TAxis &rhs,
                 const std::string &path) {
  require(lhs.GetNbins() == rhs.GetNbins(), path + ": axis bin mismatch");
  require(sameDouble(lhs.GetXmin(), rhs.GetXmin()) &&
              sameDouble(lhs.GetXmax(), rhs.GetXmax()),
          path + ": axis range mismatch");
  const auto *lhsBins = lhs.GetXbins();
  const auto *rhsBins = rhs.GetXbins();
  require(lhsBins->GetSize() == rhsBins->GetSize(),
          path + ": variable-axis size mismatch");
  for (int i = 0; i < lhsBins->GetSize(); ++i)
    require(sameDouble(lhsBins->At(i), rhsBins->At(i)),
            path + ": variable-axis edge mismatch");
  for (int i = 1; i <= lhs.GetNbins(); ++i)
    require(std::string(lhs.GetBinLabel(i)) == rhs.GetBinLabel(i),
            path + ": axis-label mismatch");
}

void compareHistogram(const TH1 &lhs, const TH1 &rhs,
                      const std::string &path) {
  require(lhs.GetDimension() == rhs.GetDimension(),
          path + ": histogram dimension mismatch");
  require(lhs.GetNcells() == rhs.GetNcells(),
          path + ": histogram cell-count mismatch");
  require(sameDouble(lhs.GetEntries(), rhs.GetEntries()),
          path + ": histogram entry-count mismatch");
  compareAxis(*lhs.GetXaxis(), *rhs.GetXaxis(), path + "/x");
  if (lhs.GetDimension() >= 2)
    compareAxis(*lhs.GetYaxis(), *rhs.GetYaxis(), path + "/y");
  if (lhs.GetDimension() >= 3)
    compareAxis(*lhs.GetZaxis(), *rhs.GetZaxis(), path + "/z");
  for (int bin = 0; bin < lhs.GetNcells(); ++bin) {
    require(sameDouble(lhs.GetBinContent(bin), rhs.GetBinContent(bin)),
            path + ": histogram content mismatch at cell " +
                std::to_string(bin));
    require(sameDouble(lhs.GetBinError(bin), rhs.GetBinError(bin)),
            path + ": histogram error mismatch at cell " +
                std::to_string(bin));
  }
}

std::vector<std::string> branchNames(TTree &tree) {
  std::vector<std::string> result;
  const auto *branches = tree.GetListOfBranches();
  result.reserve(branches->GetEntries());
  for (int i = 0; i < branches->GetEntries(); ++i)
    result.emplace_back(branches->At(i)->GetName());
  return result;
}

void compareTree(TTree &lhs, TTree &rhs, const std::string &path) {
  require(lhs.GetEntries() == rhs.GetEntries(), path + ": tree entry mismatch");
  const auto lhsBranches = branchNames(lhs);
  const auto rhsBranches = branchNames(rhs);
  require(lhsBranches == rhsBranches, path + ": tree branch mismatch");

  constexpr Long64_t kEstimate = 10'000'000;
  lhs.SetEstimate(kEstimate);
  rhs.SetEstimate(kEstimate);
  for (const auto &branch : lhsBranches) {
    const auto lhsRows = lhs.Draw(branch.c_str(), "", "goff");
    require(lhsRows >= 0, path + ": failed to read branch " + branch);
    std::vector<double> lhsValues(lhs.GetV1(), lhs.GetV1() + lhsRows);
    const auto rhsRows = rhs.Draw(branch.c_str(), "", "goff");
    require(rhsRows == lhsRows,
            path + ": flattened value-count mismatch for " + branch);
    const auto *rhsValues = rhs.GetV1();
    for (Long64_t i = 0; i < lhsRows; ++i)
      require(sameDouble(lhsValues[static_cast<std::size_t>(i)], rhsValues[i]),
              path + ": value mismatch for " + branch + " at " +
                  std::to_string(i));
  }
}

std::map<std::string, TKey *> currentKeys(TDirectory &directory) {
  std::map<std::string, TKey *> result;
  TIter next(directory.GetListOfKeys());
  while (auto *key = dynamic_cast<TKey *>(next())) {
    const auto found = result.find(key->GetName());
    if (found == result.end() || found->second->GetCycle() < key->GetCycle())
      result[key->GetName()] = key;
  }
  return result;
}

struct Counts {
  std::size_t directories = 0;
  std::size_t trees = 0;
  std::size_t branches = 0;
  std::size_t histograms = 0;
};

void compareDirectory(TDirectory &lhs, TDirectory &rhs,
                      const std::string &path, Counts &counts) {
  const auto lhsKeys = currentKeys(lhs);
  const auto rhsKeys = currentKeys(rhs);
  require(lhsKeys.size() == rhsKeys.size(), path + ": object-count mismatch");

  for (const auto &[name, lhsKey] : lhsKeys) {
    const auto found = rhsKeys.find(name);
    require(found != rhsKeys.end(), path + ": missing object " + name);
    auto *rhsKey = found->second;
    require(std::string(lhsKey->GetClassName()) == rhsKey->GetClassName(),
            path + ": class mismatch for " + name);

    std::unique_ptr<TObject> lhsObject(lhsKey->ReadObj());
    std::unique_ptr<TObject> rhsObject(rhsKey->ReadObj());
    const auto objectPath = path.empty() ? name : path + "/" + name;
    if (auto *lhsDirectory = dynamic_cast<TDirectory *>(lhsObject.get())) {
      auto *rhsDirectory = dynamic_cast<TDirectory *>(rhsObject.get());
      require(rhsDirectory != nullptr, objectPath + ": expected directory");
      ++counts.directories;
      compareDirectory(*lhsDirectory, *rhsDirectory, objectPath, counts);
    } else if (auto *lhsTree = dynamic_cast<TTree *>(lhsObject.get())) {
      auto *rhsTree = dynamic_cast<TTree *>(rhsObject.get());
      require(rhsTree != nullptr, objectPath + ": expected tree");
      compareTree(*lhsTree, *rhsTree, objectPath);
      ++counts.trees;
      counts.branches += branchNames(*lhsTree).size();
    } else if (auto *lhsHistogram = dynamic_cast<TH1 *>(lhsObject.get())) {
      auto *rhsHistogram = dynamic_cast<TH1 *>(rhsObject.get());
      require(rhsHistogram != nullptr, objectPath + ": expected histogram");
      compareHistogram(*lhsHistogram, *rhsHistogram, objectPath);
      ++counts.histograms;
    } else {
      throw std::runtime_error(objectPath + ": unsupported class " +
                               lhsKey->GetClassName());
    }
  }
}

} // namespace

int main(int argc, char **argv) {
  if (argc != 3) {
    std::cerr << "usage: compare_root_outputs REFERENCE.root CANDIDATE.root\n";
    return 2;
  }
  try {
    TFile lhs(argv[1], "READ");
    TFile rhs(argv[2], "READ");
    require(!lhs.IsZombie(), std::string("failed to open ") + argv[1]);
    require(!rhs.IsZombie(), std::string("failed to open ") + argv[2]);
    Counts counts;
    compareDirectory(lhs, rhs, "", counts);
    std::cout << "EXACT_ROOT_OUTPUT_MATCH directories=" << counts.directories
              << " trees=" << counts.trees << " branches=" << counts.branches
              << " histograms=" << counts.histograms << '\n';
    return 0;
  } catch (const std::exception &error) {
    std::cerr << "compare_root_outputs: " << error.what() << '\n';
    return 1;
  }
}

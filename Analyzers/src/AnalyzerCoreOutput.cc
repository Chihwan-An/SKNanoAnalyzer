#include "AnalyzerCore.h"
#include <Compression.h>
#include <algorithm>
#include <string_view>
#include <utility>
#include <vector>

namespace {

bool outputPathsConflict(std::string_view first, std::string_view second) {
  if (first == second)
    return true;
  const auto isParent = [](std::string_view parent, std::string_view child) {
    return child.size() > parent.size() &&
           child.compare(0, parent.size(), parent) == 0 &&
           child[parent.size()] == '/';
  };
  return isParent(first, second) || isParent(second, first);
}

} // namespace

void AnalyzerCore::FillHist(std::string_view histname, float value,
                            float weight, int n_bin, float x_min, float x_max) {
  BookHist1D(histname, n_bin, x_min, x_max).Fill(value, weight);
}

void AnalyzerCore::FillHist(std::string_view histname, float value,
                            float weight, int n_bin, float *xbins) {
  BookHist1D(histname, n_bin, xbins).Fill(value, weight);
}

void AnalyzerCore::FillHist(std::string_view histname, float value_x,
                            float value_y, float weight, int n_binx,
                            float x_min, float x_max, int n_biny, float y_min,
                            float y_max) {
  BookHist2D(histname, n_binx, x_min, x_max, n_biny, y_min, y_max)
      .Fill(value_x, value_y, weight);
}

void AnalyzerCore::FillHist(std::string_view histname, float value_x,
                            float value_y, float weight, int n_binx,
                            float *xbins, int n_biny, float *ybins) {
  BookHist2D(histname, n_binx, xbins, n_biny, ybins)
      .Fill(value_x, value_y, weight);
}

void AnalyzerCore::FillHist(std::string_view histname, float value_x,
                            float value_y, float value_z, float weight,
                            int n_binx, float x_min, float x_max, int n_biny,
                            float y_min, float y_max, int n_binz, float z_min,
                            float z_max) {
  BookHist3D(histname, n_binx, x_min, x_max, n_biny, y_min, y_max, n_binz,
             z_min, z_max)
      .Fill(value_x, value_y, value_z, weight);
}

void AnalyzerCore::FillHist(std::string_view histname, float value_x,
                            float value_y, float value_z, float weight,
                            int n_binx, float *xbins, int n_biny, float *ybins,
                            int n_binz, float *zbins) {
  BookHist3D(histname, n_binx, xbins, n_biny, ybins, n_binz, zbins)
      .Fill(value_x, value_y, value_z, weight);
}

TTree *AnalyzerCore::NewTree(const TString &treename,
                             const RVec<TString> &keeps,
                             const RVec<TString> &drops) {
  auto treekey = string(treename);
  auto it = treemap.find(treekey);
  if (it == treemap.end()) {
    ValidateTreePath(treekey);
    const bool makeEmptyTree =
        keeps.empty() &&
        (drops.empty() || (drops.size() == 1 && drops[0] == "*"));
    if (makeEmptyTree) {
      TTree *newtree = new TTree(treekey.c_str(), "");
      treemap[treekey] = newtree;
      return newtree;
    } else {
      // check tree is empty.
      if (fChain->GetEntries() == 0) {
        throw SKNano::ConfigError("[AnalyzerCore::NewTree] fChain is empty");
      }
      TTree *newtree = fChain->CloneTree(0);
      newtree->SetName(treekey.c_str());
      for (const auto &drop : drops) {
        newtree->SetBranchStatus(drop, 0);
      }
      for (const auto &keep : keeps) {
        newtree->SetBranchStatus(keep, 1);
      }
      treemap[treekey] = newtree;
      unordered_map<string, TBranch *> this_branchmap;
      branchmaps[newtree] = this_branchmap;
      return newtree;
    }
  } else {
    return it->second;
  }
}

AnalyzerCore::TreeHandle
AnalyzerCore::BookTree(const TString &treename, const RVec<TString> &keeps,
                       const RVec<TString> &drops) {
  NewTree(treename, keeps, drops);
  return TreeHandle(this, std::string(treename.Data()));
}

AnalyzerCore::TreeHandle AnalyzerCore::OutputTree(const TString &treename) {
  GetTree(treename);
  return TreeHandle(this, std::string(treename.Data()));
}

AnalyzerCore::HistogramGroup AnalyzerCore::Hists(std::string_view prefix) {
  std::string normalized(prefix);
  while (!normalized.empty() && normalized.front() == '/')
    normalized.erase(normalized.begin());
  while (!normalized.empty() && normalized.back() == '/')
    normalized.pop_back();
  return HistogramGroup(this, std::move(normalized));
}

void AnalyzerCore::TreeHandle::RequireValid() const {
  if (!owner_)
    throw SKNano::LogicError("[TreeHandle] empty handle access");
}

TTree *AnalyzerCore::TreeHandle::get() const {
  RequireValid();
  return owner_->GetTree(treeName_);
}

AnalyzerCore::TreeHandle &AnalyzerCore::TreeHandle::Set(const TString &name,
                                                        float value) {
  RequireValid();
  owner_->SetBranch(treeName_, name, value);
  return *this;
}

AnalyzerCore::TreeHandle &AnalyzerCore::TreeHandle::Set(const TString &name,
                                                        double value) {
  RequireValid();
  owner_->SetBranch(treeName_, name, value);
  return *this;
}

AnalyzerCore::TreeHandle &AnalyzerCore::TreeHandle::Set(const TString &name,
                                                        int value) {
  RequireValid();
  owner_->SetBranch(treeName_, name, value);
  return *this;
}

AnalyzerCore::TreeHandle &AnalyzerCore::TreeHandle::Set(const TString &name,
                                                        bool value) {
  RequireValid();
  owner_->SetBranch(treeName_, name, value);
  return *this;
}

void AnalyzerCore::TreeHandle::Fill() const {
  RequireValid();
  owner_->FillTrees(treeName_);
}

void AnalyzerCore::HistogramGroup::RequireValid() const {
  if (!owner_)
    throw SKNano::LogicError("[HistogramGroup] empty group access");
}

std::string
AnalyzerCore::HistogramGroup::Resolve(std::string_view name) const {
  RequireValid();
  if (name.empty())
    throw SKNano::ConfigError("[HistogramGroup] histogram name is empty");
  std::string relative(name);
  while (!relative.empty() && relative.front() == '/')
    relative.erase(relative.begin());
  return prefix_.empty() ? relative : prefix_ + "/" + relative;
}

AnalyzerCore::HistogramGroup
AnalyzerCore::HistogramGroup::Group(std::string_view child) const {
  return HistogramGroup(owner_, Resolve(child));
}

Hist1DHandle AnalyzerCore::HistogramGroup::Book1D(
    std::string_view name, int bins, double minimum, double maximum) const {
  return owner_->BookHist1D(Resolve(name), bins, minimum, maximum);
}

Hist1DHandle AnalyzerCore::HistogramGroup::Book1D(
    std::string_view name, int bins, const float *edges) const {
  return owner_->BookHist1D(Resolve(name), bins, edges);
}

Hist2DHandle AnalyzerCore::HistogramGroup::Book2D(
    std::string_view name, int binsX, double minX, double maxX, int binsY,
    double minY, double maxY) const {
  return owner_->BookHist2D(Resolve(name), binsX, minX, maxX, binsY, minY,
                            maxY);
}

Hist2DHandle AnalyzerCore::HistogramGroup::Book2D(
    std::string_view name, int binsX, const float *edgesX, int binsY,
    const float *edgesY) const {
  return owner_->BookHist2D(Resolve(name), binsX, edgesX, binsY, edgesY);
}

Hist3DHandle AnalyzerCore::HistogramGroup::Book3D(
    std::string_view name, int binsX, double minX, double maxX, int binsY,
    double minY, double maxY, int binsZ, double minZ, double maxZ) const {
  return owner_->BookHist3D(Resolve(name), binsX, minX, maxX, binsY, minY,
                            maxY, binsZ, minZ, maxZ);
}

Hist3DHandle AnalyzerCore::HistogramGroup::Book3D(
    std::string_view name, int binsX, const float *edgesX, int binsY,
    const float *edgesY, int binsZ, const float *edgesZ) const {
  return owner_->BookHist3D(Resolve(name), binsX, edgesX, binsY, edgesY,
                            binsZ, edgesZ);
}

void AnalyzerCore::HistogramGroup::Fill(std::string_view name, float value,
                                        float weight, int bins, float minimum,
                                        float maximum) const {
  Book1D(name, bins, minimum, maximum).Fill(value, weight);
}

void AnalyzerCore::HistogramGroup::Fill(std::string_view name, float value,
                                        float weight, int bins,
                                        float *edges) const {
  Book1D(name, bins, edges).Fill(value, weight);
}

void AnalyzerCore::HistogramGroup::Fill(
    std::string_view name, float x, float y, float weight, int binsX,
    float minX, float maxX, int binsY, float minY, float maxY) const {
  Book2D(name, binsX, minX, maxX, binsY, minY, maxY).Fill(x, y, weight);
}

void AnalyzerCore::HistogramGroup::Fill(std::string_view name, float x,
                                        float y, float weight, int binsX,
                                        float *edgesX, int binsY,
                                        float *edgesY) const {
  Book2D(name, binsX, edgesX, binsY, edgesY).Fill(x, y, weight);
}

void AnalyzerCore::HistogramGroup::Fill(
    std::string_view name, float x, float y, float z, float weight, int binsX,
    float minX, float maxX, int binsY, float minY, float maxY, int binsZ,
    float minZ, float maxZ) const {
  Book3D(name, binsX, minX, maxX, binsY, minY, maxY, binsZ, minZ, maxZ)
      .Fill(x, y, z, weight);
}

void AnalyzerCore::HistogramGroup::Fill(
    std::string_view name, float x, float y, float z, float weight, int binsX,
    float *edgesX, int binsY, float *edgesY, int binsZ, float *edgesZ) const {
  Book3D(name, binsX, edgesX, binsY, edgesY, binsZ, edgesZ)
      .Fill(x, y, z, weight);
}

TTree *AnalyzerCore::GetTree(const TString &treename) {
  auto treekey = string(treename);
  auto it = treemap.find(treekey);
  if (it == treemap.end()) {
    throw SKNano::LogicError("[AnalyzerCore::GetTree] Tree " +
                             std::string(treename.Data()) + " not found");
  }
  return it->second;
}

void AnalyzerCore::SetBranch(const TString &treename, const TString &branchname,
                             void *address, const TString &leaflist) {
  try {
    void *this_address = address;
    TTree *tree = GetTree(treename);

    unordered_map<string, TBranch *> *this_branchmap = &branchmaps[tree];
    auto it = this_branchmap->find(string(branchname));

    if (it == this_branchmap->end()) {
      auto br = tree->Branch(branchname, this_address, leaflist);
      this_branchmap->insert({string(branchname), br});
      branchAddressCache[br] = this_address;
      branchSchemaCache[br] = std::string(leaflist.Data());
    } else {
      const auto schemaIt = branchSchemaCache.find(it->second);
      if (schemaIt == branchSchemaCache.end() ||
          schemaIt->second != std::string(leaflist.Data()))
        throw SKNano::ConfigError(
            "[AnalyzerCore::SetBranch] incompatible duplicate branch '" +
            std::string(treename.Data()) + "/" +
            std::string(branchname.Data()) + "'");
      auto cacheIt = branchAddressCache.find(it->second);
      if (cacheIt == branchAddressCache.end() ||
          cacheIt->second != this_address) {
        it->second->SetAddress(this_address);
        branchAddressCache[it->second] = this_address;
      }
    }
  } catch (int e) {
    throw SKNano::LogicError("[AnalyzerCore::SetBranch] Error get tree: " +
                             std::string(treename.Data()) +
                             " code=" + std::to_string(e));
  }
}

void AnalyzerCore::ValidateTreePath(std::string_view treeName) const {
  if (treeName.empty())
    throw SKNano::ConfigError("[AnalyzerCore::BookTree] tree name is empty");
  for (const auto &[otherTree, unused] : treemap) {
    static_cast<void>(unused);
    if (outputPathsConflict(treeName, otherTree))
      throw SKNano::ConfigError(
          "[AnalyzerCore::BookTree] output path conflicts with tree '" +
          otherTree + "'");
  }
  const auto checkHistograms = [&](const auto &histograms) {
    for (const auto &[histogram, unused] : histograms) {
      static_cast<void>(unused);
      if (outputPathsConflict(treeName, histogram))
        throw SKNano::ConfigError(
            "[AnalyzerCore::BookTree] output path conflicts with histogram '" +
            histogram + "'");
    }
  };
  checkHistograms(histmap1d);
  checkHistograms(histmap2d);
  checkHistograms(histmap3d);
}

void AnalyzerCore::ValidateHistogramPath(
    std::string_view histogramName) const {
  if (histogramName.empty())
    throw SKNano::ConfigError(
        "[AnalyzerCore::BookHist] histogram name is empty");
  for (const auto &[treeName, unused] : treemap) {
    static_cast<void>(unused);
    if (outputPathsConflict(histogramName, treeName))
      throw SKNano::ConfigError(
          "[AnalyzerCore::BookHist] output path conflicts with tree '" +
          treeName + "'");
  }
}

template void AnalyzerCore::SetBranch_Vector<int>(const TString &,
                                                  const TString &,
                                                  std::vector<int> &);
template void AnalyzerCore::SetBranch_Vector<float>(const TString &,
                                                    const TString &,
                                                    std::vector<float> &);
template void AnalyzerCore::SetBranch_Vector<double>(const TString &,
                                                     const TString &,
                                                     std::vector<double> &);
template void AnalyzerCore::SetBranch_Vector<bool>(const TString &,
                                                   const TString &,
                                                   std::vector<bool> &);

void AnalyzerCore::FillTrees(const TString &treename) {
  if (treename == "") {
    for (const auto &pair : treemap) {
      const string &treename = pair.first;
      TTree *tree = pair.second;
      tree->Fill();
    }
  } else {
    // Convert treeName to std::string for comparison
    std::string treeNameStr(treename.Data());

    auto it = treemap.find(treeNameStr);
    if (it != treemap.end()) {
      // Tree with the given name exists, fill it
      TTree *tree = it->second;
      tree->Fill();
    } else {
      // Handle the case where the treeName is not found in the map
      throw std::runtime_error("[AnalyzerCore::FillTrees] Tree with name '" +
                               treeNameStr + "' not found in treemap.");
    }
  }
}

void AnalyzerCore::WriteHist() {
  const int compression_level = 4;
  const int compression_algorithm = ROOT::RCompressionSetting::EAlgorithm::kLZ4;
  cout << "[AnalyzerCore::WriteHist] Writing histograms to "
       << outfile->GetName() << endl;
  cout << "[AnalyzerCore::WriteHist] Set compression algorithm to LZ4 with "
          "level "
       << compression_level << endl;
  outfile->SetCompressionAlgorithm(compression_algorithm);
  outfile->SetCompressionLevel(compression_level);
  std::vector<std::pair<std::string, TH1 *>> sorted_histograms1d(
      histmap1d.begin(), histmap1d.end());
  std::vector<std::pair<std::string, TH2 *>> sorted_histograms2d(
      histmap2d.begin(), histmap2d.end());
  std::vector<std::pair<std::string, TH3 *>> sorted_histograms3d(
      histmap3d.begin(), histmap3d.end());
  std::sort(
      sorted_histograms1d.begin(), sorted_histograms1d.end(),
      [](const std::pair<std::string, TH1 *> &a,
         const std::pair<std::string, TH1 *> &b) { return a.first < b.first; });
  std::sort(
      sorted_histograms2d.begin(), sorted_histograms2d.end(),
      [](const std::pair<std::string, TH2 *> &a,
         const std::pair<std::string, TH2 *> &b) { return a.first < b.first; });
  std::sort(
      sorted_histograms3d.begin(), sorted_histograms3d.end(),
      [](const std::pair<std::string, TH3 *> &a,
         const std::pair<std::string, TH3 *> &b) { return a.first < b.first; });
  for (const auto &pair : sorted_histograms1d) {
    const string &histname = pair.first;
    TH1 *hist = pair.second;
    cout << "[AnalyzerCore::WriteHist] Writing 1D histogram: " << histname
         << endl;
    // Split the directory and name
    // e.g. "dir1/dir2/histname" -> "dir1/dir2", "histname"
    // e.g. "histname" -> "", "histname"
    size_t last_slash = histname.find_last_of('/');
    string this_prefix, this_name;
    last_slash == string::npos ? this_prefix = ""
                               : this_prefix = histname.substr(0, last_slash);
    last_slash == string::npos ? this_name = histname
                               : this_name = histname.substr(last_slash + 1);

    TDirectory *this_dir = outfile->GetDirectory(this_prefix.c_str());
    if (!this_dir)
      outfile->mkdir(this_prefix.c_str());
    outfile->cd(this_prefix.c_str());
    hist->Write(this_name.c_str());
  }
  for (const auto &pair : sorted_histograms2d) {
    const string &histname = pair.first;
    cout << "[AnalyzerCore::WriteHist] Writing 2D histogram: " << histname
         << endl;
    TH2 *hist = pair.second;
    // Split the directory and name
    // e.g. "dir1/dir2/histname" -> "dir1/dir2", "histname"
    // e.g. "histname" -> "", "histname"
    size_t last_slash = histname.find_last_of('/');
    string this_prefix, this_name;
    last_slash == string::npos ? this_prefix = ""
                               : this_prefix = histname.substr(0, last_slash);
    last_slash == string::npos ? this_name = histname
                               : this_name = histname.substr(last_slash + 1);

    TDirectory *this_dir = outfile->GetDirectory(this_prefix.c_str());
    if (!this_dir)
      outfile->mkdir(this_prefix.c_str());
    outfile->cd(this_prefix.c_str());
    hist->Write(this_name.c_str());
  }
  for (const auto &pair : sorted_histograms3d) {
    const string &histname = pair.first;
    cout << "[AnalyzerCore::WriteHist] Writing 3D histogram: " << histname
         << endl;
    TH3 *hist = pair.second;
    // Split the directory and name
    // e.g. "dir1/dir2/histname" -> "dir1/dir2", "histname"
    // e.g. "histname" -> "", "histname"
    size_t last_slash = histname.find_last_of('/');
    string this_prefix, this_name;
    last_slash == string::npos ? this_prefix = ""
                               : this_prefix = histname.substr(0, last_slash);
    last_slash == string::npos ? this_name = histname
                               : this_name = histname.substr(last_slash + 1);

    TDirectory *this_dir = outfile->GetDirectory(this_prefix.c_str());
    if (!this_dir)
      outfile->mkdir(this_prefix.c_str());
    outfile->cd(this_prefix.c_str());
    hist->Write(this_name.c_str());
  }
  for (const auto &pair : treemap) {
    const string &treename = pair.first;
    cout << "[AnalyzerCore::WriteHist] Writing tree: " << treename << endl;
    TTree *tree = pair.second;

    size_t last_slash = treename.find_last_of('/');
    string this_prefix, this_name;
    last_slash == string::npos ? this_prefix = ""
                               : this_prefix = treename.substr(0, last_slash);
    last_slash == string::npos ? this_name = treename
                               : this_name = treename.substr(last_slash + 1);

    TDirectory *this_dir = outfile->GetDirectory(this_prefix.c_str());
    if (!this_dir)
      outfile->mkdir(this_prefix.c_str());
    outfile->cd(this_prefix.c_str());
    tree->Write(this_name.c_str());
    delete tree;
  }
  cout << "[AnalyzerCore::WriteHist] Writing histograms done" << endl;
  outfile->Close();
}

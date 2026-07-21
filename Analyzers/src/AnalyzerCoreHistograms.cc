#include "AnalyzerCore.h"

#include "TArrayD.h"

#include <string>

namespace {

[[noreturn]] void conflict(std::string_view name, const std::string &reason) {
  throw SKNano::ConfigError("[AnalyzerCore] histogram '" + std::string(name) +
                            "' " + reason);
}

void requireUniform(std::string_view name, const TAxis &axis, int bins,
                    double minimum, double maximum) {
  if (bins <= 0 || axis.GetNbins() != bins ||
      axis.GetXbins()->GetSize() != 0 || axis.GetXmin() != minimum ||
      axis.GetXmax() != maximum) {
    conflict(name, "was requested with incompatible binning");
  }
}

void requireVariable(std::string_view name, const TAxis &axis, int bins,
                     const float *edges) {
  if (bins <= 0 || !edges)
    conflict(name, "received invalid bin edges");
  const TArrayD *stored = axis.GetXbins();
  if (axis.GetNbins() != bins || stored->GetSize() != bins + 1)
    conflict(name, "was requested with incompatible binning");
  for (int index = 0; index <= bins; ++index) {
    if ((*stored)[index] != static_cast<double>(edges[index]))
      conflict(name, "was requested with incompatible binning");
  }
}

} // namespace

Hist1DHandle AnalyzerCore::BookHist1D(std::string_view name, int bins,
                                      double minimum, double maximum) {
  ValidateHistogramPath(name);
  if (bins <= 0 || !(minimum < maximum))
    conflict(name, "received invalid uniform binning");
  if (histmap2d.find(name) != histmap2d.end() ||
      histmap3d.find(name) != histmap3d.end())
    conflict(name, "was already booked with another dimension");
  const auto found = histmap1d.find(name);
  if (found != histmap1d.end()) {
    requireUniform(name, *found->second->GetXaxis(), bins, minimum, maximum);
    return Hist1DHandle(found->second);
  }
  const std::string key(name);
  TH1 *histogram = useTH1F
      ? static_cast<TH1 *>(new TH1F(key.c_str(), "", bins, minimum, maximum))
      : static_cast<TH1 *>(new TH1D(key.c_str(), "", bins, minimum, maximum));
  histogram->SetDirectory(nullptr);
  histmap1d.emplace(key, histogram);
  return Hist1DHandle(histogram);
}

Hist1DHandle AnalyzerCore::BookHist1D(std::string_view name, int bins,
                                      const float *edges) {
  ValidateHistogramPath(name);
  if (bins <= 0 || !edges)
    conflict(name, "received invalid bin edges");
  if (histmap2d.find(name) != histmap2d.end() ||
      histmap3d.find(name) != histmap3d.end())
    conflict(name, "was already booked with another dimension");
  const auto found = histmap1d.find(name);
  if (found != histmap1d.end()) {
    requireVariable(name, *found->second->GetXaxis(), bins, edges);
    return Hist1DHandle(found->second);
  }
  const std::string key(name);
  TH1 *histogram = useTH1F
      ? static_cast<TH1 *>(new TH1F(key.c_str(), "", bins, edges))
      : static_cast<TH1 *>(new TH1D(key.c_str(), "", bins, edges));
  histogram->SetDirectory(nullptr);
  histmap1d.emplace(key, histogram);
  return Hist1DHandle(histogram);
}

Hist2DHandle AnalyzerCore::BookHist2D(std::string_view name, int binsX,
                                      double minX, double maxX, int binsY,
                                      double minY, double maxY) {
  ValidateHistogramPath(name);
  if (binsX <= 0 || binsY <= 0 || !(minX < maxX) || !(minY < maxY))
    conflict(name, "received invalid uniform binning");
  if (histmap1d.find(name) != histmap1d.end() ||
      histmap3d.find(name) != histmap3d.end())
    conflict(name, "was already booked with another dimension");
  const auto found = histmap2d.find(name);
  if (found != histmap2d.end()) {
    requireUniform(name, *found->second->GetXaxis(), binsX, minX, maxX);
    requireUniform(name, *found->second->GetYaxis(), binsY, minY, maxY);
    return Hist2DHandle(found->second);
  }
  const std::string key(name);
  TH2 *histogram = useTH1F
      ? static_cast<TH2 *>(new TH2F(key.c_str(), "", binsX, minX, maxX,
                                   binsY, minY, maxY))
      : static_cast<TH2 *>(new TH2D(key.c_str(), "", binsX, minX, maxX,
                                   binsY, minY, maxY));
  histogram->SetDirectory(nullptr);
  histmap2d.emplace(key, histogram);
  return Hist2DHandle(histogram);
}

Hist2DHandle AnalyzerCore::BookHist2D(std::string_view name, int binsX,
                                      const float *edgesX, int binsY,
                                      const float *edgesY) {
  ValidateHistogramPath(name);
  if (binsX <= 0 || binsY <= 0 || !edgesX || !edgesY)
    conflict(name, "received invalid bin edges");
  if (histmap1d.find(name) != histmap1d.end() ||
      histmap3d.find(name) != histmap3d.end())
    conflict(name, "was already booked with another dimension");
  const auto found = histmap2d.find(name);
  if (found != histmap2d.end()) {
    requireVariable(name, *found->second->GetXaxis(), binsX, edgesX);
    requireVariable(name, *found->second->GetYaxis(), binsY, edgesY);
    return Hist2DHandle(found->second);
  }
  const std::string key(name);
  TH2 *histogram = useTH1F
      ? static_cast<TH2 *>(new TH2F(key.c_str(), "", binsX, edgesX, binsY,
                                   edgesY))
      : static_cast<TH2 *>(new TH2D(key.c_str(), "", binsX, edgesX, binsY,
                                   edgesY));
  histogram->SetDirectory(nullptr);
  histmap2d.emplace(key, histogram);
  return Hist2DHandle(histogram);
}

Hist3DHandle AnalyzerCore::BookHist3D(std::string_view name, int binsX,
                                      double minX, double maxX, int binsY,
                                      double minY, double maxY, int binsZ,
                                      double minZ, double maxZ) {
  ValidateHistogramPath(name);
  if (binsX <= 0 || binsY <= 0 || binsZ <= 0 || !(minX < maxX) ||
      !(minY < maxY) || !(minZ < maxZ))
    conflict(name, "received invalid uniform binning");
  if (histmap1d.find(name) != histmap1d.end() ||
      histmap2d.find(name) != histmap2d.end())
    conflict(name, "was already booked with another dimension");
  const auto found = histmap3d.find(name);
  if (found != histmap3d.end()) {
    requireUniform(name, *found->second->GetXaxis(), binsX, minX, maxX);
    requireUniform(name, *found->second->GetYaxis(), binsY, minY, maxY);
    requireUniform(name, *found->second->GetZaxis(), binsZ, minZ, maxZ);
    return Hist3DHandle(found->second);
  }
  const std::string key(name);
  TH3 *histogram = useTH1F
      ? static_cast<TH3 *>(new TH3F(key.c_str(), "", binsX, minX, maxX,
                                   binsY, minY, maxY, binsZ, minZ, maxZ))
      : static_cast<TH3 *>(new TH3D(key.c_str(), "", binsX, minX, maxX,
                                   binsY, minY, maxY, binsZ, minZ, maxZ));
  histogram->SetDirectory(nullptr);
  histmap3d.emplace(key, histogram);
  return Hist3DHandle(histogram);
}

Hist3DHandle AnalyzerCore::BookHist3D(std::string_view name, int binsX,
                                      const float *edgesX, int binsY,
                                      const float *edgesY, int binsZ,
                                      const float *edgesZ) {
  ValidateHistogramPath(name);
  if (binsX <= 0 || binsY <= 0 || binsZ <= 0 || !edgesX || !edgesY ||
      !edgesZ)
    conflict(name, "received invalid bin edges");
  if (histmap1d.find(name) != histmap1d.end() ||
      histmap2d.find(name) != histmap2d.end())
    conflict(name, "was already booked with another dimension");
  const auto found = histmap3d.find(name);
  if (found != histmap3d.end()) {
    requireVariable(name, *found->second->GetXaxis(), binsX, edgesX);
    requireVariable(name, *found->second->GetYaxis(), binsY, edgesY);
    requireVariable(name, *found->second->GetZaxis(), binsZ, edgesZ);
    return Hist3DHandle(found->second);
  }
  const std::string key(name);
  TH3 *histogram = useTH1F
      ? static_cast<TH3 *>(new TH3F(key.c_str(), "", binsX, edgesX, binsY,
                                   edgesY, binsZ, edgesZ))
      : static_cast<TH3 *>(new TH3D(key.c_str(), "", binsX, edgesX, binsY,
                                   edgesY, binsZ, edgesZ));
  histogram->SetDirectory(nullptr);
  histmap3d.emplace(key, histogram);
  return Hist3DHandle(histogram);
}

TH1D *AnalyzerCore::GetHist1D(const std::string &name) {
  const auto found = histmap1d.find(name);
  if (found == histmap1d.end())
    throw SKNano::LogicError("[AnalyzerCore::GetHist1D] histogram '" + name +
                             "' is not booked");
  auto *histogram = dynamic_cast<TH1D *>(found->second);
  if (!histogram)
    throw SKNano::LogicError("[AnalyzerCore::GetHist1D] histogram '" + name +
                             "' is not TH1D");
  return histogram;
}

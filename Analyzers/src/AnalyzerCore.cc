#include "AnalyzerCore.h"
#include "JetView.h"
#include "TObjArray.h"
#include "TObjString.h"
#include <Compression.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <execution>
#include <fstream>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <utility>

AnalyzerCore::AnalyzerCore() {
  myCorr = nullptr;
  outfile = nullptr;
  if (HasFlag("useTH1F")) {
    cout << "[AnalyzerCore::AnalyzerCore] Using TH1F" << endl;
    useTH1F = true;
  } else {
    cout << "[AnalyzerCore::AnalyzerCore] Using TH1D" << endl;
    useTH1F = false;
  }
  histmap1d.reserve(512);
  histmap2d.reserve(256);
  histmap3d.reserve(128);
  // pdfReweight = new PDFReweight();
}
AnalyzerCore::~AnalyzerCore() {
  for (const auto &pair : histmap1d)
    delete pair.second;
  histmap1d.clear();
  for (const auto &pair : histmap2d)
    delete pair.second;
  histmap2d.clear();
  for (const auto &pair : histmap3d)
    delete pair.second;
  histmap3d.clear();
  if (outfile)
    delete outfile;
  if (myCorr)
    delete myCorr;
  // if (pdfReweight) delete pdfReweight;
}

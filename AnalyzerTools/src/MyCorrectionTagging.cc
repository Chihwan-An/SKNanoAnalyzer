#include "MyCorrection.h"
#include <algorithm>
#include <cmath>
#include <execution>
#include <numeric>
#include <unordered_set>
#include <vector>

// Heavy flavor tagging
void MyCorrection::SetTaggingParam(JetTagging::JetFlavTagger tagger,
                                   JetTagging::JetFlavTaggerWP wp) {
  global_tagger = tagger;
  global_wp = wp;
  global_taggerStr = JetTagging::GetTaggerCorrectionLibStr(tagger);
  global_wpStr = JetTagging::GetTaggerCorrectionWPStr(wp);
}

float MyCorrection::GetBTaggingWP() const {
  try {
    correction::Correction::Ref cset =
        cset_btagging->at(global_taggerStr + "_wp_values");
    return safeEvaluate(cset, "GetBTaggingWP", {global_wpStr});
  } catch (const exception &e) {
    cerr << "[Correction::GetBTaggingWP] Warning: Failed to evaluate WP '"
         << global_wpStr << "' for tagger '" << global_taggerStr << endl;
    throw runtime_error(e.what());
    return 1.f;
  }
}

float MyCorrection::GetBTaggingWP(JetTagging::JetFlavTagger tagger,
                                  JetTagging::JetFlavTaggerWP wp) const {
  // Convert enumerations to strings
  string this_taggerStr = JetTagging::GetTaggerCorrectionLibStr(tagger).Data();
  string this_wpStr = JetTagging::GetTaggerCorrectionWPStr(wp).Data();

  try {
    correction::Correction::Ref cset =
        cset_btagging->at(this_taggerStr + "_wp_values");
    return safeEvaluate(cset, "GetBTaggingWP", {this_wpStr});
  } catch (const exception &e) {
    cerr << "[Correction::GetBTaggingWP] Warning: Failed to evaluate WP '"
         << this_wpStr << "' for tagger '" << this_taggerStr << endl;
    throw runtime_error(e.what());
    return 1.f;
  }
}

float MyCorrection::GetBTaggingEff(const float eta, const float pt,
                                   const int flav,
                                   JetTagging::JetFlavTagger tagger,
                                   JetTagging::JetFlavTaggerWP wp,
                                   const variation syst) {
  string this_taggerStr = JetTagging::GetTaggerCorrectionLibStr(tagger).Data();
  string this_wpStr = JetTagging::GetTaggerCorrectionWPStr(wp).Data();
  auto cset = cset_btagging_eff->at(this_taggerStr);
  return safeEvaluate(cset, "GetBTaggingSF",
                      {"central", this_wpStr, flav, fabs(eta), pt});
}

pair<float, float> MyCorrection::GetCTaggingWP() const {
  try {
    correction::Correction::Ref cset =
        cset_ctagging->at(global_taggerStr + "_wp_values");
    float valCvB = safeEvaluate(cset, "GetCTaggingWP", {global_wpStr, "CvB"});
    float valCvL = safeEvaluate(cset, "GetCTaggingWP", {global_wpStr, "CvL"});
    return make_pair(valCvB, valCvL);
  } catch (const exception &e) {
    // If the requested WP is not found or any other error occurs,
    // log a warning and return (1.f, 1.f) as a fallback.
    cerr << "[Correction::GetCTaggingWP] Warning: Failed to evaluate WP '"
         << global_wpStr << "' for tagger '" << global_taggerStr << endl;
    throw runtime_error(e.what());
    return make_pair(1.f, 1.f);
  }
}

pair<float, float>
MyCorrection::GetCTaggingWP(JetTagging::JetFlavTagger tagger,
                            JetTagging::JetFlavTaggerWP wp) const {
  // Convert enumerations to strings using your existing utility functions
  string this_taggerStr = JetTagging::GetTaggerCorrectionLibStr(tagger).Data();
  string this_wpStr = JetTagging::GetTaggerCorrectionWPStr(wp).Data();

  // Retrieve the relevant correction set
  correction::Correction::Ref cset =
      cset_ctagging->at(this_taggerStr + "_wp_values");

  try {
    // Evaluate the corrections. If the WP does not exist, an exception might be
    // thrown
    float valCvB = safeEvaluate(cset, "GetCTaggingWP", {this_wpStr, "CvB"});
    float valCvL = safeEvaluate(cset, "GetCTaggingWP", {this_wpStr, "CvL"});

    // If everything is fine, return the pair
    return make_pair(valCvB, valCvL);
  } catch (const exception &e) {
    // In case the WP is not found (or any other error occurs),
    // print a warning (optional) and return default values
    cerr << "[Correction::GetCTaggingWP] Warning: WP '" << this_wpStr
         << "' not found for tagger '" << this_taggerStr << endl;
    throw runtime_error(e.what());
    return make_pair(1.f, 1.f);
  }
}

float MyCorrection::GetCTaggingEff(const float eta, const float pt,
                                   const int flav,
                                   JetTagging::JetFlavTagger tagger,
                                   JetTagging::JetFlavTaggerWP wp,
                                   const variation syst) {
  return 1.;
  string this_taggerStr = JetTagging::GetTaggerCorrectionLibStr(tagger).Data();
  string this_wpStr = JetTagging::GetTaggerCorrectionWPStr(wp).Data();
  correction::Correction::Ref cset = cset_btagging_eff->at(this_taggerStr);
  return safeEvaluate(cset, "GetBTaggingEff",
                      {
                          getSystString_BTV(syst),
                          this_wpStr,
                      });
}

float MyCorrection::GetCTaggingR(const float nTrueInt, const float HT,
                                 const JetTagging::JetFlavTagger tagger,
                                 const TString &processName,
                                 const TString &ttBarCategory,
                                 const TString &syst_str) const {
  string this_taggerStr = JetTagging::GetTaggerCorrectionLibStr(tagger).Data();
  if (processName != "")
    this_taggerStr += "_" + processName;
  else
    this_taggerStr += (string("_") + Sample.Data());
  auto cset = cset_ctagging_R->at(this_taggerStr);
  return safeEvaluate(cset, "GetTopPtReweight",
                      {syst_str.Data(), ttBarCategory.Data(), nTrueInt, HT});
}

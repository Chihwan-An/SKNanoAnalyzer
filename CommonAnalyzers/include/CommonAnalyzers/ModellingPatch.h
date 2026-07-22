#ifndef ModellingPatch_h
#define ModellingPatch_h

#include <AnalyzerFramework/AnalyzerCore.h>
#include <array>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

class ModellingPatch : public AnalyzerCore {
public:
  struct SubprocessSums {
    uint64_t nMC = 0;
    double sumSign = 0.0;
    std::vector<double> sumScaleVariation;
    std::vector<double> sumPSVariation;
    double sum_hdamp_up = 0.0;
    double sum_hdamp_down = 0.0;
    double sum_minnlo = 0.0;
    double sum_bfrag_nom = 0.0;
    double sum_bfrag_up = 0.0;
  };

  ModellingPatch();
  ~ModellingPatch() override = default;

  void initializeAnalyzer() override;
  void executeEvent() override;

private:
  void ensure_sizes(SubprocessSums &sums, std::size_t scale, std::size_t ps) const;
  std::string subprocess_name(int genTtbarId) const;
  std::array<size_t, 4> GetTopAndAntiTopIndices(const GenViewCollection &gens) const;

  std::unordered_map<std::string, SubprocessSums> accumulators_;
  bool isTT_ = false;
  std::string baseSubprocessName_;
};

#endif

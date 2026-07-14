#ifndef SKNanoCore_VariationKernel_h
#define SKNanoCore_VariationKernel_h

#include "AnalysisException.h"
#include "ExecutionPlan.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace SKNano {

struct VariationPlan {
    std::vector<SystematicId> lanes;
    std::size_t chunks() const noexcept { return (lanes.size() + 63) / 64; }
};

struct FusedSelectionResult {
    std::size_t objectCount = 0;
    std::size_t laneCount = 0;
    std::vector<std::uint64_t> objectLaneMasks;
    std::vector<std::vector<std::size_t>> selectedIndices;
    std::vector<double> laneWeights;

    bool passed(std::size_t object, std::size_t lane) const {
        if (object >= objectCount || lane >= laneCount)
            throw EventDataError("[FusedSelectionResult] index out of range");
        const std::size_t chunks = (laneCount + 63) / 64;
        return (objectLaneMasks[object * chunks + lane / 64] &
                (std::uint64_t{1} << (lane % 64))) != 0;
    }
};

class FusedSelectionKernel {
public:
    template <typename Evaluator, typename Sink>
    void run(std::size_t objectCount, const VariationPlan &plan,
             const Evaluator &evaluator, Sink &sink,
             FusedSelectionResult &result) const {
        result.objectCount = objectCount;
        result.laneCount = plan.lanes.size();
        result.objectLaneMasks.assign(objectCount * plan.chunks(), 0);
        result.selectedIndices.assign(plan.lanes.size(), {});
        result.laneWeights.resize(plan.lanes.size());
        for (std::size_t lane = 0; lane < plan.lanes.size(); ++lane) {
            result.selectedIndices[lane].reserve(objectCount);
            result.laneWeights[lane] = evaluator.eventWeight(lane);
        }

        for (std::size_t object = 0; object < objectCount; ++object) {
            if (!evaluator.commonPass(object))
                continue;
            for (std::size_t lane = 0; lane < plan.lanes.size(); ++lane) {
                if (!evaluator.lanePass(lane, object))
                    continue;
                result.objectLaneMasks[object * plan.chunks() + lane / 64] |=
                    std::uint64_t{1} << (lane % 64);
                result.selectedIndices[lane].push_back(object);
                sink.fill(lane, object, result.laneWeights[lane]);
            }
        }
    }
};

inline std::vector<double>
stableLeaveOneOutProducts(const std::vector<double> &factors) {
    std::vector<double> prefix(factors.size() + 1, 1.);
    std::vector<double> suffix(factors.size() + 1, 1.);
    for (std::size_t index = 0; index < factors.size(); ++index)
        prefix[index + 1] = prefix[index] * factors[index];
    for (std::size_t index = factors.size(); index > 0; --index)
        suffix[index - 1] = factors[index - 1] * suffix[index];
    std::vector<double> result(factors.size(), 1.);
    for (std::size_t index = 0; index < factors.size(); ++index)
        result[index] = prefix[index] * suffix[index + 1];
    return result;
}

template <typename T>
class StableWeightProductPlan {
public:
    explicit StableWeightProductPlan(std::vector<T> factors)
        : factors_(std::move(factors)), prefix_(factors_.size() + 1, T{1}),
          suffix_(factors_.size() + 1, T{1}) {
        for (std::size_t index = 0; index < factors_.size(); ++index)
            prefix_[index + 1] = prefix_[index] * factors_[index];
        for (std::size_t index = factors_.size(); index > 0; --index)
            suffix_[index - 1] = factors_[index - 1] * suffix_[index];
    }
    T product() const noexcept { return prefix_.back(); }
    T productExcluding(std::size_t index) const {
        if (index >= factors_.size())
            throw EventDataError("[StableWeightProductPlan] index out of range");
        return prefix_[index] * suffix_[index + 1];
    }
    T productExcluding(const std::vector<std::size_t> &indices) const {
        std::vector<std::uint8_t> excluded(factors_.size(), 0);
        for (const auto index : indices) {
            if (index >= factors_.size())
                throw EventDataError(
                    "[StableWeightProductPlan] index out of range");
            excluded[index] = 1;
        }
        T result{1};
        for (std::size_t index = 0; index < factors_.size(); ++index)
            if (!excluded[index])
                result *= factors_[index];
        return result;
    }
private:
    std::vector<T> factors_;
    std::vector<T> prefix_;
    std::vector<T> suffix_;
};

} // namespace SKNano

#endif

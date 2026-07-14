#include <exception>
#include <iostream>
#include <stdexcept>
#include <tuple>
#include <vector>

#include <boost/histogram.hpp>

#include "VariationKernel.h"

namespace {
struct Evaluator {
    bool commonPass(std::size_t object) const { return object != 1; }
    bool lanePass(std::size_t lane, std::size_t object) const {
        return object >= lane;
    }
    double eventWeight(std::size_t lane) const { return lane == 0 ? 0. : 2.; }
};
struct Sink {
    std::vector<std::tuple<std::size_t, std::size_t, double>> fills;
    void fill(std::size_t lane, std::size_t object, double weight) {
        fills.emplace_back(lane, object, weight);
    }
};
void require(bool condition, const char *message) {
    if (!condition) throw std::runtime_error(message);
}
}

int main() {
    try {
        SKNano::VariationPlan plan{{SKNano::SystematicId(0),
                                    SKNano::SystematicId(1)}};
        SKNano::FusedSelectionResult result;
        Sink sink;
        SKNano::FusedSelectionKernel().run(4, plan, Evaluator{}, sink, result);
        require(result.passed(0, 0) && !result.passed(0, 1) &&
                    !result.passed(1, 0) && result.passed(3, 1),
                "fused mask must preserve common and lane-specific selection");
        require(result.selectedIndices[0] == std::vector<std::size_t>({0, 2, 3}) &&
                    result.selectedIndices[1] == std::vector<std::size_t>({2, 3}),
                "selected indices must retain stable input ordering");
        require(sink.fills.size() == 5,
                "sink must receive exactly one fill per passing lane/object");

        const auto products =
            SKNano::stableLeaveOneOutProducts({2., 0., 3.});
        require(products == std::vector<double>({0., 6., 0.}),
                "prefix/suffix products must handle nominal zero without ratios");
        SKNano::StableWeightProductPlan<float> floatPlan({2.f, 0.f, 5.f});
        require(floatPlan.product() == 0.f &&
                    floatPlan.productExcluding(1) == 10.f &&
                    floatPlan.productExcluding(
                        std::vector<std::size_t>{0, 1}) == 5.f,
                "compiled float plan must replace zero factors safely");

        namespace bh = boost::histogram;
        auto first = bh::make_histogram_with(
            bh::weight_storage(), bh::axis::variable<>({0., 1., 2.}));
        auto second = bh::make_histogram_with(
            bh::weight_storage(), bh::axis::variable<>({0., 1., 2.}));
        first(0.5, bh::weight(2.));
        second(1.5, bh::weight(-1.));
        first += second;
        require(first.at(0).value() == 2. &&
                    first.at(0).variance() == 4. &&
                    first.at(1).value() == -1.,
                "Boost.Histogram merge must preserve sumw and sumw2");
    } catch (const std::exception &error) {
        std::cerr << "test_variation_kernel: " << error.what() << '\n';
        return 1;
    }
    return 0;
}

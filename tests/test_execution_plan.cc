#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "ExecutionPlan.h"

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
        SKNano::ExecutionPlanBuilder builder;
        const auto pt = builder.column("Jet_pt", SKNano::PlanRequirement::Required,
                                       "Float_t", SKNano::PlanCardinality::Vector);
        const auto ptAgain = builder.column(
            "Jet_pt", SKNano::PlanRequirement::Optional, "Float_t",
            SKNano::PlanCardinality::Vector);
        require(pt == ptAgain,
                "duplicate compatible column must reuse one dense ID");
        const auto eta = builder.column(
            "Jet_eta", SKNano::PlanRequirement::Required, "Float_t",
            SKNano::PlanCardinality::Vector);
        const auto trigger = builder.trigger("HLT_IsoMu24");
        const auto triggerAgain = builder.trigger("HLT_IsoMu24");
        require(trigger == triggerAgain,
                "duplicate trigger must reuse one dense ID");
        const auto correction = builder.correction("JEC", {pt, eta}, {0.4});
        const auto nominal = builder.systematic("nominal");
        const auto jes = builder.systematic("jes_up", {nominal});
        const auto histogram = builder.histogram(
            {"jet_pt", 1, {0., 20., 40.}, {}, {}});
        const auto modelInput = builder.modelInput(
            {"jet_features", "float32", {-1, 8}});
        const auto plan = builder.compile();

        require(plan.column(pt).name == "Jet_pt" &&
                    plan.column(pt).requirement ==
                        SKNano::PlanRequirement::Required,
                "required request must dominate an optional duplicate");
        require(plan.correction(correction).inputs.size() == 2,
                "correction input order must be compiled");
        require(plan.systematicOrder().size() == 2 &&
                    plan.systematicOrder()[0] == nominal &&
                    plan.systematicOrder()[1] == jes,
                "systematic dependency order must be topological");
        require(plan.histogram(histogram).xEdges.size() == 3 &&
                    plan.modelInput(modelInput).shape[1] == 8,
                "histogram and model binding schemas must be retained");

        require(throws<SKNano::ConfigError>([&] {
                    builder.column("Jet_pt", SKNano::PlanRequirement::Required,
                                   "Int_t", SKNano::PlanCardinality::Vector);
                }),
                "conflicting branch type must fail at plan construction");
        require(throws<SKNano::ConfigError>([&] {
                    builder.histogram({"bad", 2, {0., 1.}, {}, {}});
                }),
                "invalid histogram schema must fail at plan construction");
    } catch (const std::exception &error) {
        std::cerr << "test_execution_plan: " << error.what() << '\n';
        return 1;
    }
    return 0;
}

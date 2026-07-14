#ifndef SKNanoCore_ExecutionPlan_h
#define SKNanoCore_ExecutionPlan_h

#include "AnalysisException.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace SKNano {

template <typename Tag>
class DenseId {
public:
    DenseId() = default;
    explicit DenseId(std::uint32_t value_) : value_(value_) {}
    std::uint32_t value() const noexcept { return value_; }
    explicit operator bool() const noexcept { return value_ != invalid(); }
    friend bool operator==(DenseId left, DenseId right) {
        return left.value_ == right.value_;
    }
    static constexpr std::uint32_t invalid() { return UINT32_MAX; }
private:
    std::uint32_t value_ = invalid();
};

using ColumnId = DenseId<struct ColumnIdTag>;
using TriggerId = DenseId<struct TriggerIdTag>;
using CorrectionId = DenseId<struct CorrectionIdTag>;
using SystematicId = DenseId<struct SystematicIdTag>;
using HistId = DenseId<struct HistIdTag>;
using ModelInputId = DenseId<struct ModelInputIdTag>;

enum class PlanRequirement { Required, Optional };
enum class PlanCardinality { Any, Scalar, Vector };

struct BranchPlanEntry {
    std::string name;
    std::string leafType;
    PlanRequirement requirement = PlanRequirement::Required;
    PlanCardinality cardinality = PlanCardinality::Any;
};

struct CorrectionPlanEntry {
    std::string name;
    std::vector<ColumnId> inputs;
    std::vector<double> invariantArguments;
};

struct SystematicPlanEntry {
    std::string name;
    std::vector<SystematicId> dependencies;
};

struct HistogramPlanEntry {
    std::string name;
    unsigned dimensions = 1;
    std::vector<double> xEdges;
    std::vector<double> yEdges;
    std::vector<double> zEdges;
};

struct ModelInputPlanEntry {
    std::string name;
    std::string dtype;
    std::vector<std::int64_t> shape;
};

class ExecutionPlan {
public:
    const BranchPlanEntry &column(ColumnId id) const {
        return columns_.at(id.value());
    }
    const std::string &trigger(TriggerId id) const {
        return triggers_.at(id.value());
    }
    const CorrectionPlanEntry &correction(CorrectionId id) const {
        return corrections_.at(id.value());
    }
    const SystematicPlanEntry &systematic(SystematicId id) const {
        return systematics_.at(id.value());
    }
    const HistogramPlanEntry &histogram(HistId id) const {
        return histograms_.at(id.value());
    }
    const ModelInputPlanEntry &modelInput(ModelInputId id) const {
        return modelInputs_.at(id.value());
    }
    const std::vector<BranchPlanEntry> &columns() const noexcept {
        return columns_;
    }
    const std::vector<SystematicId> &systematicOrder() const noexcept {
        return systematicOrder_;
    }
    std::size_t triggerCount() const noexcept { return triggers_.size(); }
    std::size_t correctionCount() const noexcept { return corrections_.size(); }
    std::size_t histogramCount() const noexcept { return histograms_.size(); }
    std::size_t modelInputCount() const noexcept { return modelInputs_.size(); }

private:
    friend class ExecutionPlanBuilder;
    std::vector<BranchPlanEntry> columns_;
    std::vector<std::string> triggers_;
    std::vector<CorrectionPlanEntry> corrections_;
    std::vector<SystematicPlanEntry> systematics_;
    std::vector<SystematicId> systematicOrder_;
    std::vector<HistogramPlanEntry> histograms_;
    std::vector<ModelInputPlanEntry> modelInputs_;
};

class ExecutionPlanBuilder {
public:
    ColumnId column(std::string name, PlanRequirement requirement,
                    std::string leafType = {},
                    PlanCardinality cardinality = PlanCardinality::Any) {
        const auto found = columnIds_.find(name);
        if (found != columnIds_.end()) {
            auto &prior = plan_.columns_[found->second.value()];
            if ((!leafType.empty() && !prior.leafType.empty() &&
                 leafType != prior.leafType) ||
                (cardinality != PlanCardinality::Any &&
                 prior.cardinality != PlanCardinality::Any &&
                 cardinality != prior.cardinality)) {
                throw ConfigError("[ExecutionPlan] conflicting branch manifest for '" +
                                  name + "'");
            }
            if (requirement == PlanRequirement::Required)
                prior.requirement = PlanRequirement::Required;
            if (prior.leafType.empty())
                prior.leafType = std::move(leafType);
            if (prior.cardinality == PlanCardinality::Any)
                prior.cardinality = cardinality;
            return found->second;
        }
        const ColumnId id(static_cast<std::uint32_t>(plan_.columns_.size()));
        columnIds_.emplace(name, id);
        plan_.columns_.push_back(
            {std::move(name), std::move(leafType), requirement, cardinality});
        return id;
    }

    TriggerId trigger(std::string name) {
        return intern(name, triggerIds_, plan_.triggers_);
    }

    CorrectionId correction(std::string name, std::vector<ColumnId> inputs,
                            std::vector<double> invariantArguments = {}) {
        const auto found = correctionIds_.find(name);
        if (found != correctionIds_.end())
            throw ConfigError("[ExecutionPlan] duplicate correction '" + name + "'");
        for (const auto id : inputs)
            static_cast<void>(plan_.columns_.at(id.value()));
        const CorrectionId id(
            static_cast<std::uint32_t>(plan_.corrections_.size()));
        correctionIds_.emplace(name, id);
        plan_.corrections_.push_back(
            {std::move(name), std::move(inputs), std::move(invariantArguments)});
        return id;
    }

    SystematicId systematic(std::string name,
                            std::vector<SystematicId> dependencies = {}) {
        const auto found = systematicIds_.find(name);
        if (found != systematicIds_.end())
            throw ConfigError("[ExecutionPlan] duplicate systematic '" + name + "'");
        for (const auto id : dependencies)
            static_cast<void>(plan_.systematics_.at(id.value()));
        const SystematicId id(
            static_cast<std::uint32_t>(plan_.systematics_.size()));
        systematicIds_.emplace(name, id);
        plan_.systematics_.push_back({std::move(name), std::move(dependencies)});
        return id;
    }

    HistId histogram(HistogramPlanEntry entry) {
        const auto found = histogramIds_.find(entry.name);
        if (found != histogramIds_.end())
            throw ConfigError("[ExecutionPlan] duplicate histogram '" + entry.name + "'");
        if (entry.dimensions < 1 || entry.dimensions > 3 ||
            entry.xEdges.size() < 2 ||
            (entry.dimensions >= 2 && entry.yEdges.size() < 2) ||
            (entry.dimensions == 3 && entry.zEdges.size() < 2))
            throw ConfigError("[ExecutionPlan] invalid histogram schema for '" +
                              entry.name + "'");
        const HistId id(static_cast<std::uint32_t>(plan_.histograms_.size()));
        histogramIds_.emplace(entry.name, id);
        plan_.histograms_.push_back(std::move(entry));
        return id;
    }

    ModelInputId modelInput(ModelInputPlanEntry entry) {
        const auto found = modelInputIds_.find(entry.name);
        if (found != modelInputIds_.end())
            throw ConfigError("[ExecutionPlan] duplicate model input '" + entry.name + "'");
        if (entry.dtype.empty() || entry.shape.empty())
            throw ConfigError("[ExecutionPlan] invalid model input '" + entry.name + "'");
        const ModelInputId id(
            static_cast<std::uint32_t>(plan_.modelInputs_.size()));
        modelInputIds_.emplace(entry.name, id);
        plan_.modelInputs_.push_back(std::move(entry));
        return id;
    }

    ExecutionPlan compile() {
        plan_.systematicOrder_.clear();
        std::vector<unsigned char> state(plan_.systematics_.size(), 0);
        for (std::uint32_t index = 0; index < plan_.systematics_.size(); ++index)
            visitSystematic(SystematicId(index), state);
        return plan_;
    }

private:
    template <typename Id>
    static Id intern(const std::string &name,
                     std::unordered_map<std::string, Id> &ids,
                     std::vector<std::string> &values) {
        const auto found = ids.find(name);
        if (found != ids.end())
            return found->second;
        const Id id(static_cast<std::uint32_t>(values.size()));
        ids.emplace(name, id);
        values.push_back(name);
        return id;
    }

    void visitSystematic(SystematicId id, std::vector<unsigned char> &state) {
        auto &current = state.at(id.value());
        if (current == 2)
            return;
        if (current == 1)
            throw ConfigError("[ExecutionPlan] systematic dependency cycle");
        current = 1;
        for (const auto dependency : plan_.systematics_.at(id.value()).dependencies)
            visitSystematic(dependency, state);
        current = 2;
        plan_.systematicOrder_.push_back(id);
    }

    ExecutionPlan plan_;
    std::unordered_map<std::string, ColumnId> columnIds_;
    std::unordered_map<std::string, TriggerId> triggerIds_;
    std::unordered_map<std::string, CorrectionId> correctionIds_;
    std::unordered_map<std::string, SystematicId> systematicIds_;
    std::unordered_map<std::string, HistId> histogramIds_;
    std::unordered_map<std::string, ModelInputId> modelInputIds_;
};

} // namespace SKNano

#endif

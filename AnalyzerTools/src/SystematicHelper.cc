#include "SystematicHelper.h"
#include <yaml-cpp/yaml.h>
#include <algorithm>

namespace {

bool ends_with(const std::string &value, const std::string &suffix) {
    if (value.size() < suffix.size()) return false;
    return value.compare(value.size() - suffix.size(), suffix.size(), suffix) == 0;
}

std::string strip_variation_suffix(const std::string &syst_name) {
    if (ends_with(syst_name, "_Up")) {
        return syst_name.substr(0, syst_name.size() - 3);
    }
    if (ends_with(syst_name, "_Down")) {
        return syst_name.substr(0, syst_name.size() - 5);
    }
    return syst_name;
}

const char *variation_suffix(MyCorrection::variation variation) {
    switch (variation) {
    case MyCorrection::variation::up:
        return "_Up";
    case MyCorrection::variation::down:
        return "_Down";
    default:
        return "";
    }
}

} // namespace

SystematicHelper::CompiledVariationPlan
SystematicHelper::compileVariationPlan() const {
    CompiledVariationPlan compiled;
    compiled.lanes = systematics_evtLoopAgain;
    compiled.plan.lanes.reserve(compiled.lanes.size());
    for (std::size_t index = 0; index < compiled.lanes.size(); ++index)
        compiled.plan.lanes.emplace_back(
            SKNano::SystematicId(static_cast<std::uint32_t>(index)));
    return compiled;
}

SystematicHelper::SystematicHelper(std::string yaml_path,
    TString sample,
    TString Era)
: sample(sample.Data()),
Era(Era.Data()){
    YAML::Node config = YAML::LoadFile(yaml_path);
    for (const auto &node : config["systematics"])
    {
        std::string syst_name = node["syst"].as<std::string>();
        SystematicHelper::SYST syst;
        syst.syst = syst_name;
        syst.raw_name = syst_name;
        if (node["decorrelate_by_era"].IsDefined())
        {
            syst.decorrelate_by_era = node["decorrelate_by_era"].as<bool>();
        }
        if(syst.decorrelate_by_era){
            syst.syst = syst.syst + "_" + Era.Data();
        }
        if (node["source"].IsDefined())
        {
            syst.source = node["source"].as<std::string>();
        }
        if (node["evtLoopAgain"].IsDefined())
        {
            syst.evtLoopAgain = node["evtLoopAgain"].as<bool>();
        }
        if (node["oneSided"].IsDefined())
        {
            syst.oneSided = node["oneSided"].as<bool>();
        }
        if (node["hasDedicatedSample"].IsDefined())
        {
            syst.hasDedicatedSample = node["hasDedicatedSample"].as<bool>();
        }
        if (node["dedicatedSampleKey_up"].IsDefined())
        {
            syst.dedicatedSampleKey_up = node["dedicatedSampleKey_up"].as<std::string>();
        }
        if (node["dedicatedSampleKey_down"].IsDefined())
        {
            syst.dedicatedSampleKey_down = node["dedicatedSampleKey_down"].as<std::string>();
        }
        if (node["target"].IsDefined())
        {
            syst.target = node["target"].as<std::string>();
        }
        else
        {
            syst.target = syst_name;
        }

        systematics.push_back(syst);
    }

    rebuildSystLookup();

    for (const auto &node : config["correlations"])
    {
        CORRELATION correlation;
        std::string name = node["correlation"].as<std::string>();
        correlation.name = name;
        std::string rep_name;
        for (unsigned int i = 0; i < node["sources"].size(); i++)
        {
            if (i == 0)
            {
                rep_name = node["sources"][i].as<std::string>();
                correlation.rep_name = rep_name;
            }
            else
            {
                correlation.child_syst_names.insert(node["sources"][i].as<std::string>());
            }
            std::string source = node["sources"][i].as<std::string>();
            SystematicHelper::SYST *source_ptr = findSystematic(source);
            correlation.sources.push_back(source_ptr);
            if (i == 0)
            {
                correlation.rep_ptr = source_ptr;
            }
            else
            {
                correlation.child_ptrs.push_back(source_ptr);
            }
            correlation.member_ptrs.push_back(source_ptr);
        }
        correlations[name] = correlation;
        correlation_order.push_back(name);
    }

    make_map_dedicatedSample();
    isDedicatedSample = IsDedicatedSample();
    checkBadSystematics();
    make_Iter_obj_EvtLoopAgain();
    buildSchedule();
    current_lane = 0; // Central
    std::cout << "[SystematicHelper::SystematicHelper] SystematicHelper is created for " <<  sample << std::endl;
    std::cout << "[SystematicHelper::SystematicHelper] Iterator has " << systematics_evtLoopAgain.size() << " elements" << std::endl;
    for (const auto &syst : systematics_evtLoopAgain)
    {
        std::cout << "[SystematicHelper::SystematicHelper] " << syst.iter_name << " " << syst.syst_name << " " << variation_suffix(syst.variation) << std::endl;
    }
    std::cout << "[SystematicHelper::SystematicHelper] All systematics are: " << std::endl;
    for (const auto &syst : systematics)
    {
        std::cout << "[SystematicHelper::SystematicHelper] " << syst.syst << " " << syst.source << " " << syst.evtLoopAgain << " " << syst.oneSided << " " << syst.hasDedicatedSample << " " << syst.dedicatedSampleKey_up << " " << syst.dedicatedSampleKey_down << std::endl;
    }
    std::cout << "[SystematicHelper::SystematicHelper] isDedicatedSample: " << isDedicatedSample << std::endl;
    std::cout << "[SystematicHelper::SystematicHelper] Current systematic: " << getCurrentIter().iter_name << std::endl;
    std::cout << "[SystematicHelper::SystematicHelper] Current systematic source: " << getCurrentIter().syst_name << std::endl;
    std::cout << "[SystematicHelper::SystematicHelper] Current systematic variation: " << variation_suffix(getCurrentIter().variation) << std::endl;
}

SystematicHelper::~SystematicHelper() {}

SystematicHelper::SYST *SystematicHelper::findSystematic(std::string syst_name)
{
    auto it_lookup = syst_lookup.find(syst_name);
    if (it_lookup != syst_lookup.end())
        return it_lookup->second;

    for (auto &syst : systematics)
    {
        if (syst.syst == syst_name)
        {
            return &syst;
        }

        // Check if syst.syst ends with "_" + Era and the prefix matches
        std::string suffix = "_" + Era;
        if (syst.syst.size() > suffix.size() &&
            syst.syst.compare(syst.syst.size() - suffix.size(), suffix.size(), suffix) == 0)
        {
            std::string prefix = syst.syst.substr(0, syst.syst.size() - suffix.size());
            if (prefix == syst_name)
            {
                return &syst;
            }
        }
    }
    return nullptr;
}

void SystematicHelper::checkBadSystematics()
{
    bool badSysts = false;
    for (const auto &syst : systematics)
    {
        // check 1. if hasDedicatedSample is true, then dedicatedSampleKey_up and dedicatedSampleKey_down must be defined
        if (syst.hasDedicatedSample)
        {
            if (syst.dedicatedSampleKey_up == "" || syst.dedicatedSampleKey_down == "")
            {
                std::cerr << "Systematic " << syst.syst << " has hasDedicatedSample set to true, but dedicatedSampleKey_up and dedicatedSampleKey_down are not defined" << std::endl;
                badSysts = true;
            }
        }

        // check 2. if evtLoopAgain is true, hasDedicatedSample must be false
        if (syst.evtLoopAgain && syst.hasDedicatedSample)
        {
            std::cerr << "Systematic " << syst.syst << " has evtLoopAgain set to true, but hasDedicatedSample is also true" << std::endl;
            badSysts = true;
        }
    }

    for (const auto &correlation : correlations)
    {
        for (const auto &source : correlation.second.sources)
        {
            if (source == nullptr)
            {
                std::cerr << "Correlation " << correlation.first << " has a source that is not defined" << std::endl;
                badSysts = true;
            }
        }
    }

    if (badSysts)
    {
        throw SKNano::ConfigError("Bad systematics found");
    }
}

void SystematicHelper::make_Iter_obj_EvtLoopAgain()
{

    SystematicHelper::Iter_obj obj_central;
    obj_central.iter_name = central_name;
    obj_central.syst_name = central_name;
    obj_central.syst_source = "total";
    obj_central.variation = MyCorrection::variation::nom;
    systematics_evtLoopAgain.push_back(obj_central);
    lane_syst_index.push_back(-1);

    // if systematic sample is provided, only loop over central
    if (isDedicatedSample)
        return;

    for (std::size_t index = 0; index < systematics.size(); ++index)
    {
        const auto &syst = systematics[index];
        if (syst.evtLoopAgain)
        {
            SystematicHelper::Iter_obj obj_up;
            SystematicHelper::Iter_obj obj_down;
            obj_up.iter_name = syst.syst + variation_suffix(MyCorrection::variation::up);
            obj_down.iter_name = syst.syst + variation_suffix(MyCorrection::variation::down);
            obj_up.syst_name = syst.syst;
            obj_up.syst_source = syst.source;
            obj_up.variation = MyCorrection::variation::up;
            obj_down.syst_name = syst.syst;
            obj_down.syst_source = syst.source;
            obj_down.variation = MyCorrection::variation::down;

            systematics_evtLoopAgain.push_back(obj_up);
            systematics_evtLoopAgain.push_back(obj_down);
            lane_syst_index.push_back(static_cast<std::ptrdiff_t>(index));
            lane_syst_index.push_back(static_cast<std::ptrdiff_t>(index));
        }
        else if (!syst.hasDedicatedSample)
        {
            // Store weight-only systematics (those without evtLoopAgain and without dedicated samples)
            systematics_weightOnly.push_back(syst.syst);
        }
    }
}

void SystematicHelper::make_map_dedicatedSample()
{
    for (const auto &syst : systematics)
    {
        if (syst.hasDedicatedSample)
        {
            map_dedicatesamplekey_systname[syst.dedicatedSampleKey_up] = syst.syst + variation_suffix(MyCorrection::variation::up);
            map_dedicatesamplekey_systname[syst.dedicatedSampleKey_down] = syst.syst + variation_suffix(MyCorrection::variation::down);
        }
    }
}

std::size_t SystematicHelper::indexOfSystematic(const SYST *syst) const
{
    const std::ptrdiff_t offset = syst - systematics.data();
    if (offset < 0 || static_cast<std::size_t>(offset) >= systematics.size())
        throw SKNano::ConfigError(
            "[SystematicHelper] correlation refers to an unknown systematic");
    return static_cast<std::size_t>(offset);
}

// Resolves everything that does not depend on the event: which nominal factor
// each systematic owns, which systematics move together, and the exact key set
// each event-loop lane reports.  The event loop then only multiplies floats.
void SystematicHelper::buildSchedule()
{
    const std::size_t syst_count = systematics.size();

    target_names.clear();
    target_index.clear();
    syst_slots.assign(syst_count, SystSlot{});
    for (std::size_t index = 0; index < syst_count; ++index)
    {
        const auto &syst = systematics[index];
        auto inserted = target_index.emplace(syst.target, target_names.size());
        if (inserted.second)
            target_names.push_back(syst.target);

        SystSlot &slot = syst_slots[index];
        slot.target_slot = inserted.first->second;
        slot.source = syst.source.c_str();
        slot.kind = (syst.hasDedicatedSample || syst.evtLoopAgain)
                        ? WeightKind::Inert
                        : (syst.oneSided ? WeightKind::OneSided
                                         : WeightKind::TwoSided);
    }

    const std::size_t target_count = target_names.size();
    nominal_factors.assign(target_count, 1.);
    up_factors.assign(syst_count, 1.);
    down_factors.assign(syst_count, 1.);
    prefix.assign(target_count + 1, 1.);
    suffix.assign(target_count + 1, 1.);
    twosided_fn.assign(syst_count, nullptr);
    onesided_fn.assign(syst_count, nullptr);

    // Correlations, kept in YAML declaration order so lane -> group resolution
    // is deterministic even when a systematic appears in several of them.
    corr_groups.clear();
    std::vector<std::ptrdiff_t> syst_group(syst_count, -1);
    for (const auto &name : correlation_order)
    {
        const CORRELATION &correlation = correlations.at(name);
        CorrGroup group;
        for (const SYST *member : correlation.member_ptrs)
        {
            if (!member)
                continue;
            const std::size_t index = indexOfSystematic(member);
            group.member_systs.push_back(index);
            group.excluded_targets.push_back(syst_slots[index].target_slot);
            if (syst_slots[index].kind == WeightKind::Inert)
                group.skipped = true;
            if (syst_group[index] < 0)
                syst_group[index] = static_cast<std::ptrdiff_t>(corr_groups.size());
        }
        std::sort(group.excluded_targets.begin(), group.excluded_targets.end());
        group.excluded_targets.erase(
            std::unique(group.excluded_targets.begin(), group.excluded_targets.end()),
            group.excluded_targets.end());
        corr_groups.push_back(std::move(group));
    }

    solo_entries.clear();
    for (std::size_t index = 0; index < syst_count; ++index)
    {
        if (syst_group[index] >= 0)
            continue;
        if (syst_slots[index].kind == WeightKind::Inert)
            continue;
        SoloEntry entry;
        entry.syst_slot = index;
        entry.target_slot = syst_slots[index].target_slot;
        solo_entries.push_back(entry);
    }

    // Pre-populate the per-lane result maps.  All insertions happen here so the
    // float* handles taken below stay valid for the lifetime of the helper.
    const std::size_t lane_count = systematics_evtLoopAgain.size();
    lane_results.assign(lane_count, {});
    lane_recipes.assign(lane_count, LaneRecipe{});

    auto &central_result = lane_results[0];
    lane_recipes[0].central = true;
    if (isDedicatedSample)
    {
        central_result.emplace(dedicated_iter_name, 1.f);
    }
    else
    {
        central_result.emplace(central_name, 1.f);
        for (std::size_t group_index = 0; group_index < corr_groups.size(); ++group_index)
        {
            if (corr_groups[group_index].skipped)
                continue;
            const std::string &rep_name = correlations.at(correlation_order[group_index]).rep_name;
            central_result.emplace(rep_name + variation_suffix(MyCorrection::variation::up), 1.f);
            central_result.emplace(rep_name + variation_suffix(MyCorrection::variation::down), 1.f);
        }
        for (const auto &entry : solo_entries)
        {
            const std::string &syst_name = systematics[entry.syst_slot].syst;
            central_result.emplace(syst_name + variation_suffix(MyCorrection::variation::up), 1.f);
            central_result.emplace(syst_name + variation_suffix(MyCorrection::variation::down), 1.f);
        }
    }

    for (std::size_t lane = 1; lane < lane_count; ++lane)
    {
        lane_results[lane].emplace(systematics_evtLoopAgain[lane].iter_name, 1.f);
        const std::ptrdiff_t syst_index = lane_syst_index[lane];
        lane_recipes[lane].corr_group =
            syst_index >= 0 ? syst_group[static_cast<std::size_t>(syst_index)] : -1;
    }

    // Bind the output handles now that no further rehashing can occur.
    if (isDedicatedSample)
    {
        central_out = &central_result.at(dedicated_iter_name);
    }
    else
    {
        central_out = &central_result.at(central_name);
        for (std::size_t group_index = 0; group_index < corr_groups.size(); ++group_index)
        {
            if (corr_groups[group_index].skipped)
                continue;
            const std::string &rep_name = correlations.at(correlation_order[group_index]).rep_name;
            corr_groups[group_index].out_up =
                &central_result.at(rep_name + variation_suffix(MyCorrection::variation::up));
            corr_groups[group_index].out_down =
                &central_result.at(rep_name + variation_suffix(MyCorrection::variation::down));
        }
        for (auto &entry : solo_entries)
        {
            const std::string &syst_name = systematics[entry.syst_slot].syst;
            entry.out_up = &central_result.at(syst_name + variation_suffix(MyCorrection::variation::up));
            entry.out_down = &central_result.at(syst_name + variation_suffix(MyCorrection::variation::down));
        }
    }
    for (std::size_t lane = 1; lane < lane_count; ++lane)
        lane_recipes[lane].out = &lane_results[lane].at(systematics_evtLoopAgain[lane].iter_name);
}

void SystematicHelper::bindWeightFunctions()
{
    for (std::size_t index = 0; index < systematics.size(); ++index)
    {
        const auto &target = systematics[index].target;
        twosided_fn[index] = nullptr;
        onesided_fn[index] = nullptr;
        switch (syst_slots[index].kind)
        {
        case WeightKind::OneSided: {
            auto it = weight_functions_onesided.find(target);
            if (it != weight_functions_onesided.end())
                onesided_fn[index] = &it->second;
            break;
        }
        case WeightKind::TwoSided: {
            auto it = weight_functions.find(target);
            if (it != weight_functions.end())
                twosided_fn[index] = &it->second;
            break;
        }
        case WeightKind::Inert:
            break;
        }
    }
}

void SystematicHelper::assignWeightFunctionMap(const unordered_map<std::string, std::variant<std::function<float(MyCorrection::variation, TString)>, std::function<float()>>> &weight_function_map)
{
    std::vector<std::string> syst_no_weight_function;
    // clear weight functions
    weight_functions.clear();
    weight_functions_onesided.clear();
    for (const auto &syst : systematics)
    {
        if (syst.hasDedicatedSample || syst.evtLoopAgain)
            continue;

        auto it = weight_function_map.find(syst.target);
        if (it == weight_function_map.end())
        {
            syst_no_weight_function.push_back(syst.syst);
            continue;
        }

        const auto &weight_variant = it->second;

        auto assignOneSided = [&]()
        {
            if (weight_variant.index() != 1)
            {
                throw SKNano::ConfigError("Weight function for " + syst.syst + " is not a one-sided weight function");
            }
            weight_functions_onesided[syst.target] = std::get<std::function<float()>>(weight_variant);
        };

        auto assignTwoSided = [&]()
        {
            if (weight_variant.index() != 0)
            {
                throw SKNano::ConfigError("Weight function for " + syst.syst + " is not a two-sided weight function");
            }
            weight_functions[syst.target] = std::get<std::function<float(MyCorrection::variation, TString)>>(weight_variant);
        };

        syst.oneSided ? assignOneSided() : assignTwoSided();
    }
    if (syst_no_weight_function.size() > 0)
    {
        std::string missing = "Weight function for ";
        for (const auto &syst : syst_no_weight_function)
        {
            missing += syst + " ";
        }
        missing += "are not assigned";
        throw SKNano::ConfigError(missing);
    }
    bindWeightFunctions();
    weight_functions_assigned = true;
}

void SystematicHelper::evaluateWeightSlots(bool dry_run)
{
    for (std::size_t index = 0; index < syst_slots.size(); ++index)
    {
        const SystSlot &slot = syst_slots[index];
        switch (slot.kind)
        {
        case WeightKind::Inert:
            nominal_factors[slot.target_slot] = 1.;
            up_factors[index] = 1.;
            down_factors[index] = 1.;
            break;
        case WeightKind::OneSided: {
            float weight = 1.f;
            if (!dry_run)
            {
                if (!onesided_fn[index])
                    throw SKNano::ConfigError(
                        "[SystematicHelper] missing one-sided weight function for " +
                        systematics[index].syst);
                weight = (*onesided_fn[index])();
            }
            // A one-sided variation means "drop this correction", so the up
            // side removes it from the product and the down side is nominal.
            nominal_factors[slot.target_slot] = weight;
            up_factors[index] = 1.;
            down_factors[index] = weight;
            break;
        }
        case WeightKind::TwoSided: {
            float nominal = 1.f;
            float up = 1.f;
            float down = 1.f;
            if (!dry_run)
            {
                if (!twosided_fn[index])
                    throw SKNano::ConfigError(
                        "[SystematicHelper] missing weight function for " +
                        systematics[index].syst);
                const auto &weight_function = *twosided_fn[index];
                nominal = weight_function(MyCorrection::variation::nom, "total");
                up = weight_function(MyCorrection::variation::up, slot.source);
                down = weight_function(MyCorrection::variation::down, slot.source);
            }
            nominal_factors[slot.target_slot] = nominal;
            up_factors[index] = up;
            down_factors[index] = down;
            break;
        }
        }
    }
}

void SystematicHelper::buildNominalProducts()
{
    const std::size_t count = nominal_factors.size();
    for (std::size_t index = 0; index < count; ++index)
        prefix[index + 1] = prefix[index] * nominal_factors[index];
    suffix[count] = 1.;
    for (std::size_t index = count; index > 0; --index)
        suffix[index - 1] = nominal_factors[index - 1] * suffix[index];
}

// Product of every nominal factor whose target is not in `sorted_targets`,
// assembled from the prefix/suffix tables so no division is involved.
double SystematicHelper::productExcluding(
    const std::vector<std::size_t> &sorted_targets) const
{
    if (sorted_targets.empty())
        return prefix.back();
    double result = prefix[sorted_targets.front()];
    for (std::size_t position = 1; position < sorted_targets.size(); ++position)
        for (std::size_t index = sorted_targets[position - 1] + 1;
             index < sorted_targets[position]; ++index)
            result *= nominal_factors[index];
    return result * suffix[sorted_targets.back() + 1];
}

void SystematicHelper::fillCentralWeights()
{
    *central_out = static_cast<float>(prefix.back());
    if (isDedicatedSample)
        return;

    for (const auto &group : corr_groups)
    {
        if (group.skipped)
            continue;
        const double base = productExcluding(group.excluded_targets);
        double up = base;
        double down = base;
        for (const auto index : group.member_systs)
        {
            up *= up_factors[index];
            down *= down_factors[index];
        }
        *group.out_up = static_cast<float>(up);
        *group.out_down = static_cast<float>(down);
    }

    for (const auto &entry : solo_entries)
    {
        const double base = prefix[entry.target_slot] * suffix[entry.target_slot + 1];
        *entry.out_up = static_cast<float>(base * up_factors[entry.syst_slot]);
        *entry.out_down = static_cast<float>(base * down_factors[entry.syst_slot]);
    }
}

void SystematicHelper::fillLaneWeight(const LaneRecipe &recipe)
{
    if (recipe.corr_group < 0)
    {
        *recipe.out = static_cast<float>(prefix.back());
        return;
    }

    const CorrGroup &group = corr_groups[static_cast<std::size_t>(recipe.corr_group)];
    double weight = productExcluding(group.excluded_targets);
    const auto variation = getCurrentIter().variation;
    if (variation != MyCorrection::variation::nom)
    {
        const std::vector<double> &varied =
            variation == MyCorrection::variation::down ? down_factors : up_factors;
        for (const auto index : group.member_systs)
            weight *= varied[index];
    }
    *recipe.out = static_cast<float>(weight);
}

const std::unordered_map<std::string, float> &
SystematicHelper::calculateWeightRef(bool dry_run)
{
    if (!weight_functions_assigned && !dry_run)
    {
        throw SKNano::ConfigError("Weight functions are not assigned");
    }

    evaluateWeightSlots(dry_run);
    buildNominalProducts();

    const LaneRecipe &recipe = lane_recipes[current_lane];
    if (recipe.central)
        fillCentralWeights();
    else
        fillLaneWeight(recipe);

    return lane_results[current_lane];
}

std::unordered_map<std::string, float> SystematicHelper::calculateWeight(bool dry_run)
{
    return calculateWeightRef(dry_run);
}

bool SystematicHelper::IsDedicatedSample()
{
    for (const auto &dedicatedSample : map_dedicatesamplekey_systname)
    {
        if (sample.find(dedicatedSample.first) != std::string::npos)
        {
            dedicated_iter_name = dedicatedSample.second;
            return true;
        }
    }
    return false;
}

std::vector<std::string> SystematicHelper::get_targets_from_name(const std::string &syst_name){
    // remove variation prefix
    std::vector<std::string> targets;
    if(syst_name == central_name){
        targets.push_back(central_name);
        return targets;
    }
    std::string syst_name_no_variation = strip_variation_suffix(syst_name);
    SYST *syst = findSystematic(syst_name_no_variation);

    //check if syst is in correlation table
    for (const auto &correlation : correlations)
    {
        if (correlation.second.rep_name == syst_name_no_variation)
        {
            if (correlation.second.rep_ptr)
                targets.push_back(correlation.second.rep_ptr->target);
            for (const auto *child_syst : correlation.second.child_ptrs)
            {
                if (child_syst)
                    targets.push_back(child_syst->target);
            }
            return targets;
        }
    }
    if (syst)
        targets.push_back(syst->target);
    return targets;
}

std::vector<std::string> SystematicHelper::get_sources_from_name(const std::string &syst_name){
    // remove variation prefix
    std::vector<std::string> sources;
    if (syst_name == central_name){
        sources.push_back(central_name);
        return sources;
    }
    std::string syst_name_no_variation = strip_variation_suffix(syst_name);

    SYST *syst = findSystematic(syst_name_no_variation);

    //check if syst is in correlation table
    for (const auto &correlation : correlations)
    {
        if (correlation.second.rep_name == syst_name_no_variation)
        {
            if (correlation.second.rep_ptr)
                sources.push_back(correlation.second.rep_ptr->source);
            for (const auto *child_syst : correlation.second.child_ptrs)
            {
                if (child_syst)
                    sources.push_back(child_syst->source);
            }
            return sources;
        }
    }
    if (syst)
        sources.push_back(syst->source);
    return sources;
}

MyCorrection::variation SystematicHelper::get_variation_from_name(const std::string &syst_name){
    // find variation from explicit suffix
    if (syst_name == central_name){
        return MyCorrection::variation::nom;
    }
    if (ends_with(syst_name, "_Up")) {
        return MyCorrection::variation::up;
    }
    if (ends_with(syst_name, "_Down")) {
        return MyCorrection::variation::down;
    }
    throw std::runtime_error("[SystematicHelper::get_variation_from_name] weird syst_name");
}

void SystematicHelper::rebuildSystLookup()
{
    syst_lookup.clear();
    for (auto &syst : systematics)
    {
        syst_lookup[syst.syst] = &syst;
        syst_lookup[syst.raw_name] = &syst;
    }
}

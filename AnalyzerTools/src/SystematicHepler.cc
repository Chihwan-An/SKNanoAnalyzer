#include "SystematicHelper.h"
#include <yaml-cpp/yaml.h>

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

} // namespace

SystematicHelper::SystematicHelper(std::string yaml_path,
    TString sample,
    TString Era)
: sample(sample.Data()),  
Era(Era.Data()){
    variation_prefix = {
        {MyCorrection::variation::nom, ""},
        {MyCorrection::variation::up, "_Up"},
        {MyCorrection::variation::down, "_Down"}};

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
    }

    make_map_dedicatedSample();
    isDedicatedSample = IsDedicatedSample();
    checkBadSystematics();
    make_Iter_obj_EvtLoopAgain();
    current_Iter_obj.clone(systematics_evtLoopAgain[0]); // Central
    std::cout << "[SystematicHelper::SystematicHelper] SystematicHelper is created for " <<  sample << std::endl;
    std::cout << "[SystematicHelper::SystematicHelper] Iterator has " << systematics_evtLoopAgain.size() << " elements" << std::endl;
    for (const auto &syst : systematics_evtLoopAgain)
    {
        std::cout << "[SystematicHelper::SystematicHelper] " << syst.iter_name << " " << syst.syst_name << " " << variation_prefix[syst.variation] << std::endl;
    }
    std::cout << "[SystematicHelper::SystematicHelper] All systematics are: " << std::endl;
    for (const auto &syst : systematics)
    {
        std::cout << "[SystematicHelper::SystematicHelper] " << syst.syst << " " << syst.source << " " << syst.evtLoopAgain << " " << syst.oneSided << " " << syst.hasDedicatedSample << " " << syst.dedicatedSampleKey_up << " " << syst.dedicatedSampleKey_down << std::endl;
    }
    std::cout << "[SystematicHelper::SystematicHelper] isDedicatedSample: " << isDedicatedSample << std::endl;
    std::cout << "[SystematicHelper::SystematicHelper] Current systematic: " << current_Iter_obj.iter_name << std::endl;
    std::cout << "[SystematicHelper::SystematicHelper] Current systematic source: " << current_Iter_obj.syst_name << std::endl;
    std::cout << "[SystematicHelper::SystematicHelper] Current systematic variation: " << variation_prefix[current_Iter_obj.variation] << std::endl;
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
        throw std::runtime_error("Bad systematics found");
        exit(1);
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

    // if systematic sample is provided, only loop over central
    if (isDedicatedSample)
        return;

    for (const auto &syst : systematics)
    {
        if (syst.evtLoopAgain)
        {
            SystematicHelper::Iter_obj obj_up;
            SystematicHelper::Iter_obj obj_down;
            obj_up.iter_name = syst.syst + variation_prefix[MyCorrection::variation::up];
            obj_down.iter_name = syst.syst + variation_prefix[MyCorrection::variation::down];
            obj_up.syst_name = syst.syst;
            obj_up.syst_source = syst.source;
            obj_up.variation = MyCorrection::variation::up;
            obj_down.syst_name = syst.syst;
            obj_down.syst_source = syst.source;
            obj_down.variation = MyCorrection::variation::down;

            systematics_evtLoopAgain.push_back(obj_up);
            systematics_evtLoopAgain.push_back(obj_down);
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
            map_dedicatesamplekey_systname[syst.dedicatedSampleKey_up] = syst.syst + variation_prefix[MyCorrection::variation::up];
            map_dedicatesamplekey_systname[syst.dedicatedSampleKey_down] = syst.syst + variation_prefix[MyCorrection::variation::down];
        }
    }
}

void SystematicHelper::assignWeightFunctionMap(const unordered_map<std::string, std::variant<std::function<float(MyCorrection::variation, TString)>, std::function<float()>>> &weight_function_map)
{
    std::vector<std::string> syst_no_weight_function;
    // clear weight functions
    weight_functions.clear();
    weight_functions_onesided.clear();
    std::unordered_set<std::string> all_targets;
    for (const auto &syst : systematics)
        all_targets.insert(syst.target);
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
                std::cerr << "Weight function for " << syst.syst << " is not a one-sided weight function" << std::endl;
                exit(1);
            }
            weight_functions_onesided[syst.target] = std::get<std::function<float()>>(weight_variant);
        };

        auto assignTwoSided = [&]()
        {
            if (weight_variant.index() != 0)
            {
                std::cerr << "Weight function for " << syst.syst << " is not a two-sided weight function" << std::endl;
                exit(1);
            }
            weight_functions[syst.target] = std::get<std::function<float(MyCorrection::variation, TString)>>(weight_variant);
        };

        syst.oneSided ? assignOneSided() : assignTwoSided();
    }
    if (syst_no_weight_function.size() > 0)
    {
        std::cerr << "Weight function for " << std::endl;
        for (const auto &syst : syst_no_weight_function)
        {
            std::cerr << syst << std::endl;
        }
        std::cerr << "are not assigned" << std::endl;
        exit(1);
    }
    weight_functions_assigned = true;
}

std::unordered_map<std::string, float> SystematicHelper::calculateWeight(bool dry_run)
{
    if(!weight_functions_assigned && !dry_run)
    {
        std::cerr << "Weight functions are not assigned" << std::endl;
        exit(1);
    }
    for (const auto &syst : systematics)
    {
        if (syst.hasDedicatedSample || syst.evtLoopAgain)
        {
            weight_map_nominal[syst.target] = 1.;
            weight_map_up[syst.target] = 1.;
            weight_map_down[syst.target] = 1.;
        }
        else if (syst.oneSided)
        {
            float weight = 1.;
            auto weight_function = weight_functions_onesided[syst.target];
            if(!dry_run) weight = weight_function();

            weight_map_nominal[syst.target] = weight;
            weight_map_up[syst.syst] = 1.;
            weight_map_down[syst.syst] = weight;
        }
        else
        {
            float weight_up = 1.;
            float weight_down = 1.;
            float weight_nominal = 1.;

            auto weight_function = weight_functions[syst.target];
            if(!dry_run)
            {
                weight_nominal = weight_function(MyCorrection::variation::nom, "total");
                weight_up = weight_function(MyCorrection::variation::up, syst.source);
                weight_down = weight_function(MyCorrection::variation::down, syst.source);
            }

            weight_map_nominal[syst.target] = weight_nominal;
            weight_map_up[syst.syst] = weight_up;
            weight_map_down[syst.syst] = weight_down;
        }
    }
    unordered_map<std::string, float> weights;
    if (current_Iter_obj.variation == MyCorrection::variation::nom)
    {
        weights = calculateWeight_central_case();
    }
    else
    {
        weights = calculateWeight_non_central_case();
    }

    return weights;
}

unordered_map<std::string, float> SystematicHelper::calculateWeight_central_case()
{
    std::vector<string> all_weight_systs;
    unordered_map<std::string, float> weights;
    float nominal_weight = 1.;
    unordered_set<std::string> all_weight_targets;

    for (const auto &syst : systematics)
        all_weight_targets.insert(syst.target);

    for (const auto &target : all_weight_targets)
        nominal_weight *= weight_map_nominal[target];

    for (const auto &syst : systematics)
    {
        all_weight_systs.push_back(syst.syst);
    }

    weights[central_name] = nominal_weight;

    for (const auto &correlation : correlations)
    {
        const auto &member_ptrs = correlation.second.member_ptrs;
        float weight_up = nominal_weight;
        float weight_down = nominal_weight;
        // If current Iter_obj is Central and correlation include systematic that has dedicated sample of require evtLoopAgain, skip.
        // that correlation set will be calculated in that situation
        bool correlation_should_be_skipped = false;
        for (const auto *sources_in_table : member_ptrs)
        {
            if (!sources_in_table)
                continue;
            if (sources_in_table->hasDedicatedSample || sources_in_table->evtLoopAgain)
            {
                correlation_should_be_skipped = true;
                break;
            }
        }
        
        for (const auto *sources_in_table : member_ptrs)
        {
            if (!sources_in_table)
                continue;
            const std::string &this_target = sources_in_table->target;
            weight_up = safe_divide(weight_up, weight_map_nominal[this_target]);
            weight_down = safe_divide(weight_down, weight_map_nominal[this_target]);
            weight_up *= weight_map_up[sources_in_table->syst];
            weight_down *= weight_map_down[sources_in_table->syst];
            auto it = find(all_weight_systs.begin(), all_weight_systs.end(), sources_in_table->syst);
            if (it != all_weight_systs.end())
            {
                all_weight_systs.erase(it);
            }
        }
        if (correlation_should_be_skipped) continue;
        weights[correlation.second.rep_name + variation_prefix[MyCorrection::variation::up]] = weight_up;
        weights[correlation.second.rep_name + variation_prefix[MyCorrection::variation::down]] = weight_down;
    }

    // systematic that not in correlation table
    for (const auto &syst_name : all_weight_systs)
    {
        float weight_up = nominal_weight;
        float weight_down = nominal_weight;
        SystematicHelper::SYST *syst_ptr = findSystematic(syst_name);
        if (!syst_ptr)
            continue;
        if(syst_ptr->hasDedicatedSample || syst_ptr->evtLoopAgain)
        {
            continue;
        }
        const std::string &this_target = syst_ptr->target;
        weight_up = safe_divide(weight_up, weight_map_nominal[this_target]);
        weight_down = safe_divide(weight_down, weight_map_nominal[this_target]);
        weight_up *= weight_map_up[syst_ptr->syst];
        weight_down *= weight_map_down[syst_ptr->syst];
        weights[syst_ptr->syst + variation_prefix[MyCorrection::variation::up]] = weight_up;
        weights[syst_ptr->syst + variation_prefix[MyCorrection::variation::down]] = weight_down;
    }
    if(isDedicatedSample)
    {
        for (const auto &dedicatedSample : map_dedicatesamplekey_systname)
        {
            std::string this_key = dedicatedSample.first;
            // Check if sample is found as a substring of this_sample
            if (sample.find(this_key) != std::string::npos)
            {
                float central_weight = weights[central_name];
                weights.clear();
                weights[dedicatedSample.second] = central_weight; // Use value from "Central"
                return weights;
            }
        }
        throw std::runtime_error("[SystematicHelper::calculateWeight_central_case] Dedicated sample not found");
    }
    return weights;
}

unordered_map<std::string, float> SystematicHelper::calculateWeight_non_central_case()
{
    // first check current Iter_obj is in correlation table
    bool Iter_obj_in_correlation = false;
    CORRELATION this_correlation;
    unordered_map<std::string, float> weights;
    unordered_set<std::string> all_weight_targets;
    for (const auto &syst : systematics)
        all_weight_targets.insert(syst.target);
    for (const auto &correlation : correlations)
    {
        if (correlation.second.rep_name == current_Iter_obj.syst_name)
        {
            Iter_obj_in_correlation = true;
            this_correlation = correlation.second;
            break;
        }
        for (const auto &child_syst : correlation.second.child_syst_names)
        {
            if (child_syst == current_Iter_obj.syst_name)
            {
                Iter_obj_in_correlation = true;
                this_correlation = correlation.second;
                break;
            }
        }
    }

    weights[current_Iter_obj.iter_name] = 1.;
    for (const auto &target : all_weight_targets)
    {
        weights[current_Iter_obj.iter_name] *= weight_map_nominal[target];
    }
    if (!Iter_obj_in_correlation)
        return weights;
    else
    {

        switch (current_Iter_obj.variation)
        {
        case MyCorrection::variation::up:
            if (this_correlation.rep_ptr)
            {
                weights[current_Iter_obj.iter_name]  = safe_divide(weights[current_Iter_obj.iter_name], weight_map_nominal[this_correlation.rep_ptr->target]);
                weights[current_Iter_obj.iter_name] *= weight_map_up[this_correlation.rep_ptr->syst];
            }
            for (const auto *syst : this_correlation.child_ptrs)
            {
                if (!syst)
                    continue;
                weights[current_Iter_obj.iter_name] = safe_divide(weights[current_Iter_obj.iter_name] ,weight_map_nominal[syst->target]);
                weights[current_Iter_obj.iter_name] *= weight_map_up[syst->syst];
            }
            break;
        case MyCorrection::variation::down:
            if (this_correlation.rep_ptr)
            {
                weights[current_Iter_obj.iter_name] = safe_divide(weights[current_Iter_obj.iter_name], weight_map_nominal[this_correlation.rep_ptr->target]);
                weights[current_Iter_obj.iter_name] *= weight_map_down[this_correlation.rep_ptr->syst];
            }
            for (const auto *syst : this_correlation.child_ptrs)
            {
                if (!syst)
                    continue;
                weights[current_Iter_obj.iter_name] = safe_divide(weights[current_Iter_obj.iter_name], weight_map_nominal[syst->target]);
                weights[current_Iter_obj.iter_name] *= weight_map_down[syst->syst];
            }
            break;
        default:
            break;
        }
    }
    return weights;
}

bool SystematicHelper::IsDedicatedSample()
{ 
    for (const auto &dedicatedSample : map_dedicatesamplekey_systname)
    {
        if (sample.find(dedicatedSample.first) != std::string::npos)
        {
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

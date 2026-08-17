#ifndef SystematicHelper_h
#define SystematicHelper_h
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <vector>
#include "ROOT/RVec.hxx"
#include "MyCorrection.h"
#include "VariationKernel.h"
#include <variant>

using namespace ROOT::VecOps;
using namespace std;

typedef std::unordered_map<std::string, std::variant<std::function<float(MyCorrection::variation, std::string)>, std::function<float()>>> weight_function_map;

class SystematicHelper
{
public:
    struct SYST
    {
        std::string syst;
        std::string raw_name;
        std::string source = "total";
        std::string target = "";
        bool evtLoopAgain = false;
        bool oneSided = false;
        bool hasDedicatedSample = false;
        bool decorrelate_by_era = false;
        std::string dedicatedSampleKey_up = "";
        std::string dedicatedSampleKey_down = "";
    };

    struct CORRELATION
    {
        std::string name;
        std::string rep_name;                        // the first one in the sources
        unordered_set<std::string> child_syst_names; // the rest of the sources
        RVec<SYST *> sources;
        SYST *rep_ptr = nullptr;
        std::vector<SYST *> member_ptrs;
        std::vector<SYST *> child_ptrs;
    };

    struct Iter_obj
    {
        std::string iter_name;
        std::string syst_name;
        std::string syst_source;
        MyCorrection::variation variation;
    };

    struct CompiledVariationPlan {
        SKNano::VariationPlan plan;
        std::vector<Iter_obj> lanes;
    };

    SystematicHelper(std::string yaml_path, TString sample, TString Era);
    ~SystematicHelper();
    SYST* findSystematic(std::string syst_name);
    void assignWeightFunctionMap(const unordered_map < std::string, std::variant<std::function<float(MyCorrection::variation, TString)>, std::function<float()>>> &weight_functions);
    class EventLoopIterator
    {
    public:
        using Iterator = std::vector<Iter_obj>::iterator;

        EventLoopIterator(Iterator it, SystematicHelper *parent) : iter(it), parent(parent) { sync(); }

        EventLoopIterator &operator++()
        {
            ++iter;
            sync();
            return *this;
        }

        bool operator!=(const EventLoopIterator &other) const
        {
            return iter != other.iter;
        }

        const Iter_obj &operator*() const
        {
            return *iter;
        }

    private:
        void sync()
        {
            if (iter != parent->systematics_evtLoopAgain.end())
                parent->current_lane = static_cast<std::size_t>(
                    iter - parent->systematics_evtLoopAgain.begin());
        }

        Iterator iter;
        SystematicHelper *parent;
    };


    EventLoopIterator begin()
    {
        return EventLoopIterator(systematics_evtLoopAgain.begin(), this);
    }

    // Method to return the end iterator
    EventLoopIterator end()
    {
        return EventLoopIterator(systematics_evtLoopAgain.end(), this);
    }

    inline const Iter_obj &getCurrentIter() const { return systematics_evtLoopAgain[current_lane]; }
    inline std::string getCurrentSysName() const { return getCurrentIter().iter_name; }
    inline std::string getCurrentIterSysTarget() const { return getCurrentIter().syst_name; }
    inline std::string getCurrentIterSysSource() const { return getCurrentIter().syst_source; }
    inline const std::vector<std::string>& getWeightOnlySystematics() const { return systematics_weightOnly; }
    inline MyCorrection::variation getCurrentIterVariation() const { return getCurrentIter().variation; }

    std::vector<std::string> get_targets_from_name(const std::string &syst_name);
    std::vector<std::string> get_sources_from_name(const std::string &syst_name);
    MyCorrection::variation get_variation_from_name(const std::string &syst_name);
    CompiledVariationPlan compileVariationPlan() const;

    // Recomputes the weights of the current event-loop lane and returns a
    // reference to the helper-owned result map.  The key set is fixed at
    // construction, so only the values change from event to event; prefer this
    // over calculateWeight() in hot loops to avoid rebuilding the map.
    const std::unordered_map<std::string, float> &calculateWeightRef(bool dry_run = false);
    std::unordered_map<std::string, float> calculateWeight(bool dry_run = false);

private:
    // A systematic either contributes no weight at all (it is realised by a
    // dedicated sample or by re-running the event loop), or it owns a
    // one-sided / two-sided weight function.
    enum class WeightKind : std::uint8_t { Inert, OneSided, TwoSided };

    // Per-systematic data needed inside the event loop, resolved once at
    // construction so no string lookup happens per event.
    struct SystSlot
    {
        std::size_t target_slot = 0;
        WeightKind kind = WeightKind::Inert;
        TString source = "total";
    };

    // A set of systematics that must be varied coherently.  The nominal
    // factors of every member target are dropped from the product and replaced
    // by the members' up/down weights.
    struct CorrGroup
    {
        std::vector<std::size_t> member_systs;
        std::vector<std::size_t> excluded_targets; // sorted, unique
        bool skipped = false;
        float *out_up = nullptr;
        float *out_down = nullptr;
    };

    // A systematic that belongs to no correlation: only its own target factor
    // is replaced.
    struct SoloEntry
    {
        std::size_t syst_slot = 0;
        std::size_t target_slot = 0;
        float *out_up = nullptr;
        float *out_down = nullptr;
    };

    // What one event-loop lane has to write once the weight slots are known.
    struct LaneRecipe
    {
        bool central = false;
        std::ptrdiff_t corr_group = -1; // -1 when the lane's systematic is uncorrelated
        float *out = nullptr;
    };

    std::string central_name = "Central";
    void checkBadSystematics();
    void make_Iter_obj_EvtLoopAgain();
    void make_map_dedicatedSample();
    bool IsDedicatedSample();

    void buildSchedule();
    void bindWeightFunctions();
    std::size_t indexOfSystematic(const SYST *syst) const;
    void evaluateWeightSlots(bool dry_run);
    void buildNominalProducts();
    double productExcluding(const std::vector<std::size_t> &sorted_targets) const;
    void fillCentralWeights();
    void fillLaneWeight(const LaneRecipe &recipe);
    void rebuildSystLookup();

    std::vector<SYST> systematics;
    std::unordered_map<std::string, CORRELATION> correlations;
    std::vector<std::string> correlation_order; // declaration order of `correlations`
    std::unordered_map<std::string, std::function<float(MyCorrection::variation, TString)>> weight_functions;
    std::unordered_map<std::string, std::function<float()>> weight_functions_onesided;

    std::vector<Iter_obj> systematics_evtLoopAgain;
    std::vector<std::ptrdiff_t> lane_syst_index; //! systematic index per lane, -1 for Central
    std::vector<std::string> systematics_weightOnly;
    unordered_map<std::string, std::string> map_dedicatesamplekey_systname;
    std::string dedicated_iter_name;

    // --- compiled schedule, rebuilt only on construction / function binding ---
    std::vector<SystSlot> syst_slots;                              //!
    std::vector<std::string> target_names;                         //!
    std::unordered_map<std::string, std::size_t> target_index;     //!
    std::vector<CorrGroup> corr_groups;                            //!
    std::vector<SoloEntry> solo_entries;                           //!
    std::vector<LaneRecipe> lane_recipes;                          //!
    std::vector<std::unordered_map<std::string, float>> lane_results; //!
    std::vector<const std::function<float(MyCorrection::variation, TString)> *> twosided_fn; //!
    std::vector<const std::function<float()> *> onesided_fn;       //!
    float *central_out = nullptr;                                  //!

    // --- per-event scratch, allocated once ---
    std::vector<double> nominal_factors; //!
    std::vector<double> up_factors;      //!
    std::vector<double> down_factors;    //!
    std::vector<double> prefix;          //!
    std::vector<double> suffix;          //!

    bool isDedicatedSample;
    bool weight_functions_assigned = false;
    std::string sample;
    std::string Era;
    std::size_t current_lane = 0;
    std::unordered_map<std::string, SYST *> syst_lookup;
};

#endif

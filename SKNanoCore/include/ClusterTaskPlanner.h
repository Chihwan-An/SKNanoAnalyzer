#ifndef SKNanoCore_ClusterTaskPlanner_h
#define SKNanoCore_ClusterTaskPlanner_h

#include "AnalysisException.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace SKNano {

struct ClusterRange {
    std::int64_t begin = 0;
    std::int64_t end = 0;
};

struct ClusterTask {
    std::uint64_t taskId = 0;
    std::uint64_t fileToken = 0;
    std::int64_t begin = 0;
    std::int64_t end = 0;
};

class ClusterTaskPlanner {
public:
    static std::vector<ClusterTask>
    plan(std::uint64_t fileToken, const std::vector<ClusterRange> &clusters,
         std::size_t maximumEventsPerTask) {
        if (maximumEventsPerTask == 0)
            throw ConfigError("[ClusterTaskPlanner] maximum task size must be positive");
        std::vector<ClusterTask> tasks;
        std::int64_t priorEnd = -1;
        for (const auto &cluster : clusters) {
            if (cluster.begin < 0 || cluster.end <= cluster.begin ||
                (priorEnd >= 0 && cluster.begin < priorEnd))
                throw ConfigError("[ClusterTaskPlanner] invalid cluster ranges");
            for (std::int64_t begin = cluster.begin; begin < cluster.end;) {
                const std::int64_t end = std::min<std::int64_t>(
                    cluster.end,
                    begin + static_cast<std::int64_t>(maximumEventsPerTask));
                tasks.push_back({static_cast<std::uint64_t>(tasks.size()),
                                 fileToken, begin, end});
                begin = end;
            }
            priorEnd = cluster.end;
        }
        return tasks;
    }
};

} // namespace SKNano

#endif

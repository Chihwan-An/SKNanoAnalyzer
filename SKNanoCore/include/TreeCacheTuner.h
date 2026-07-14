#ifndef SKNanoCore_TreeCacheTuner_h
#define SKNanoCore_TreeCacheTuner_h

#include "AnalysisException.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>

namespace SKNano {

struct TreeCacheTuningInput {
    std::size_t requestedCacheBytes = 0;
    std::size_t totalMemoryBudgetBytes = 0;
    std::size_t eventBlockBudgetBytes = 0;
    std::uint64_t activeCompressedBytes = 0;
    std::uint64_t treeEntries = 0;
    std::uint64_t clusterEntries = 0;
    bool independentReadContext = false;
};

struct TreeCacheTuningResult {
    std::size_t cacheBytes = 0;
    std::size_t estimatedActiveClusterBytes = 0;
    bool asyncDoubleBufferAllowed = false;
};

class TreeCacheTuner {
public:
    static TreeCacheTuningResult tune(const TreeCacheTuningInput &input) {
        if (input.totalMemoryBudgetBytes == 0)
            throw ConfigError("[TreeCacheTuner] memory budget must be positive");
        const std::size_t reservedBlock =
            std::min(input.eventBlockBudgetBytes,
                     input.totalMemoryBudgetBytes);
        const std::size_t available =
            input.totalMemoryBudgetBytes - reservedBlock;
        TreeCacheTuningResult result;
        if (available == 0 || input.requestedCacheBytes == 0)
            return result;

        std::size_t footprint = 0;
        if (input.activeCompressedBytes && input.treeEntries &&
            input.clusterEntries) {
            const long double estimate =
                static_cast<long double>(input.activeCompressedBytes) *
                static_cast<long double>(input.clusterEntries) /
                static_cast<long double>(input.treeEntries);
            footprint = estimate >=
                    static_cast<long double>(std::numeric_limits<std::size_t>::max())
                ? std::numeric_limits<std::size_t>::max()
                : static_cast<std::size_t>(estimate + 0.999L);
        }
        result.estimatedActiveClusterBytes = footprint;
        constexpr std::size_t minimumUsefulCache = 1024 * 1024;
        std::size_t target = input.requestedCacheBytes;
        if (footprint) {
            const std::size_t doubled = footprint >
                    std::numeric_limits<std::size_t>::max() / 2
                ? std::numeric_limits<std::size_t>::max()
                : footprint * 2;
            target = std::max(minimumUsefulCache, doubled);
        }
        result.cacheBytes = std::min({target, input.requestedCacheBytes,
                                      available});
        result.asyncDoubleBufferAllowed = input.independentReadContext &&
            reservedBlock > 0 &&
            input.totalMemoryBudgetBytes - result.cacheBytes >=
                2 * std::min(reservedBlock,
                             input.totalMemoryBudgetBytes / 2);
        return result;
    }
};

} // namespace SKNano

#endif

#include "TreeCacheTuner.h"

#include <stdexcept>

namespace {
void require(bool condition, const char *message) {
    if (!condition) throw std::runtime_error(message);
}
}

int main() {
    const auto local = SKNano::TreeCacheTuner::tune({
        200ULL << 20, 256ULL << 20, 64ULL << 20,
        100ULL << 20, 1000, 100, false});
    require(local.estimatedActiveClusterBytes == 10ULL << 20 &&
                local.cacheBytes == 20ULL << 20 &&
                !local.asyncDoubleBufferAllowed,
            "cache must track two active compressed clusters");
    const auto bounded = SKNano::TreeCacheTuner::tune({
        200ULL << 20, 100ULL << 20, 80ULL << 20,
        100ULL << 20, 100, 100, true});
    require(bounded.cacheBytes == 20ULL << 20 &&
                !bounded.asyncDoubleBufferAllowed,
            "cache and double buffers must obey the central memory budget");
    const auto empty = SKNano::TreeCacheTuner::tune({
        16ULL << 20, 64ULL << 20, 0, 0, 0, 0, false});
    require(empty.cacheBytes == 16ULL << 20,
            "unknown footprint must preserve the requested portable cache");
    return 0;
}

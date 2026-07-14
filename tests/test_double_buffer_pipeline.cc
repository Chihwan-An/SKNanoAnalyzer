#include "DoubleBufferPipeline.h"
#include "ClusterTaskPlanner.h"

#include <atomic>
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace {
void require(bool condition, const char *message) {
    if (!condition) throw std::runtime_error(message);
}
struct Reader {
    std::uint64_t token = 17;
    Reader() = default;
    Reader(const Reader &) = delete;
    Reader(Reader &&) = delete;
};
}

int main() {
    std::vector<SKNano::ClusterTask> tasks;
    for (std::uint64_t id = 0; id < 20; ++id)
        tasks.push_back({id, 0, static_cast<std::int64_t>(id),
                        static_cast<std::int64_t>(id + 1)});
    std::vector<std::uint64_t> observed;
    std::atomic<std::size_t> reads{0};
    SKNano::DoubleBufferPipeline::run(
        tasks, [] { return Reader{}; },
        [&](Reader &reader, const SKNano::ClusterTask &task) {
            ++reads;
            return std::vector<std::uint64_t>{reader.token + task.taskId};
        },
        [&](const SKNano::ClusterTask &task,
            const std::vector<std::uint64_t> &block) {
            require(block.at(0) == 17 + task.taskId,
                    "owned block must come from the dedicated reader context");
            observed.push_back(task.taskId);
        });
    require(reads == tasks.size() && observed.size() == tasks.size(),
            "double buffer must read and process every task exactly once");
    for (std::size_t index = 0; index < observed.size(); ++index)
        require(observed[index] == index,
                "double buffer processing order must remain deterministic");
    return 0;
}

#include <exception>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "ClusterTaskPlanner.h"

namespace {
template <typename Exception, typename Function>
bool throws(Function &&function) {
    try { function(); } catch (const Exception &) { return true; } catch (...) {}
    return false;
}
void require(bool condition, const char *message) {
    if (!condition) throw std::runtime_error(message);
}
}

int main() {
    try {
        const auto tasks = SKNano::ClusterTaskPlanner::plan(
            9, {{0, 10}, {10, 17}}, 6);
        require(tasks.size() == 4 && tasks[0].begin == 0 && tasks[0].end == 6 &&
                    tasks[1].begin == 6 && tasks[1].end == 10 &&
                    tasks[2].begin == 10 && tasks[2].end == 16 &&
                    tasks[3].taskId == 3 && tasks[3].fileToken == 9,
                "tasks must split deterministically without crossing clusters");
        require(throws<SKNano::ConfigError>([] {
                    SKNano::ClusterTaskPlanner::plan(0, {{5, 3}}, 10);
                }), "invalid cluster range must fail");
        require(throws<SKNano::ConfigError>([] {
                    SKNano::ClusterTaskPlanner::plan(0, {{0, 1}}, 0);
                }), "zero task size must fail");
    } catch (const std::exception &error) {
        std::cerr << "test_cluster_task_planner: " << error.what() << '\n';
        return 1;
    }
    return 0;
}

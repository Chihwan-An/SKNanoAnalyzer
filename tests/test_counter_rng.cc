#include <cmath>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <vector>

#include "CounterRNG.h"

namespace {
void require(bool condition, const char *message) {
    if (!condition) throw std::runtime_error(message);
}
}

int main() {
    try {
        std::vector<double> reference(1000);
        for (std::size_t index = 0; index < reference.size(); ++index)
            reference[index] = SKNano::CounterRNG::normal(1, 2, 3, index, 7);
        std::vector<double> parallel(reference.size());
        std::vector<std::thread> workers;
        for (std::size_t worker = 0; worker < 4; ++worker) {
            workers.emplace_back([&, worker] {
                for (std::size_t index = worker; index < parallel.size(); index += 4)
                    parallel[index] =
                        SKNano::CounterRNG::normal(1, 2, 3, index, 7);
            });
        }
        for (auto &worker : workers) worker.join();
        require(reference == parallel,
                "counter RNG must be independent of worker scheduling");
        require(reference[0] == SKNano::CounterRNG::normal(1, 2, 3, 0, 7) &&
                    reference[0] != SKNano::CounterRNG::normal(1, 2, 4, 0, 7),
                "fixed keys must repeat and event keys must decorrelate");
        double mean = 0.;
        double square = 0.;
        for (const double value : reference) {
            mean += value;
            square += value * value;
        }
        mean /= reference.size();
        square /= reference.size();
        require(std::abs(mean) < 0.12 && square > 0.8 && square < 1.2,
                "counter RNG normal smoke distribution must be centered and unit width");
    } catch (const std::exception &error) {
        std::cerr << "test_counter_rng: " << error.what() << '\n';
        return 1;
    }
    return 0;
}

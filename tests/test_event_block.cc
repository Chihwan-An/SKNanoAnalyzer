#include <cstdint>
#include <exception>
#include <iostream>
#include <stdexcept>

#include "EventBlock.h"

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
        const SKNano::ColumnId pt(0);
        const SKNano::ColumnId flavour(1);
        SKNano::EventBlock block(4096, 4);
        block.registerColumn<float>(pt);
        block.registerColumn<std::int32_t>(flavour);
        block.begin(7, 100);
        const float pt0[] = {10.f, 20.f};
        const std::int32_t flavour0[] = {4, 5};
        require(block.beginEvent(7, 100, sizeof(pt0) + sizeof(flavour0)),
                "complete event reservation must fit");
        block.append(pt, pt0, 2);
        block.append(flavour, flavour0, 2);
        block.finishEvent(7, 100);
        const float pt1[] = {30.f};
        const std::int32_t flavour1[] = {1};
        require(block.beginEvent(7, 101, sizeof(pt1) + sizeof(flavour1)),
                "second complete event reservation must fit");
        block.append(pt, pt1, 1);
        block.append(flavour, flavour1, 1);
        block.finishEvent(7, 101);
        block.seal();

        const auto second = block.event<float>(pt, 1);
        require(second.size() == 1 && second[0] == 30.f,
                "ragged values and offsets must preserve event ordering");
        require(block.event<std::int32_t>(flavour, 0)[1] == 5,
                "typed block columns must preserve values");
        require(block.bytesUsed() > 0,
                "block must report owned gather bytes");
        require(throws<SKNano::ConfigError>([&] {
                    static_cast<void>(block.event<double>(pt, 0));
                }), "wrong block column type must fail");

        block.reset();
        require(throws<SKNano::LogicError>([&] {
                    static_cast<void>(second.size());
                }), "views must fail after block epoch reset");

        block.begin(8, 0);
        require(block.beginEvent(8, 0, sizeof(pt1) + sizeof(flavour1)),
                "event reservation must fit after reset");
        block.append(pt, pt1, 1);
        block.append(flavour, flavour1, 1);
        require(throws<SKNano::LogicError>([&] { block.finishEvent(9, 0); }),
                "a block must not cross tree boundaries");

        SKNano::EventBlock tiny(sizeof(std::size_t) * 2 + sizeof(float), 2);
        tiny.registerColumn<float>(pt);
        tiny.registerColumn<std::int32_t>(flavour);
        tiny.begin(1, 0);
        require(!tiny.beginEvent(1, 0, sizeof(float) + sizeof(std::int32_t)),
                "capacity rejection must happen before any column mutation");
        require(tiny.eventCount() == 0 &&
                    tiny.bytesUsed() == sizeof(std::size_t) * 2,
                "rejected event must leave block unchanged");
    } catch (const std::exception &error) {
        std::cerr << "test_event_block: " << error.what() << '\n';
        return 1;
    }
    return 0;
}

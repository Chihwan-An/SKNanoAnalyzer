#include "MLHelper.h"

#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace {
void require(bool condition, const char *message) {
    if (!condition) throw std::runtime_error(message);
}
}

int main() {
    MLHelper helper("stub.onnx", MLHelper::ModelType::ONNX);
    FloatArray floats{1.f, 2.f, 3.f, 4.f};
    IntArray integers{10, 20};
    std::vector<MLHelper::TensorView> inputs{
        {MLHelper::TensorDType::Float32, floats.data(), floats.size(), {2, 2}},
        {MLHelper::TensorDType::Int32, integers.data(), integers.size(), {2, 1}}
    };
    const auto &first = helper.RunPreparedView(inputs);
    require(first.size() == 1 && first[0] == FloatArray({13.f, 27.f}),
            "prepared output must match the model reference");
    const float *outputStorage = first[0].data();
    require(helper.PreparedBindingRebuilds() == 1,
            "first prepared call must create one binding set");

    floats[0] = 5.f;
    const auto &second = helper.RunPreparedView(inputs);
    require(second[0] == FloatArray({17.f, 27.f}) &&
                helper.PreparedBindingRebuilds() == 1 &&
                second[0].data() == outputStorage,
            "warm prepared call must reuse bindings and output capacity");
    const FloatArray preparedReference = second[0];

    std::unordered_map<std::string, VariousArray> legacyData{
        {"float_input", floats}, {"int_input", integers}};
    std::unordered_map<std::string, IntArray> legacyShape{
        {"float_input", {2, 2}}, {"int_input", {2, 1}}};
    const auto legacy = helper.Run_ONNX_Model(legacyData, legacyShape);
    require(legacy.at("output") == preparedReference,
            "legacy map API and prepared API must be bitwise-equal");

    FloatArray moved = floats;
    inputs[0].data = moved.data();
    helper.RunPreparedView(inputs);
    require(helper.PreparedBindingRebuilds() >= 2,
            "a changed input address must rebuild the tensor binding");

    return 0;
}

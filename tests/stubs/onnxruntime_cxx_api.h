#ifndef SKNANO_TEST_ONNXRUNTIME_CXX_API_H
#define SKNANO_TEST_ONNXRUNTIME_CXX_API_H

#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

enum OrtLoggingLevel { ORT_LOGGING_LEVEL_WARNING };
enum GraphOptimizationLevel { ORT_ENABLE_ALL };
enum OrtAllocatorType { OrtArenaAllocator };
enum OrtMemType { OrtMemTypeDefault };
enum ONNXTensorElementDataType {
    ONNX_TENSOR_ELEMENT_DATA_TYPE_UNDEFINED,
    ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT,
    ONNX_TENSOR_ELEMENT_DATA_TYPE_INT32 = 6,
    ONNX_TENSOR_ELEMENT_DATA_TYPE_BOOL = 9
};

namespace Ort {

class Exception : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class Env {
public:
    Env(OrtLoggingLevel, const char *) {}
};
class SessionOptions {
public:
    void SetIntraOpNumThreads(int) {}
    void SetGraphOptimizationLevel(GraphOptimizationLevel) {}
};
class MemoryInfo {
public:
    static MemoryInfo CreateCpu(OrtAllocatorType, OrtMemType) { return {}; }
};
class RunOptions {
public:
    void SetRunLogVerbosityLevel(int) {}
};
class AllocatorWithDefaultOptions {};
class AllocatedStringPtr {
public:
    explicit AllocatedStringPtr(std::string value) : value_(std::move(value)) {}
    const char *get() const noexcept { return value_.c_str(); }
private:
    std::string value_;
};

class TensorTypeAndShapeInfo {
public:
    TensorTypeAndShapeInfo(ONNXTensorElementDataType type,
                           std::vector<std::int64_t> shape)
        : type_(type), shape_(std::move(shape)) {}
    std::vector<std::int64_t> GetShape() const { return shape_; }
    ONNXTensorElementDataType GetElementType() const { return type_; }
    std::size_t GetElementCount() const {
        std::size_t result = 1;
        for (const auto dimension : shape_)
            result *= static_cast<std::size_t>(dimension);
        return result;
    }
private:
    ONNXTensorElementDataType type_;
    std::vector<std::int64_t> shape_;
};
class TypeInfo {
public:
    TypeInfo(ONNXTensorElementDataType type, std::vector<std::int64_t> shape)
        : info_(type, std::move(shape)) {}
    TensorTypeAndShapeInfo GetTensorTypeAndShapeInfo() const { return info_; }
private:
    TensorTypeAndShapeInfo info_;
};

class Value {
public:
    template <typename T>
    static Value CreateTensor(const MemoryInfo &, T *data, std::size_t count,
                              const std::int64_t *shape, std::size_t rank) {
        Value value;
        value.data_ = data;
        value.count_ = count;
        value.shape_.assign(shape, shape + rank);
        if constexpr (std::is_same_v<T, float>)
            value.type_ = ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT;
        else if constexpr (std::is_same_v<T, int>)
            value.type_ = ONNX_TENSOR_ELEMENT_DATA_TYPE_INT32;
        else if constexpr (std::is_same_v<T, bool>)
            value.type_ = ONNX_TENSOR_ELEMENT_DATA_TYPE_BOOL;
        return value;
    }
    static Value OwnedFloat(std::vector<float> data,
                            std::vector<std::int64_t> shape) {
        Value value;
        value.ownedFloat_ =
            std::make_shared<std::vector<float>>(std::move(data));
        value.data_ = value.ownedFloat_->data();
        value.count_ = value.ownedFloat_->size();
        value.shape_ = std::move(shape);
        value.type_ = ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT;
        return value;
    }
    TensorTypeAndShapeInfo GetTensorTypeAndShapeInfo() const {
        return {type_, shape_};
    }
    template <typename T> const T *GetTensorData() const {
        return static_cast<const T *>(data_);
    }
private:
    friend class Session;
    void *data_ = nullptr;
    std::size_t count_ = 0;
    std::vector<std::int64_t> shape_;
    ONNXTensorElementDataType type_ = ONNX_TENSOR_ELEMENT_DATA_TYPE_UNDEFINED;
    std::shared_ptr<std::vector<float>> ownedFloat_;
};

class Session;
class IoBinding {
public:
    explicit IoBinding(Session &session) : session_(&session) {}
    void BindInput(const char *, const Value &value) { inputs_.push_back(value); }
    void BindOutput(const char *, const MemoryInfo &) {}
    void ClearBoundInputs() { inputs_.clear(); }
    void ClearBoundOutputs() { outputs_.clear(); }
    void SynchronizeInputs() {}
    void SynchronizeOutputs() {}
    std::vector<Value> GetOutputValues() const { return outputs_; }
private:
    friend class Session;
    Session *session_;
    std::vector<Value> inputs_;
    std::vector<Value> outputs_;
};

class Session {
public:
    Session(Env &, const char *, const SessionOptions &) {}
    std::size_t GetInputCount() const { return 2; }
    std::size_t GetOutputCount() const { return 1; }
    AllocatedStringPtr GetInputNameAllocated(
        std::size_t index, AllocatorWithDefaultOptions &) const {
        return AllocatedStringPtr(index == 0 ? "float_input" : "int_input");
    }
    AllocatedStringPtr GetOutputNameAllocated(
        std::size_t, AllocatorWithDefaultOptions &) const {
        return AllocatedStringPtr("output");
    }
    TypeInfo GetInputTypeInfo(std::size_t index) const {
        return index == 0
            ? TypeInfo(ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT, {-1, 2})
            : TypeInfo(ONNX_TENSOR_ELEMENT_DATA_TYPE_INT32, {-1, 1});
    }
    TypeInfo GetOutputTypeInfo(std::size_t) const {
        return {ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT, {-1, 1}};
    }
    std::vector<Value> Run(const RunOptions &, const char *const *,
                           const Value *inputs, std::size_t inputCount,
                           const char *const *, std::size_t) {
        if (inputCount != 2)
            throw Exception("stub input count mismatch");
        const auto rows = static_cast<std::size_t>(inputs[0].shape_.at(0));
        const float *floats = static_cast<const float *>(inputs[0].data_);
        const int *integers = static_cast<const int *>(inputs[1].data_);
        std::vector<float> output(rows);
        for (std::size_t row = 0; row < rows; ++row)
            output[row] = floats[row * 2] + floats[row * 2 + 1] +
                          static_cast<float>(integers[row]);
        return {Value::OwnedFloat(std::move(output),
                                  {static_cast<std::int64_t>(rows), 1})};
    }
    void Run(const RunOptions &options, IoBinding &binding) {
        binding.outputs_ = Run(options, nullptr, binding.inputs_.data(),
                               binding.inputs_.size(), nullptr, 0);
    }
};

} // namespace Ort

#endif

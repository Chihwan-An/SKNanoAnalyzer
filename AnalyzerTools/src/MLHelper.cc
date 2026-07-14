#include "MLHelper.h"


// #include <torch/script.h>
#include <onnxruntime_cxx_api.h>
#include <unordered_map>
#include <stdexcept>
#include <limits>
#include <type_traits>

namespace {
Ort::Env &ProcessOrtEnvironment()
{
    static Ort::Env environment(ORT_LOGGING_LEVEL_WARNING, "SKNanoML");
    return environment;
}
}

// Implementation class definition
class MLHelperImpl
{
public:
    // Constructor
    MLHelperImpl(const std::string &modelPath, MLHelper::ModelType modelType)
        : modelPath_(modelPath), modelType_(modelType),
          memoryInfo_(Ort::MemoryInfo::CreateCpu(OrtArenaAllocator,
                                                 OrtMemTypeDefault))
    {
        runOptions_.SetRunLogVerbosityLevel(0);
        if (modelType_ == MLHelper::ModelType::ONNX)
        {
            Load_ONNX_Model(modelPath_);
        }
        else if (modelType_ == MLHelper::ModelType::TORCHSCRIPT)
        {
            Load_TorchScript_Model(modelPath_);
        }
    }

    ~MLHelperImpl()
    {
        // Resources are managed by smart pointers 
    }

    void Load_TorchScript_Model(const std::string &)
    {
        throw std::runtime_error("[MLHelperImpl::Load_TorchScript_Model] TorchScript model loading is not implemented yet.");
    }

    // Load ONNX model
    void Load_ONNX_Model(const std::string &modelPath)
    {
        if (torchScriptModelLoaded_)
        {
            throw std::runtime_error("[MLHelperImpl::Load_ONNX_Model] TorchScript model is already loaded.");
        }

        try
        {
            if (onnxModelLoaded_)
            {
                throw std::runtime_error(
                    "[MLHelperImpl::Load_ONNX_Model] ONNX model is already loaded.");
            }
            Ort::SessionOptions sessionOptions;
            sessionOptions.SetIntraOpNumThreads(1);
            sessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
            session_ = std::make_unique<Ort::Session>(ProcessOrtEnvironment(), modelPath.c_str(), sessionOptions);

            Ort::AllocatorWithDefaultOptions allocator;

            // Get input node names and shapes
            size_t numInputNodes = session_->GetInputCount();
            inputNodeNames_.reserve(numInputNodes);
            inputNodeNamesChar_.reserve(numInputNodes);
            inputShapes_.reserve(numInputNodes);
            inputShapesByIndex_.reserve(numInputNodes);
            inputElementTypes_.reserve(numInputNodes);

            for (size_t i = 0; i < numInputNodes; i++)
            {
                Ort::AllocatedStringPtr inputNameAllocated = session_->GetInputNameAllocated(i, allocator);
                const char *inputName = inputNameAllocated.get();
                inputNodeNames_.emplace_back(inputName);
                inputNodeNamesChar_.emplace_back(inputNodeNames_.back().c_str());

                Ort::TypeInfo typeInfo = session_->GetInputTypeInfo(i);
                auto tensorInfo = typeInfo.GetTensorTypeAndShapeInfo();
                auto shape = tensorInfo.GetShape();
                inputShapes_[inputNodeNames_.back()] = shape;
                inputShapesByIndex_.push_back(std::move(shape));
                inputElementTypes_.push_back(tensorInfo.GetElementType());
            }

            // Get output node names and shapes
            size_t numOutputNodes = session_->GetOutputCount();
            outputNodeNames_.reserve(numOutputNodes);
            outputNodeNamesChar_.reserve(numOutputNodes);
            outputShapes_.reserve(numOutputNodes);

            for (size_t i = 0; i < numOutputNodes; i++)
            {
                Ort::AllocatedStringPtr outputNameAllocated = session_->GetOutputNameAllocated(i, allocator);
                const char *outputName = outputNameAllocated.get();
                outputNodeNames_.emplace_back(outputName);
                outputNodeNamesChar_.emplace_back(outputNodeNames_.back().c_str());

                Ort::TypeInfo typeInfo = session_->GetOutputTypeInfo(i);
                auto tensorInfo = typeInfo.GetTensorTypeAndShapeInfo();
                outputShapes_[outputNodeNames_.back()] = tensorInfo.GetShape();
            }

            ioBinding_ = std::make_unique<Ort::IoBinding>(*session_);

            onnxModelLoaded_ = true;
        }
        catch (const Ort::Exception &e)
        {
            throw std::runtime_error("[MLHelperImpl::Load_ONNX_Model] Failed to load ONNX model: " + std::string(e.what()));
        }
    }

    // Run ONNX model
    // currrently supports float, int, bool(should be get as uint8_t)
    const FloatArrays &
    RunPreparedView(const std::vector<MLHelper::TensorView> &inputs)
    {
        if (!onnxModelLoaded_ || !session_)
            throw std::runtime_error(
                "[MLHelperImpl::RunPrepared] ONNX model is not loaded.");
        if (inputs.size() != inputNodeNames_.size())
            throw std::runtime_error(
                "[MLHelperImpl::RunPrepared] input count mismatch");
        bool rebuildBindings = inputBindings_.size() != inputs.size();
        if (!rebuildBindings) {
            for (std::size_t index = 0; index < inputs.size(); ++index) {
                const auto &old = inputBindings_[index];
                const auto &current = inputs[index];
                if (old.dtype != current.dtype || old.data != current.data ||
                    old.size != current.size || old.shape != current.shape) {
                    rebuildBindings = true;
                    break;
                }
            }
        }
        if (rebuildBindings) {
            ioBinding_->ClearBoundInputs();
            ioBinding_->ClearBoundOutputs();
            inputTensors_.clear();
            inputTensors_.reserve(inputs.size());
            inputBindings_.clear();
            inputBindings_.reserve(inputs.size());
        }
        for (std::size_t index = 0; index < inputs.size(); ++index)
        {
            const auto &input = inputs[index];
            if (input.shape.size() != inputShapesByIndex_[index].size())
                throw std::runtime_error(
                    "[MLHelperImpl::RunPrepared] rank mismatch for " +
                    inputNodeNames_[index]);
            std::size_t expectedSize = 1;
            for (std::size_t dimension = 0; dimension < input.shape.size();
                 ++dimension)
            {
                if (input.shape[dimension] <= 0 ||
                    (inputShapesByIndex_[index][dimension] > 0 &&
                     inputShapesByIndex_[index][dimension] !=
                         input.shape[dimension]))
                    throw std::runtime_error(
                        "[MLHelperImpl::RunPrepared] shape mismatch for " +
                        inputNodeNames_[index]);
                if (expectedSize >
                    std::numeric_limits<std::size_t>::max() /
                        static_cast<std::size_t>(input.shape[dimension]))
                    throw std::runtime_error(
                        "[MLHelperImpl::RunPrepared] shape size overflow for " +
                        inputNodeNames_[index]);
                expectedSize *= static_cast<std::size_t>(input.shape[dimension]);
            }
            if (expectedSize != input.size || (input.size && !input.data))
                throw std::runtime_error(
                    "[MLHelperImpl::RunPrepared] data size mismatch for " +
                    inputNodeNames_[index]);
            switch (input.dtype)
            {
            case MLHelper::TensorDType::Float32:
                if (inputElementTypes_[index] !=
                    ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT)
                    throw std::runtime_error(
                        "[MLHelperImpl::RunPrepared] dtype mismatch for " +
                        inputNodeNames_[index]);
                if (rebuildBindings)
                    inputTensors_.emplace_back(Ort::Value::CreateTensor<float>(
                        memoryInfo_, const_cast<float *>(
                                         static_cast<const float *>(input.data)),
                        input.size, input.shape.data(), input.shape.size()));
                break;
            case MLHelper::TensorDType::Int32:
                static_assert(sizeof(int) == sizeof(std::int32_t),
                              "IntArray requires a 32-bit int");
                if (inputElementTypes_[index] !=
                    ONNX_TENSOR_ELEMENT_DATA_TYPE_INT32)
                    throw std::runtime_error(
                        "[MLHelperImpl::RunPrepared] dtype mismatch for " +
                        inputNodeNames_[index]);
                if (rebuildBindings)
                    inputTensors_.emplace_back(Ort::Value::CreateTensor<int>(
                        memoryInfo_, const_cast<int *>(
                                         static_cast<const int *>(input.data)),
                        input.size, input.shape.data(), input.shape.size()));
                break;
            case MLHelper::TensorDType::Bool:
                if (inputElementTypes_[index] !=
                    ONNX_TENSOR_ELEMENT_DATA_TYPE_BOOL)
                    throw std::runtime_error(
                        "[MLHelperImpl::RunPrepared] dtype mismatch for " +
                        inputNodeNames_[index]);
                if (rebuildBindings)
                    inputTensors_.emplace_back(Ort::Value::CreateTensor<bool>(
                        memoryInfo_, reinterpret_cast<bool *>(
                                         const_cast<void *>(input.data)),
                        input.size, input.shape.data(), input.shape.size()));
                break;
            }
            if (rebuildBindings)
                inputBindings_.push_back(
                    {input.dtype, input.data, input.size, input.shape});
        }
        if (rebuildBindings) {
            for (std::size_t index = 0; index < inputTensors_.size(); ++index)
                ioBinding_->BindInput(inputNodeNamesChar_[index],
                                      inputTensors_[index]);
            for (const auto *name : outputNodeNamesChar_)
                ioBinding_->BindOutput(name, memoryInfo_);
            ++preparedBindingRebuilds_;
        }
        ioBinding_->SynchronizeInputs();
        session_->Run(runOptions_, *ioBinding_);
        ioBinding_->SynchronizeOutputs();
        auto outputTensors = ioBinding_->GetOutputValues();
        preparedOutputs_.resize(outputTensors.size());
        for (std::size_t index = 0; index < outputTensors.size(); ++index)
        {
            auto &tensor = outputTensors[index];
            const auto info = tensor.GetTensorTypeAndShapeInfo();
            if (info.GetElementType() != ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT)
                throw std::runtime_error(
                    "[MLHelperImpl::RunPrepared] only float outputs are supported");
            const std::size_t count = info.GetElementCount();
            const float *data = tensor.GetTensorData<float>();
            preparedOutputs_[index].assign(data, data + count);
        }
        return preparedOutputs_;
    }

    std::vector<FloatArray>
    RunPrepared(const std::vector<MLHelper::TensorView> &inputs)
    {
        return RunPreparedView(inputs);
    }

    std::unordered_map<std::string, FloatArray> Run_ONNX_Model(
        const std::unordered_map<std::string, VariousArray> &inputDataMap,
        const std::unordered_map<std::string, IntArray> &inputDataShapeMap)
    {
        if (!onnxModelLoaded_ || !session_)
        {
            throw std::runtime_error("[MLHelperImpl::Run_ONNX_Model] ONNX model is not loaded.");
        }

        try
        {
            if (inputNodeNames_.empty())
            {
                throw std::runtime_error("[MLHelperImpl::Run_ONNX_Model] No input nodes found in the ONNX model.");
            }

            for (const auto &inputName : inputNodeNames_)
            {
                if (inputDataMap.find(inputName) == inputDataMap.end())
                {
                    throw std::runtime_error("[MLHelperImpl::Run_ONNX_Model] Missing input data for node: " + inputName);
                }

                if (inputDataShapeMap.find(inputName) == inputDataShapeMap.end())
                {
                    throw std::runtime_error("[MLHelperImpl::Run_ONNX_Model] Missing input shape data for node: " + inputName);
                }
            }

            std::vector<MLHelper::TensorView> preparedInputs;
            preparedInputs.reserve(inputNodeNames_.size());

            for (const auto &inputName : inputNodeNames_)
            {
                const IntArray &shapeData = inputDataShapeMap.at(inputName);

                std::vector<int64_t> actualShape(shapeData.size());
                for (size_t i = 0; i < shapeData.size(); ++i)
                {
                    int64_t dim = static_cast<int64_t>(shapeData[i]);
                    if (dim <= 0)
                    {
                        throw std::runtime_error("[MLHelperImpl::Run_ONNX_Model] Invalid dimension (<=0) provided for input: " + inputName);
                    }
                    actualShape[i] = dim;
                }

                size_t inputTensorSize = 1;
                for (auto dim : actualShape)
                {
                    inputTensorSize *= static_cast<size_t>(dim);
                }

                const auto &inputData = inputDataMap.at(inputName);

                std::visit([&](auto &&data){
                using T = std::decay_t<decltype(data)>;
                
                // Validate size
                if (data.size() != inputTensorSize) {
                    throw std::runtime_error("[MLHelperImpl::Run_ONNX_Model] Input data size for node '" + inputName +
                                            "' does not match the user-provided shape. Expected: " +
                                            std::to_string(inputTensorSize) + ", Got: " + std::to_string(data.size()));
                }

                if constexpr (std::is_same_v<T, FloatArray>) {
                    preparedInputs.push_back({MLHelper::TensorDType::Float32,
                                              data.data(), data.size(), actualShape});
                } else if constexpr (std::is_same_v<T, IntArray>) {
                    preparedInputs.push_back({MLHelper::TensorDType::Int32,
                                              data.data(), data.size(), actualShape});
                } else if constexpr (std::is_same_v<T, BoolArray>) {
                    preparedInputs.push_back({MLHelper::TensorDType::Bool,
                                              data.data(), data.size(), actualShape});
                } else {
                    throw std::runtime_error("[MLHelperImpl::Run_ONNX_Model] Unsupported input data type for node '" + inputName + "'.");
                } }, inputData);
            }

            FloatArrays results = RunPrepared(preparedInputs);

            std::unordered_map<std::string, FloatArray> outputDataMap;

            for(size_t i = 0; i < outputNodeNames_.size(); i++)
            {
                outputDataMap[outputNodeNames_[i]] = results[i];
            }
            return outputDataMap;
        }
        catch (const Ort::Exception &e)
        {
            throw std::runtime_error("[MLHelperImpl::Run_ONNX_Model] Failed to run ONNX model: " + std::string(e.what()));
        }

        
    }

    // Getter for model type
    MLHelper::ModelType GetModelType() const
    {
        return modelType_;
    }

    const std::vector<std::string> &GetInputNodeNames() const
    {
        return inputNodeNames_;
    }

    const std::vector<std::string> &GetOutputNodeNames() const
    {
        return outputNodeNames_;
    }

    std::size_t PreparedBindingRebuilds() const noexcept
    {
        return preparedBindingRebuilds_;
    }

private:
    // Member variables
    std::string modelPath_;
    MLHelper::ModelType modelType_;

    // ONNX-related members
    bool onnxModelLoaded_ = false;
    std::unique_ptr<Ort::Session> session_;
    std::unique_ptr<Ort::IoBinding> ioBinding_;
    std::vector<std::string> inputNodeNames_;
    std::vector<std::string> outputNodeNames_;
    std::unordered_map<std::string, std::vector<int64_t>> inputShapes_;
    std::unordered_map<std::string, std::vector<int64_t>> outputShapes_;
    std::vector<std::vector<int64_t>> inputShapesByIndex_;
    std::vector<ONNXTensorElementDataType> inputElementTypes_;
    std::vector<const char *> inputNodeNamesChar_;
    std::vector<const char *> outputNodeNamesChar_;
    Ort::MemoryInfo memoryInfo_;
    Ort::RunOptions runOptions_;
    std::vector<Ort::Value> inputTensors_;
    struct InputBinding {
        MLHelper::TensorDType dtype;
        const void *data;
        std::size_t size;
        std::vector<std::int64_t> shape;
    };
    std::vector<InputBinding> inputBindings_;
    FloatArrays preparedOutputs_;
    std::size_t preparedBindingRebuilds_ = 0;

    // TorchScript-related members
    bool torchScriptModelLoaded_ = false;
    // torch::jit::script::Module module_;
};

// MLHelper Class Method Implementations

// Constructor
MLHelper::MLHelper(const std::string &modelPath, ModelType modelType)
    : pImpl(std::make_unique<MLHelperImpl>(modelPath, modelType))
{
}

// Destructor
MLHelper::~MLHelper() = default;

// Move Constructor
MLHelper::MLHelper(MLHelper &&) noexcept = default;

// Move Assignment Operator
MLHelper &MLHelper::operator=(MLHelper &&) noexcept = default;

// Load ONNX Model
void MLHelper::Load_ONNX_Model(const std::string &modelPath)
{
    pImpl->Load_ONNX_Model(modelPath);
}

// Run ONNX Model
std::unordered_map<std::string, FloatArray> MLHelper::Run_ONNX_Model(const std::unordered_map<std::string, VariousArray> &inputDataMap, const std::unordered_map<std::string, IntArray> &inputDataShapeMap)
{
    return pImpl->Run_ONNX_Model(inputDataMap, inputDataShapeMap);
}

std::vector<FloatArray>
MLHelper::RunPrepared(const std::vector<TensorView> &inputs)
{
    return pImpl->RunPrepared(inputs);
}

const FloatArrays &
MLHelper::RunPreparedView(const std::vector<TensorView> &inputs)
{
    return pImpl->RunPreparedView(inputs);
}

std::size_t MLHelper::PreparedBindingRebuilds() const
{
    return pImpl ? pImpl->PreparedBindingRebuilds() : 0;
}

    // Get Model Type
MLHelper::ModelType MLHelper::GetModelType() const
{
    return pImpl->GetModelType();
}

std::vector<std::string> MLHelper::GetInputNames() const
{
    if (!pImpl)
        return {};
    return pImpl->GetInputNodeNames();
}

std::vector<std::string> MLHelper::GetOutputNames() const
{
    if (!pImpl)
        return {};
    return pImpl->GetOutputNodeNames();
}

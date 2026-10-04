#include "ScycloneModelConfig.h"
#include "BinaryData.h"
#include "onnxruntime_cxx_api.h"

#include <sstream>

// Both networks are IRCAM ACIDS RAVE models. The tuning below is a direct port of anira's own
// reference configuration for this model family — see
// modules/anira/extras/models/third-party/ircam-acids/RaveFunkDrumConfig.h. Do not change any of
// these numbers without re-deriving them from that file (or from the model itself); they are not
// arbitrary and several of them affect dry/wet alignment.

namespace {

/// Matches anira's processing_spec_rave_funk_drum_config.
anira::ProcessingSpec makeRaveProcessingSpec()
{
    return anira::ProcessingSpec{
        {1},    // preprocess_input_channels
        {1},    // postprocess_output_channels
        {2048}, // preprocess_input_size — RAVE's hop
        {2048}, // postprocess_output_size
        // internal_model_latency: the delay the model itself introduces. anira adds this to the
        // reported latency and pre-fills its receive buffer with (latency - this) zeros, so a
        // wrong value shifts the wet path against the dry path by the difference.
        {2048}};
}

/// The single tensor shape Scyclone drives RAVE with, as {batch, channels, hop}.
constexpr std::array<int64_t, 3> kRaveTensorShape{1, 1, 2048};

/// Matches anira's tensor_shape_rave_funk_drum_config.
std::vector<anira::TensorShape> makeRaveTensorShapes()
{
    return {{{{kRaveTensorShape[0], kRaveTensorShape[1], kRaveTensorShape[2]}},
             {{kRaveTensorShape[0], kRaveTensorShape[1], kRaveTensorShape[2]}}}};
}

/// A dimension is acceptable if the model pins it to the value we drive, or leaves it dynamic
/// (ORT reports dynamic axes as a negative extent).
bool isCompatibleShape(const std::vector<int64_t>& shape, std::string& errorOut)
{
    if (shape.size() != kRaveTensorShape.size())
    {
        std::ostringstream os;
        os << "expected a " << kRaveTensorShape.size() << "-dimensional tensor, got "
           << shape.size();
        errorOut = os.str();
        return false;
    }

    for (size_t i = 0; i < shape.size(); ++i)
    {
        if (shape[i] < 0)
            continue; // dynamic axis — the model accepts whatever we feed it

        if (shape[i] != kRaveTensorShape[i])
        {
            std::ostringstream os;
            os << "dimension " << i << " is " << shape[i] << ", expected "
               << kRaveTensorShape[i];
            errorOut = os.str();
            return false;
        }
    }

    return true;
}

/// Max time anira allows one inference before it treats the result as late; sizes the struct
/// pool and the wait budget. 42.66 ms is one 2048-sample hop at 48 kHz, per anira's reference.
constexpr float kMaxInferenceTimeMs = 42.66f;

/// Inferences run at load time so the first real block does not pay allocation/JIT cost.
constexpr unsigned int kWarmUpInferences = 5;

/// RAVE caches convolution state across calls, so each session needs its own processor
/// instance — sharing one between the two networks would cross-contaminate their state.
constexpr bool kSessionExclusiveProcessor = true;

/// Both tensors must hold float32: anira feeds and reads float buffers, and a model of another
/// element type fails every Run(), which anira catches and logs, so the network would be silent.
bool isFloatTensor(const Ort::TypeInfo& typeInfo, std::string& errorOut)
{
    const auto elementType = typeInfo.GetTensorTypeAndShapeInfo().GetElementType();
    if (elementType == ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT)
        return true;

    std::ostringstream os;
    os << "expected float32 samples, got ONNX element type " << static_cast<int>(elementType);
    errorOut = os.str();
    return false;
}

} // namespace

anira::InferenceConfig makeScycloneInferenceConfig(RaveModel model)
{
    switch (model)
    {
        case Djembe:
            return makeScycloneInferenceConfig(BinaryData::djembe_ort, BinaryData::djembe_ortSize);
        case FunkDrum:
        default:
            return makeScycloneInferenceConfig(BinaryData::funk_drums_ort,
                                               BinaryData::funk_drums_ortSize);
    }
}

anira::InferenceConfig makeScycloneInferenceConfig(const void* modelBytes, size_t modelSize)
{
    // Binary ModelData only points at the bytes (copies are shallow); the caller keeps them alive
    // for as long as this config, or any copy of it, can still be used to build a session.
    std::vector<anira::ModelData> modelData;
    modelData.emplace_back(const_cast<void*>(modelBytes), modelSize, anira::InferenceBackend::ONNX,
                           std::string{}, true);
    return anira::InferenceConfig(
        modelData,
        makeRaveTensorShapes(),
        makeRaveProcessingSpec(),
        kMaxInferenceTimeMs,
        kWarmUpInferences,
        kSessionExclusiveProcessor);
}

bool validateRaveModel(const void* modelBytes, size_t modelSize, std::string& errorOut)
{
    errorOut.clear();

    if (modelBytes == nullptr || modelSize == 0)
    {
        errorOut = "the file is empty";
        return false;
    }

    try
    {
        Ort::Env env{ORT_LOGGING_LEVEL_ERROR, "ScycloneModelValidation"};
        Ort::SessionOptions options;
        options.SetIntraOpNumThreads(1);
        Ort::Session session{env, modelBytes, modelSize, options};

        if (session.GetInputCount() != 1 || session.GetOutputCount() != 1)
        {
            std::ostringstream os;
            os << "expected 1 input and 1 output tensor, got " << session.GetInputCount()
               << " and " << session.GetOutputCount();
            errorOut = os.str();
            return false;
        }

        const auto inputType = session.GetInputTypeInfo(0);
        const auto outputType = session.GetOutputTypeInfo(0);
        const auto inputShape = inputType.GetTensorTypeAndShapeInfo().GetShape();
        const auto outputShape = outputType.GetTensorTypeAndShapeInfo().GetShape();

        std::string reason;
        if (!isFloatTensor(inputType, reason))
        {
            errorOut = "input tensor: " + reason;
            return false;
        }
        if (!isFloatTensor(outputType, reason))
        {
            errorOut = "output tensor: " + reason;
            return false;
        }
        if (!isCompatibleShape(inputShape, reason))
        {
            errorOut = "input tensor: " + reason;
            return false;
        }
        if (!isCompatibleShape(outputShape, reason))
        {
            errorOut = "output tensor: " + reason;
            return false;
        }

        return true;
    }
    catch (const Ort::Exception& e)
    {
        errorOut = e.what();
        return false;
    }
    catch (const std::exception& e)
    {
        errorOut = e.what();
        return false;
    }
    catch (...)
    {
        errorOut = "unknown error";
        return false;
    }
}

#include "ScycloneModelConfig.h"
#include "BinaryData.h"
#include "onnxruntime_cxx_api.h"

#include <sstream>

// Both networks are IRCAM ACIDS RAVE models. The hop, inference budget, warm-up and processor
// settings below follow anira's reference configuration for this model family
// (modules/anira/extras/models/third-party/ircam-acids/RaveFunkDrumConfig.h). The internal model
// latency does not: that file describes a streaming TorchScript export, and these ONNX exports
// behave differently (see makeRaveProcessingSpec). Several of these numbers affect dry/wet
// alignment — re-measure with test/scyclone/calibration/OnnxWetLagProbeTest.cpp before changing.

namespace {

anira::ProcessingSpec makeRaveProcessingSpec()
{
    return anira::ProcessingSpec{
        {1},    // preprocess_input_channels
        {1},    // postprocess_output_channels
        {2048}, // preprocess_input_size — RAVE's hop
        {2048}, // postprocess_output_size
        // internal_model_latency: 0, measured. anira places each hop's output by its own buffering
        // alone and only adds this value to the reported latency, so it must equal the model's
        // real delay or the wet path drifts against the dry path and host delay compensation.
        // anira's reference config declares 2048 for a streaming TorchScript export with cached
        // convolutions; these ONNX exports are stateless (one input, one output, no state) and
        // process each 2048-sample hop on its own, so they add no whole-hop delay. They do lose
        // transient timing within a hop: OnnxWetLagProbe measures the median wet onset at about
        // 600 (FunkDrum) and 100 (Djembe) samples before the reported latency, spread across
        // roughly one hop depending on where in the hop a transient falls. Declaring 2048 made
        // the wet signal arrive about 2048 samples (43 ms at 48 kHz) early.
        {0}};
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

/// One processor instance per session, as in anira's reference config. (The reference reasons
/// that RAVE caches convolution state between calls; these stateless ONNX exports do not, so
/// this only keeps the two networks' inferences from sharing an ORT session.)
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

#include "ScycloneModelConfig.h"
#include "BinaryData.h"

namespace {

anira::ProcessingSpec makeRaveProcessingSpec()
{
    return anira::ProcessingSpec{
        {1},
        {1},
        {2048},
        {2048},
        {2048}};
}

std::vector<anira::TensorShape> makeRaveTensorShapes()
{
    return {{{{1, 1, 2048}}, {{1, 1, 2048}}}};
}

anira::ModelData makeEmbeddedModelData(RaveModel model)
{
    switch (model)
    {
        case FunkDrum:
            return anira::ModelData(
                (void*) BinaryData::funk_drums_ort,
                BinaryData::funk_drums_ortSize,
                anira::InferenceBackend::ONNX,
                "",
                true);
        case Djembe:
            return anira::ModelData(
                (void*) BinaryData::djembe_ort,
                BinaryData::djembe_ortSize,
                anira::InferenceBackend::ONNX,
                "",
                true);
        default:
            return anira::ModelData(
                (void*) BinaryData::funk_drums_ort,
                BinaryData::funk_drums_ortSize,
                anira::InferenceBackend::ONNX,
                "",
                true);
    }
}

} // namespace

anira::InferenceConfig makeScycloneInferenceConfig(RaveModel model)
{
    std::vector<anira::ModelData> modelData{makeEmbeddedModelData(model)};
    return anira::InferenceConfig(
        modelData,
        makeRaveTensorShapes(),
        makeRaveProcessingSpec(),
        42.66f,
        5,
        true);
}

anira::InferenceConfig makeScycloneInferenceConfigFromPath(const std::string& modelPath)
{
    std::vector<anira::ModelData> modelData;
    modelData.emplace_back(modelPath, anira::InferenceBackend::ONNX, std::string{}, false);
    return anira::InferenceConfig(
        modelData,
        makeRaveTensorShapes(),
        makeRaveProcessingSpec(),
        42.66f,
        5,
        true);
}

#include "AniraInferenceBackend.h"
#include "ScycloneModelConfig.h"

AniraInferenceBackend::AniraInferenceBackend(RaveModel model, anira::ContextConfig& contextConfigIn)
    : raveModel(model),
      contextConfig(contextConfigIn),
      inferenceConfig(makeScycloneInferenceConfig(model))
{
}

AniraInferenceBackend::~AniraInferenceBackend()
{
    handler.reset();
}

void AniraInferenceBackend::rebuildPipeline()
{
    handler.reset();
    prePostProcessor = std::make_unique<anira::PrePostProcessor>(inferenceConfig);
    handler = std::make_unique<anira::InferenceHandler>(
        *prePostProcessor, inferenceConfig, contextConfig);

    if (lastSpec.sampleRate > 0.0)
    {
        anira::HostConfig hostConfig{
            static_cast<float>(lastSpec.maximumBlockSize),
            static_cast<float>(lastSpec.sampleRate)};
        handler->prepare(hostConfig);
        latencyInSamples = static_cast<int>(handler->get_latency());
    }
}

void AniraInferenceBackend::prepare(const juce::dsp::ProcessSpec& spec)
{
    lastSpec = spec;

    if (handler == nullptr)
        rebuildPipeline();
    else
    {
        anira::HostConfig hostConfig{
            static_cast<float>(spec.maximumBlockSize),
            static_cast<float>(spec.sampleRate)};
        handler->prepare(hostConfig);
        latencyInSamples = static_cast<int>(handler->get_latency());
    }
}

void AniraInferenceBackend::processBlock(juce::AudioBuffer<float>& buffer)
{
    if (handler == nullptr || buffer.getNumChannels() < 1)
        return;

    handler->process(buffer.getArrayOfWritePointers(),
                     static_cast<size_t>(buffer.getNumSamples()));
}

int AniraInferenceBackend::getLatencyInSamples() const
{
    return latencyInSamples;
}

void AniraInferenceBackend::loadExternalModel(const juce::File& path)
{
    if (onModelLoad)
        onModelLoad(true, path.getFileNameWithoutExtension());

    inferenceConfig = makeScycloneInferenceConfigFromPath(path.getFullPathName().toStdString());
    rebuildPipeline();

    if (onModelLoad)
        onModelLoad(false, path.getFileNameWithoutExtension());
}

void AniraInferenceBackend::setInternalModel()
{
    if (onModelLoad)
        onModelLoad(true, "");

    inferenceConfig = makeScycloneInferenceConfig(raveModel);
    rebuildPipeline();

    if (onModelLoad)
        onModelLoad(false, "");
}

void AniraInferenceBackend::releaseResources()
{
    handler.reset();
    prePostProcessor.reset();
    latencyInSamples = 0;
}

#ifndef SCYCLONE_INFERENCE_STUB
std::unique_ptr<InferenceBackend> makeInferenceBackend(
    RaveModel model,
    anira::ContextConfig& contextConfig)
{
    return std::make_unique<AniraInferenceBackend>(model, contextConfig);
}
#endif

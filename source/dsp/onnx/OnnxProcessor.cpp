//
// Created by valentin.ackva on 16.03.2023.
//

#include "OnnxProcessor.h"

#ifndef SCYCLONE_INFERENCE_STUB
OnnxProcessor::OnnxProcessor(juce::AudioProcessorValueTreeState& apvts,
                             int no,
                             RaveModel raveModel,
                             anira::ContextConfig& contextConfig)
    : parameters(apvts), number(no)
{
    backend = makeInferenceBackend(raveModel, contextConfig);
#else
OnnxProcessor::OnnxProcessor(juce::AudioProcessorValueTreeState& apvts, int no, RaveModel raveModel)
    : parameters(apvts), number(no)
{
    backend = makeInferenceBackend(raveModel);
#endif
    backend->onModelLoad = [this](bool initLoading, juce::String modelName) {
        if (onOnnxModelLoad)
            onOnnxModelLoad(initLoading, modelName);
    };
}

void OnnxProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    const bool selectNetwork = (parameterID == PluginParameters::SELECT_NETWORK1_ID.getParamID() && number == 1)
                            || (parameterID == PluginParameters::SELECT_NETWORK2_ID.getParamID() && number == 2);

    if (selectNetwork)
    {
        if (!(bool) newValue)
        {
            backend->setInternalModel();
            latencyInSamples = backend->getLatencyInSamples();
        }
        return;
    }

    // Network off: skip inference rather than running it and discarding the result.
    const bool onOffNetwork = (parameterID == PluginParameters::ON_OFF_NETWORK1_ID.getParamID() && number == 1)
                           || (parameterID == PluginParameters::ON_OFF_NETWORK2_ID.getParamID() && number == 2);

    if (onOffNetwork)
        backend->setMuted(!(bool) newValue);
}

void OnnxProcessor::prepare(const juce::dsp::ProcessSpec& spec)
{
    backend->prepare(spec);
    latencyInSamples = backend->getLatencyInSamples();
}

void OnnxProcessor::processBlock(juce::AudioBuffer<float>& buffer)
{
    backend->processBlock(buffer);
}

bool OnnxProcessor::loadExternalModel(juce::File file)
{
    const bool loaded = backend->loadExternalModel(file);
    latencyInSamples = backend->getLatencyInSamples();
    return loaded;
}

void OnnxProcessor::releaseResources()
{
    backend->releaseResources();
    latencyInSamples = 0;
}

int OnnxProcessor::getLatencyInSamples() const
{
    return latencyInSamples;
}

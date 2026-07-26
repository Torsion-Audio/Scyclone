// Sanitizer stub — linked instead of InferenceThread.cpp when SCYCLONE_SANITIZER_STUB_ONNX is ON.
// Prebuilt ONNX is not MSan-instrumented and lacks MSVC ASan STL annotations; see cmake/setup_onnx_runtime.cmake.

#include "InferenceThread.h"

InferenceThread::InferenceThread(RaveModel raveModel) : juce::Thread("OnnxInference"), currentLevel(raveModel)
{
    modelInputSizeChanged(modelInputSize);
}

InferenceThread::~InferenceThread()
{
    stopThread(-1);
}

void InferenceThread::prepare(const juce::dsp::ProcessSpec &spec)
{
    receiveRingBuffer.initialise(1, (int) spec.sampleRate);
    last_spec = spec;
    init = true;
    init_samples = 0;
}

void InferenceThread::sendAudio(juce::AudioBuffer<float> &buffer)
{
    juce::ignoreUnused(buffer);
}

void InferenceThread::run()
{
    processedBuffer.clear();
    onNewProcessedBuffer(processedBuffer);
}

void InferenceThread::setExternalModel(juce::File modelPath)
{
    juce::ignoreUnused(modelPath);
}

void InferenceThread::modelInputSizeChanged(int newModelInputSize)
{
    modelInputSize = newModelInputSize;
    processedBuffer.setSize(1, newModelInputSize);
    onnxInputData.assign(static_cast<size_t>(newModelInputSize), 0.0f);
    onnxOutputData.assign(static_cast<size_t>(newModelInputSize), 0.0f);
}

bool InferenceThread::stopInferenceThreadAndWait()
{
    if (! isThreadRunning())
        return true;

    signalThreadShouldExit();

    if (! waitForThreadToExit(10000))
    {
        juce::Logger::writeToLog("InferenceThread: timed out waiting for inference thread to stop");
        return false;
    }

    return true;
}

void InferenceThread::loadExternalModel(juce::File modelPath)
{
    juce::ignoreUnused(modelPath);
    loadingModel = false;
}

void InferenceThread::loadInternalModel(RaveModel modelToLoad)
{
    juce::ignoreUnused(modelToLoad);
    loadingModel = false;
    startUp = false;
}

int InferenceThread::getLatency()
{
    return modelInputSize + maxModelCalcSize;
}

void InferenceThread::setInternalModel()
{
}

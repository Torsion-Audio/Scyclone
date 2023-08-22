//
// Created by valentin.ackva on 10.03.2023.
//

#ifndef VAESYNTH_INFERENCETHREAD_H
#define VAESYNTH_INFERENCETHREAD_H


#include "JuceHeader.h"
#include "onnxruntime_cxx_api.h"
#include "RingBuffer.h"
#include "chrono"

enum RaveModel {
    FunkDrum,
    Djembe
};

class InferenceThread : public juce::Thread {
public:
    InferenceThread(RaveModel raveModel);
    ~InferenceThread() override;

    void prepare(const juce::dsp::ProcessSpec& spec);
    void sendAudio(juce::AudioBuffer<float>& buffer);
    void setExternalModel(juce::File modelPath);
    int getLatency();

    std::function<int(juce::AudioBuffer<float> buffer)> onNewProcessedBuffer;
    std::function<void(juce::String modelName)> onModelLoaded;
    
    bool init = true;
    int init_samples = 0;
    void setInternalModel();

private:
    void run() override;

    void modelInputSizeChanged(int newModelInputSize);
    void loadExternalModel(juce::File modelPath);
    void loadInternalModel(RaveModel modelToLoad);
    std::vector<int> getInputShape(Ort::Session *sess);

private:
    bool startUp = true;
    juce::dsp::ProcessSpec last_spec;

    RaveModel currentLevel;

    Ort::Env env;
    Ort::RunOptions runOptions;
    Ort::Session session;
    Ort::SessionOptions sessionOptions;

    std::vector<float> onnxInputData;
    std::vector<float> onnxOutputData;

    juce::AudioBuffer<float> processedBuffer;
    int maxModelCalcSize = 4096;
    int modelInputSize = 16384;
    RingBuffer receiveRingBuffer;

    bool loadingModel = false;
};
#endif //VAESYNTH_INFERENCETHREAD_H

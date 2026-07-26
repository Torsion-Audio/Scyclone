#ifndef SCYCLONE_SANITIZERINFERENCEBACKEND_H
#define SCYCLONE_SANITIZERINFERENCEBACKEND_H

#include "InferenceBackend.h"
#include <vector>

class SanitizerInferenceBackend : public InferenceBackend {
public:
    explicit SanitizerInferenceBackend(RaveModel model);

    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void processBlock(juce::AudioBuffer<float>& buffer) override;
    int getLatencyInSamples() const override;
    void loadExternalModel(const juce::File& path) override;
    void setInternalModel() override;
    void releaseResources() override;

private:
    RaveModel raveModel;
    int latencyInSamples = 0;
    int fifoLength = 0;
    int writeIndex = 0;
    int filled = 0;
    std::vector<float> delayLine;
};

#endif // SCYCLONE_SANITIZERINFERENCEBACKEND_H

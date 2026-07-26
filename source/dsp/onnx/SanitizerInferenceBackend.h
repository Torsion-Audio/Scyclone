#ifndef SCYCLONE_SANITIZERINFERENCEBACKEND_H
#define SCYCLONE_SANITIZERINFERENCEBACKEND_H

#include "InferenceBackend.h"
#include <atomic>
#include <vector>

class SanitizerInferenceBackend : public InferenceBackend {
public:
    explicit SanitizerInferenceBackend(RaveModel model);

    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void processBlock(juce::AudioBuffer<float>& buffer) override;
    int getLatencyInSamples() const override;
    bool loadExternalModel(const juce::File& path) override;
    bool setInternalModel() override;
    void setMuted(bool shouldBeMuted) override;
    void releaseResources() override;

private:
    RaveModel raveModel;
    int latencyInSamples = 0;
    int fifoLength = 0;
    int writeIndex = 0;
    int filled = 0;
    std::vector<float> delayLine;
    std::atomic<bool> muted{false};
};

#endif // SCYCLONE_SANITIZERINFERENCEBACKEND_H

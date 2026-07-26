//
//  utils.c
//  Compressor
//
//  Created by Fares Schulz on 23.12.22.
//

#include "utils.h"
#include <cmath>

static constexpr float kMinAmp = 1.0e-9f;

float utils::amp2dB(float amp){
    return 20*std::log10(std::max(amp, kMinAmp));
}

float utils::amp2dB(float amp, float ampRef){
    return 20*std::log10(std::max(amp/ampRef, kMinAmp));
}

float utils::dB2amp(float db){
    return std::pow(10.f, (db/20.f));
}

float utils::dB2amp(float db, float ampRef){
    return std::pow(10.f, (db/20.f))*ampRef;
}


void utils::stereoToMono(juce::AudioBuffer<float> &targetMonoBlock, juce::AudioBuffer<float> &sourceBlock) {
    if (sourceBlock.getNumChannels() == 1) {
        targetMonoBlock.makeCopyOf(sourceBlock);
    } else {
        auto nSamples = sourceBlock.getNumSamples();

        auto monoWrite = targetMonoBlock.getWritePointer(0);
        auto lRead = sourceBlock.getReadPointer(0);
        auto rRead = sourceBlock.getReadPointer(1);

        juce::FloatVectorOperations::copy(monoWrite, lRead, nSamples);
        juce::FloatVectorOperations::add(monoWrite, rRead, nSamples);
        juce::FloatVectorOperations::multiply(monoWrite, 0.5f, nSamples);
    }
}

 void utils::monoToStereo(juce::AudioBuffer<float> &targetStereoBlock, juce::AudioBuffer<float> &sourceBlock) {
    if (sourceBlock.getNumChannels() == 2) {
        targetStereoBlock.makeCopyOf(sourceBlock);
    } else {
        auto nSamples = sourceBlock.getNumSamples();

        auto lWrite = targetStereoBlock.getWritePointer(0);
        auto rWrite = targetStereoBlock.getWritePointer(1);
        auto monoRead = sourceBlock.getReadPointer(0);

        juce::FloatVectorOperations::copy(lWrite, monoRead, nSamples);
        juce::FloatVectorOperations::copy(rWrite, monoRead, nSamples);
    }
}

int utils::computeTotalLatencyInSamples(
    int delayAtOutputRateSamples,
    int delayAtProcessingRateSamples1,
    int delayAtProcessingRateSamples2,
    double processingRate,
    double outputSampleRate)
{
    double totalSeconds = static_cast<double>(delayAtOutputRateSamples) / outputSampleRate
        + static_cast<double>(delayAtProcessingRateSamples1 + delayAtProcessingRateSamples2) / processingRate;
    return static_cast<int>(std::round(totalSeconds * outputSampleRate));
}

int utils::computeOnnxLatencyInSamples(int inferenceLatencyInSamples, int maxSamplesPerBuffer)
{
    // ceil(inference / block) * block - block, using integer arithmetic to avoid float equality pitfalls.
    int blocks = (inferenceLatencyInSamples + maxSamplesPerBuffer - 1) / maxSamplesPerBuffer;
    return blocks * maxSamplesPerBuffer - maxSamplesPerBuffer;
}
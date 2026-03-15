//
//  utils.h
//  Compressor_Plugin
//
//  Created by Fares Schulz on 23.12.22.
//

#pragma once

#include <cmath>
#include <juce_audio_basics/juce_audio_basics.h>

namespace utils {
    float amp2dB(float amp);
    float amp2dB(float amp, float ampRef);
    float dB2amp(float db);
    float dB2amp(float db, float ampRef);

    void monoToStereo(juce::AudioBuffer<float> &targetStereoBlock, juce::AudioBuffer<float> &sourceBlock);
    void stereoToMono(juce::AudioBuffer<float> &targetStereoBlock, juce::AudioBuffer<float> &sourceBlock);

    /** Returns total latency in samples at outputSampleRate. Uses std::round. */
    int computeTotalLatencyInSamples(
        int delayAtOutputRateSamples,
        int delayAtProcessingRateSamples1,
        int delayAtProcessingRateSamples2,
        double processingRate,
        double outputSampleRate);

    /** Block-aligned ONNX latency in samples. */
    int computeOnnxLatencyInSamples(int inferenceLatencyInSamples, int maxSamplesPerBuffer);
}

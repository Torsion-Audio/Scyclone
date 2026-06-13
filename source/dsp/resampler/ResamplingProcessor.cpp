#include <samplerate.h>
#include <ResamplingProcessor.h>
#include <cmath>
#include <vector>

// Nominal SRC_SINC_MEDIUM_QUALITY latency in input-rate samples at ratio >= 1:
// (coeff_half_len + 2) / index_inc = 22438 / 491 ≈ 45.7 → 46 (src_sinc.c lines 456-461).
// For downsampling (srcRatio < 1) the filter widens: latency = 46 / srcRatio input samples.
static const int kNominalSincMediumLatencyInputSamples = 46;

ResamplingProcessor::ResamplingProcessor() : converter(nullptr), srcRatio(1.0), id_string("noNameSet") {}

int ResamplingProcessor::prepare(const juce::dsp::ProcessSpec &inputSpec, double targetSampleRate, std::string name,
                               std::optional<int> forcedOutputBlockSize) {
    id_string = name;
    inputSampleRate =  inputSpec.sampleRate;
    inputBufferSize = static_cast<int>(inputSpec.maximumBlockSize);
    outputSampleRate = targetSampleRate;

    if (forcedOutputBlockSize.has_value()) {
        outputBufferSize = *forcedOutputBlockSize;
        srcRatio = static_cast<double>(outputBufferSize) / static_cast<double>(inputBufferSize);
        outputBufferMono.setSize(1, outputBufferSize);
    } else {
        setSamplerateRatio();
    }

    int error;
    converter = src_new(SRC_SINC_MEDIUM_QUALITY, 1, &error);
    if (error != 0 || !converter) {
        throw std::runtime_error(src_strerror(error));
    }

    // Impulse test: peak position in output -> latency in input-rate samples; fallback 46 if no peak.
    const int testInputSize = 1000;
    const int testOutputSize = static_cast<int>(std::ceil(testInputSize * outputSampleRate / inputSampleRate)) + 100;
    std::vector<float> testIn(static_cast<size_t>(testInputSize), 0.0f);
    std::vector<float> testOut(static_cast<size_t>(testOutputSize), 0.0f);
    testIn[0] = 1.0f;
    SRC_DATA testData{};
    testData.data_in = testIn.data();
    testData.data_out = testOut.data();
    testData.input_frames = testInputSize;
    testData.output_frames = testOutputSize;
    testData.src_ratio = srcRatio;
    testData.end_of_input = 1;
    error = src_process(converter, &testData);
    if (error == 0) {
        int peakPos = 0;
        float peakVal = 0.0f;
        for (long i = 0; i < testData.output_frames_gen; ++i) {
            float v = std::abs(testOut[static_cast<size_t>(i)]);
            if (v > peakVal) { peakVal = v; peakPos = static_cast<int>(i); }
        }
        // Convert peak position (output samples at impulse-test ratio) to input-rate samples.
        if (peakVal >= 0.01f && peakPos > 0) {
            latencyInSamples = static_cast<int>(std::round(static_cast<double>(peakPos) / srcRatio));
        } else {
            // For upsampling (srcRatio >= 1) filter half-length stays at 46 input samples.
            // For downsampling (srcRatio < 1) filter widens to 46 / srcRatio input samples.
            int fallback = static_cast<int>(std::round(
                static_cast<double>(kNominalSincMediumLatencyInputSamples) / std::min(srcRatio, 1.0)));
            latencyInSamples = (fallback > 0) ? fallback : 1;
        }
    } else {
        int fallback = static_cast<int>(std::round(
            static_cast<double>(kNominalSincMediumLatencyInputSamples) / std::min(srcRatio, 1.0)));
        latencyInSamples = (fallback > 0) ? fallback : 1;
    }
    src_reset(converter);

    return outputBufferSize;
}

void ResamplingProcessor::setSamplerateRatio()
{
    srcRatio = outputSampleRate / inputSampleRate;
    outputBufferSize = static_cast<int>(std::ceil(srcRatio * inputBufferSize));
    srcRatio = static_cast<double>(outputBufferSize) / static_cast<double>(inputBufferSize);  // match buffer size

    float timePerBlockInSec = static_cast<float>(inputBufferSize) / static_cast<float>(inputSampleRate);
    float correctedSampleRate = static_cast<float>(outputBufferSize) / timePerBlockInSec;
    outputBufferMono.setSize(1, outputBufferSize);

    juce::ignoreUnused(timePerBlockInSec, correctedSampleRate);
}



juce::AudioBuffer<float>& ResamplingProcessor::processBlock(juce::AudioBuffer<float>& inputBufferMono) {
    if (!converter) {
        throw std::runtime_error("Simple Rabbit Code Resampler not initialized");
    }

    SRC_DATA srcData;
    srcData.data_in = inputBufferMono.getReadPointer(0);
    srcData.input_frames = inputBufferMono.getNumSamples();

    srcData.data_out = outputBufferMono.getWritePointer(0);
    srcData.output_frames = outputBufferMono.getNumSamples();
    srcData.src_ratio = srcRatio;
    srcData.end_of_input = 0;

    int error = src_process(converter, &srcData);
    lastOutputFramesGenerated = srcData.output_frames_gen;
    lastInputFramesUsed = srcData.input_frames_used;
    if (error != 0) {
        jassertfalse;
    }
    // output_frames_gen can be < output_frames due to SINC transport delay or internal buffering (libsamplerate FAQ).
    if (srcData.output_frames_gen < srcData.output_frames) {
        // Only the first output_frames_gen samples are valid for this call; clear unwritten tail to avoid stale data.
        outputBufferMono.clear(0,
                               static_cast<int>(srcData.output_frames_gen),
                               static_cast<int>(srcData.output_frames - srcData.output_frames_gen));
    }
    return outputBufferMono;
}

void ResamplingProcessor::releaseResources() {
    if (converter) {
        src_delete(converter);
        converter = nullptr;
    }
}



#include <samplerate.h>
#include <ResamplingProcessor.h>
#include <cmath>
#include <vector>

// Nominal SRC_SINC_MEDIUM_QUALITY latency in input-rate samples at ratio >= 1:
// (coeff_half_len + 2) / index_inc = 22438 / 491 ≈ 45.7 → 46 (src_sinc.c lines 456-461).
// For downsampling (sampleRateRatio < 1) the filter widens: latency = 46 / sampleRateRatio input samples.
static const int kNominalSincMediumLatencyInputSamples = 46;

int ResamplingProcessor::prepare(const juce::dsp::ProcessSpec &inputSpec,
                                 double targetSampleRate,
                                 const std::string name) {
    if (converter) { src_delete(converter); converter = nullptr; }

    processorName = name;
    inputSampleRate = inputSpec.sampleRate;
    inputBufferSize = static_cast<int>(inputSpec.maximumBlockSize);

    sampleRateRatio = calculateSampleRateRatio(targetSampleRate, inputSampleRate);
    outputBufferSize = calculateOutputBufferSize(sampleRateRatio, inputBufferSize);
    bufferSizeRatio = calculateBufferSizeRatio(outputBufferSize, inputBufferSize);
    outputSampleRate = targetSampleRate;
    outputBuffer.setSize(1, outputBufferSize);
    outputBuffer.clear();
    printMetrics();

    int error;
    converter = src_new(SRC_SINC_MEDIUM_QUALITY, 1, &error);
    if (error != 0 || !converter) {
        throw std::runtime_error(src_strerror(error));
    }

    measureLatency();

    return outputBufferSize;
}

int ResamplingProcessor::calculateOutputBufferSize(double ratio, int blockSize)
{
    return static_cast<int>(std::ceil(ratio * blockSize));
}

double ResamplingProcessor::calculateSampleRateRatio(double outputRate, double inputRate)
{
    return outputRate / inputRate;
}

double ResamplingProcessor::calculateBufferSizeRatio(int outBufferSize, int inBufferSize)
{
    return static_cast<double>(outBufferSize) / static_cast<double>(inBufferSize);
}

void ResamplingProcessor::measureLatency()
{
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
    testData.src_ratio = sampleRateRatio;
    testData.end_of_input = 1;

    const int fallback = static_cast<int>(std::round(
        static_cast<double>(kNominalSincMediumLatencyInputSamples) / std::min(sampleRateRatio, 1.0)));

    if (src_process(converter, &testData) == 0) {
        int peakPos = 0;
        float peakVal = 0.0f;
        for (long i = 0; i < testData.output_frames_gen; ++i) {
            float v = std::abs(testOut[static_cast<size_t>(i)]);
            if (v > peakVal) { peakVal = v; peakPos = static_cast<int>(i); }
        }
        // Convert peak position (output samples) to input-rate samples.
        latencyInSamples = (peakVal >= 0.01f && peakPos > 0)
            ? static_cast<int>(std::round(static_cast<double>(peakPos) / sampleRateRatio))
            : std::max(fallback, 1);
    } else {
        latencyInSamples = std::max(fallback, 1);
    }

    src_reset(converter);
}

void ResamplingProcessor::printMetrics()
{
#if JUCE_DEBUG
    double timePerBlockInSec = static_cast<double>(inputBufferSize) / static_cast<double>(inputSampleRate);
    double correctedSampleRate = static_cast<double>(outputBufferSize) / timePerBlockInSec;

    DBG(juce::String(processorName));
    DBG("Samplerate Ratio Set: " << sampleRateRatio);
    DBG("Corrected Sample Rate after reconversion: " << correctedSampleRate << " Hz");
    DBG("------");
#endif
}

juce::AudioBuffer<float>& ResamplingProcessor::processBlock(juce::AudioBuffer<float>& inputBufferMono) {
    if (!converter) {
        throw std::runtime_error("Simple Rabbit Code Resampler not initialized");
    }

    SRC_DATA srcData;
    srcData.data_in = inputBufferMono.getReadPointer(0);
    srcData.input_frames = inputBufferMono.getNumSamples();

    srcData.data_out = outputBuffer.getWritePointer(0);
    srcData.output_frames = outputBuffer.getNumSamples();
    srcData.src_ratio = bufferSizeRatio;
    srcData.end_of_input = 0;

    int error = src_process(converter, &srcData);
    lastOutputFramesGenerated = srcData.output_frames_gen;
    lastInputFramesUsed = srcData.input_frames_used;
    if (error != 0) {
        DBG("Error during sample rate conversion: " << error);
        jassertfalse;
    }
    // output_frames_gen can be < output_frames due to SINC transport delay or internal buffering (libsamplerate FAQ).
    if (srcData.output_frames_gen < srcData.output_frames) {
        // Only the first output_frames_gen samples are valid for this call; clear unwritten tail to avoid stale data.
        outputBuffer.clear(0,
                               static_cast<int>(srcData.output_frames_gen),
                               static_cast<int>(srcData.output_frames - srcData.output_frames_gen));
    }
    return outputBuffer;
}

void ResamplingProcessor::setOutputBufferSize(int size)
{
    outputBufferSize = size;
    outputBuffer.setSize(1, size);
    outputBuffer.clear();
    if (inputBufferSize > 0) {
        bufferSizeRatio = calculateBufferSizeRatio(outputBufferSize, inputBufferSize);
    }
}

void ResamplingProcessor::releaseResources() {
    if (converter) {
        src_delete(converter);
        converter = nullptr;
    }
}

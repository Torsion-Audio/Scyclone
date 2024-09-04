#include <samplerate.h>
#include <ResamplingProcessor.h>

ResamplingProcessor::ResamplingProcessor() : converter(nullptr), inputBuffer(nullptr), outputBuffer(nullptr), srcRatio(1.0) {}
// Todo check for latency (prob. between 3 and 10 milliseconds) maxvalue at init

void ResamplingProcessor::prepare(double inputSampleRate, double outputSampleRate) {
    setSamplerateRatio(inputSampleRate, outputSampleRate);

    int error;
    converter = src_new(SRC_SINC_MEDIUM_QUALITY, 1, &error);

    if (error != 0 || !converter) {
        throw std::runtime_error(src_strerror(error));
    }

}

void ResamplingProcessor::processBlock(juce::AudioBuffer<float>& inputBufferMono, juce::AudioBuffer<float>& outputBufferMono) {
    if (!converter) {
        throw std::runtime_error("Sample rate converter not initialized");
    }

    SRC_DATA srcData;
    srcData.data_in = inputBufferMono.getReadPointer(0);
    srcData.input_frames = inputBufferMono.getNumSamples();
    srcData.data_out = outputBufferMono.getWritePointer(0);
    srcData.output_frames = outputBufferMono.getNumSamples();
    srcData.src_ratio = srcRatio;
    srcData.end_of_input = 0;  // This flag indicates if this is the last block of input

    // Perform the sample rate conversion
    int error = src_process(converter, &srcData);
    if (error != 0) {
        // Handle conversion error
        throw std::runtime_error("Error during sample rate conversion");
    }

    // Handle remaining frames in case the output buffer is not fully filled
    if (srcData.output_frames_gen < srcData.output_frames) {
        // Process remaining frames or handle end of data
    }
}

void ResamplingProcessor::releaseResources() {
    if (converter) {
        src_delete(converter);
        converter = nullptr;
    }
}



#include <samplerate.h>
#include <ResamplingProcessor.h>


ResamplingProcessor::ResamplingProcessor() : converter(nullptr), srcRatio(1.0), id_string("noNameSet") {}
// Todo check for latency (prob. between 3 and 10 milliseconds) maxvalue at init



int ResamplingProcessor::prepare(const juce::dsp::ProcessSpec &inputSpec, double targetSampleRate, std::string name) {
    id_string = name; // debugging
    inputSampleRate =  inputSpec.sampleRate;
    inputBufferSize = static_cast<int>(inputSpec.maximumBlockSize);
    outputSampleRate = targetSampleRate;

    setSamplerateRatio();

    int error;
    converter = src_new(SRC_SINC_MEDIUM_QUALITY, 1, &error);
    if (error != 0 || !converter) {
        throw std::runtime_error(src_strerror(error));
    }

    return outputBufferSize;
}

void ResamplingProcessor::setSamplerateRatio()
{

    // initial sample rate ratio
    srcRatio = outputSampleRate / inputSampleRate;  // According to src-doc Equal to output_sample_rate / input_sample_rate.

    outputBufferSize = static_cast<int>(std::ceil(srcRatio * inputBufferSize));

    // Adjust the resampling ratio to ensure that it evenly matches the buffer size
    srcRatio = static_cast<double>(outputBufferSize) / static_cast<double>(inputBufferSize);

    // Debugging information
    float timePerBlockInSec = static_cast<float>(inputBufferSize) / static_cast<float>(inputSampleRate);
    float correctedSampleRate = static_cast<float>(outputBufferSize) / timePerBlockInSec;

    outputBufferMono.setSize(1, outputBufferSize); // 1 channel (Mono), outputBufferSize samples

    std::cout << id_string << "\n";
    std::cout << "Samplerate Ratio Set: " << srcRatio << "\n";
    std::cout << "Corrected Sample Rate: " << correctedSampleRate << " Hz\n";
    std::cout << "------" << "\n";
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
    srcData.end_of_input = 0;  // This flag indicates if this is the last block of input TODO terminate properly

    // Perform the sample rate conversion
    int error = src_process(converter, &srcData);
    if (error != 0) {
        // Handle conversion error
        std::cout << "Error during sample rate conversion: " << std::to_string(error) << std::endl;
    }

    // Handle remaining frames in case the output buffer is not fully filled
    if (srcData.output_frames_gen < srcData.output_frames) {
        // Process remaining frames or handle end of data
        std::cout << "Remaining frames to process: "
                  << (srcData.output_frames - srcData.output_frames_gen)
                  << " for " << id_string << std::endl;
    }

    // make sense to return same pointer but switch internally?
    return outputBufferMono;
}

void ResamplingProcessor::releaseResources() {
    if (converter) {
        src_delete(converter);
        converter = nullptr;
    }
}



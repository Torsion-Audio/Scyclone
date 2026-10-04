#ifndef SCYCLONE_TEST_BACKENDRENDER_H
#define SCYCLONE_TEST_BACKENDRENDER_H

#include "JuceHeader.h"
#include "InferenceBackend.h"

#include <algorithm>
#include <vector>

/// Streams @p input through @p backend in fixed blocks of @p blockSize (the last one zero-padded)
/// and returns the output on the same sample clock as the input.
inline std::vector<float> renderThroughBackend(InferenceBackend& backend, const std::vector<float>& input,
                                               int blockSize)
{
    std::vector<float> output(input.size(), 0.0f);
    juce::AudioBuffer<float> buffer(1, blockSize);

    for (size_t position = 0; position < input.size(); position += static_cast<size_t>(blockSize))
    {
        const int count = static_cast<int>(std::min<size_t>(static_cast<size_t>(blockSize),
                                                            input.size() - position));
        buffer.clear();
        buffer.copyFrom(0, 0, input.data() + position, count);
        backend.processBlock(buffer);
        std::copy_n(buffer.getReadPointer(0), count,
                    output.begin() + static_cast<std::ptrdiff_t>(position));
    }
    return output;
}

#endif // SCYCLONE_TEST_BACKENDRENDER_H

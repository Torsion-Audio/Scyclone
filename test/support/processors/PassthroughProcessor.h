#pragma once

#include "dsp/IProcessor.h"

namespace resampling_test {

/** Zero-latency middle stage for up → middle → down chain tests. */
class PassthroughProcessor : public IProcessor {
public:
    void processBlock(juce::AudioBuffer<float>& buffer) override { juce::ignoreUnused(buffer); }
};

} // namespace resampling_test

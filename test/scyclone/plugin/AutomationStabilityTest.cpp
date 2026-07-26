#include <gtest/gtest.h>
#include "JuceFixture.h"
#include "PluginProcessor.h"

using namespace torsion::test;

namespace {

void waitUntilReadyToProcess(AudioPluginAudioProcessor& processor)
{
    juce::Thread::sleep(500);

    const auto deadline = juce::Time::getMillisecondCounter() + 20000;
    while (processor.isSuspended() && juce::Time::getMillisecondCounter() < deadline)
        juce::Thread::sleep(10);

    ASSERT_FALSE(processor.isSuspended()) << "processor still suspended (model load timed out)";
}

bool bufferHasNonFinite(const juce::AudioBuffer<float>& buffer)
{
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
        const float* data = buffer.getReadPointer(ch);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            if (! std::isfinite(data[i]))
                return true;
    }
    return false;
}

struct ProbeResult {
    int firstBadBlock = -1;
    int lastBadBlock = -1;
    int badBlockCount = 0;
};

ProbeResult probeParameter(AudioPluginAudioProcessor& processor,
                           juce::RangedAudioParameter* parameter,
                           int maxBlockSize)
{
    juce::Random random(42);
    juce::AudioBuffer<float> buffer(2, maxBlockSize);
    juce::MidiBuffer midi;
    ProbeResult result;

    constexpr int kNumBlocks = 400;
    constexpr int kMaxAttempts = 100000;
    float phase = 0.f;

    for (int block = 0, attempt = 0; block < kNumBlocks && attempt < kMaxAttempts; ++attempt, ++block) {
        if (parameter != nullptr)
            parameter->setValueNotifyingHost(random.nextFloat());

        const int numSamples = 1 + random.nextInt(maxBlockSize);
        buffer.setSize(2, numSamples, false, false, true);

        for (int i = 0; i < numSamples; ++i) {
            const float sample = 0.5f * std::sin(phase);
            phase += 0.05f;
            buffer.setSample(0, i, sample);
            buffer.setSample(1, i, sample);
        }

        {
            const juce::ScopedLock scopedLock(processor.getCallbackLock());
            if (processor.isSuspended()) {
                --block;
                continue;
            }

            processor.processBlock(buffer, midi);
        }

        if (bufferHasNonFinite(buffer)) {
            if (result.firstBadBlock < 0)
                result.firstBadBlock = block;
            result.lastBadBlock = block;
            ++result.badBlockCount;
        }
    }

    return result;
}

} // namespace

class AutomationStabilityTest : public JuceAudioTest {
protected:
    void SetUp() override
    {
        JuceAudioTest::SetUp();
#if !defined(SCYCLONE_ONNX_STUB) && defined(SCYCLONE_SKIP_PLUGIN_INTEGRATION_TEST)
        GTEST_SKIP() << "AutomationStabilityTest skipped: prebuilt ORT triggers Linux UBSan false positives";
#endif
    }
};

TEST_F(AutomationStabilityTest, VaryingBlockSizes_ProduceFiniteOutput) {
    AudioPluginAudioProcessor processor;
    processor.prepareToPlay(44100.0, 1024);
    waitUntilReadyToProcess(processor);

    const auto result = probeParameter(processor, nullptr, 1024);
    EXPECT_EQ(result.firstBadBlock, -1)
        << "non-finite output without any automation (first " << result.firstBadBlock
        << ", last " << result.lastBadBlock << ", count " << result.badBlockCount << ")";

    processor.releaseResources();
}

TEST_F(AutomationStabilityTest, PerParameterAutomation_ProducesFiniteOutput) {
    const int numParameters = [] {
        AudioPluginAudioProcessor prototype;
        return prototype.getParameters().size();
    }();

    for (int index = 0; index < numParameters; ++index) {
        AudioPluginAudioProcessor processor;
        processor.prepareToPlay(44100.0, 1024);
        waitUntilReadyToProcess(processor);

        auto* parameter = dynamic_cast<juce::RangedAudioParameter*>(processor.getParameters()[index]);
        ASSERT_NE(parameter, nullptr) << "parameter index " << index;

        const auto result = probeParameter(processor, parameter, 1024);
        EXPECT_EQ(result.firstBadBlock, -1)
            << "non-finite output while automating " << parameter->paramID
            << " (first " << result.firstBadBlock << ", last " << result.lastBadBlock
            << ", count " << result.badBlockCount << ")";

        processor.releaseResources();
    }
}

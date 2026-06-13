// Latency compensation tests:
// - Total latency formula (host + 48k delays -> host samples)
// - ONNX block-aligned latency (ceil(inference/block)*block - block)
// - Resampler getLatencyInSamples (SINC filter delay, typically 30–100 input samples)
// - Round-trip resample up/down and dry/wet alignment with reported total latency

#include <gtest/gtest.h>
#include <algorithm>
#include <cmath>
#include <vector>
#include "ResamplingTestHelpers.h"
#include "dsp/utils/utils.h"
#include "dsp/resampler/ResamplingProcessor.h"
#include "dsp/mixer/DryWetMixer.h"

using namespace resampling_test;

// --- computeTotalLatencyInSamples: delays at two rates -> host samples (std::round) ---
// Params: delayAtOutputRateSamples (in host/output rate), delayAtProcessingRate 1 & 2 (in 48k), processingRate, outputSampleRate (host).
// Formula: totalSeconds = delayAtOutputRate/outputSR + (delay1+delay2)/processingRate; return round(totalSeconds * outputSR).

TEST(LatencyCompensation, ComputeTotalLatencyInSamples_ZeroDelays) {
    EXPECT_EQ(utils::computeTotalLatencyInSamples(0, 0, 0, 48000.0, 44100.0), 0) << "zero delays -> 0 at any host rate";
    EXPECT_EQ(utils::computeTotalLatencyInSamples(0, 0, 0, 48000.0, 48000.0), 0) << "zero delays at 48k host";
}

TEST(LatencyCompensation, ComputeTotalLatencyInSamples_OnnxOnlyAt48k) {
    // 20480 samples @ 48k -> at 44.1k host: 20480/48000 * 44100 = 18816
    EXPECT_EQ(utils::computeTotalLatencyInSamples(0, 20480, 0, 48000.0, 44100.0), 18816)
        << "ONNX delay at 48k converted to 44.1k host samples";
}

TEST(LatencyCompensation, ComputeTotalLatencyInSamples_EdgeCasesHostRates) {
    EXPECT_EQ(utils::computeTotalLatencyInSamples(0, 100, 200, 48000.0, 48000.0), 300)
        << "all at 48k: 100+200 = 300 host samples";
    // 300 samples @ 48k -> 300/48000*44100 = 276.25 -> 276
    EXPECT_EQ(utils::computeTotalLatencyInSamples(0, 100, 200, 48000.0, 44100.0), 276)
        << "same 48k delay expressed at 44.1k host";
}

TEST(LatencyCompensation, ComputeTotalLatencyInSamples_OutputRateDelayOnly) {
    // First param is delay already in output (host) rate samples; result is in same rate. So 1024 @ 44.1k -> 1024.
    EXPECT_EQ(utils::computeTotalLatencyInSamples(1024, 0, 0, 48000.0, 44100.0), 1024)
        << "delay at output (host 44.1k) rate only -> result 1024 host samples";
}

TEST(LatencyCompensation, ComputeTotalLatencyInSamples_Combined) {
    // Full chain: upsampler (46 input samples) + ONNX (20480 @ 48k) + downsampler (46 @ 48k); host 44.1k.
    // totalSec = delay_at_host_rate/hostSR + delay_at_48k/48k; result = round(totalSec * hostSR).
    double totalSec = 46.0 / 44100.0 + (20480.0 + 46.0) / 48000.0;
    int expected = static_cast<int>(std::round(totalSec * 44100.0));
    EXPECT_EQ(utils::computeTotalLatencyInSamples(46, 20480, 46, 48000.0, 44100.0), expected)
        << "combined up + ONNX + down latency in host (44.1k) samples";
}

// --- computeOnnxLatencyInSamples: ceil(inference/block)*block - block (block-aligned latency) ---

TEST(LatencyCompensation, ComputeOnnxLatencyInSamples_ExactBlocks) {
    // 20480 / 512 = 40 blocks -> latency = 40*512 - 512 = 19968
    EXPECT_EQ(utils::computeOnnxLatencyInSamples(20480, 512), 19968)
        << "block-aligned ONNX latency: ceil(20480/512)*512 - 512";
}

TEST(LatencyCompensation, ComputeOnnxLatencyInSamples_VerySmallBlock) {
    EXPECT_EQ(utils::computeOnnxLatencyInSamples(20480, 32), 20448)
        << "small block 32 -> 640 blocks, latency 20480 - 32";
}

TEST(LatencyCompensation, ComputeOnnxLatencyInSamples_VeryLargeBlock) {
    // 20480/8192 = 2.5 -> 3 blocks; latency = 3*8192 - 8192 = 16384
    EXPECT_EQ(utils::computeOnnxLatencyInSamples(20480, 8192), 16384)
        << "large block rounds up to 3 blocks";
}

TEST(LatencyCompensation, ComputeOnnxLatencyInSamples_BlockSizes) {
    // Formula: ceil(inference/block)*block - block for a range of block sizes.
    const int inference = 20480;
    EXPECT_EQ(utils::computeOnnxLatencyInSamples(inference, 32), 20448) << "block 32";
    EXPECT_EQ(utils::computeOnnxLatencyInSamples(inference, 64), 20416) << "block 64";
    EXPECT_EQ(utils::computeOnnxLatencyInSamples(inference, 256), 20224) << "block 256";
    EXPECT_EQ(utils::computeOnnxLatencyInSamples(inference, 512), 19968) << "block 512";
    EXPECT_EQ(utils::computeOnnxLatencyInSamples(inference, 1024), 19456) << "block 1024";
    EXPECT_EQ(utils::computeOnnxLatencyInSamples(inference, 2048), 18432) << "block 2048";
    EXPECT_EQ(utils::computeOnnxLatencyInSamples(inference, 4096), 16384) << "block 4096";
}

// --- Resampler getLatencyInSamples: SINC filter delay in input-rate samples ---
// Formula (src_sinc.c): count = (coeff_half_len+2)/index_inc = 22438/491 ≈ 45.7.
// For downsampling (srcRatio < 1): count /= srcRatio — e.g. 96k->48k (ratio=0.5) -> ~92 input samples.
// Upper bound 100 gives comfortable margin above the 92-sample worst case.

TEST(LatencyCompensation, Resampler_GetLatencyInSamples_AfterPrepare) {
    juce::ScopedJuceInitialiser_GUI init;
    ResamplingProcessor resampler;
    juce::dsp::ProcessSpec spec{ 44100.0, 512, 1 };
    resampler.prepare(spec, 48000.0, "test");
    int lat = resampler.getLatencyInSamples();
    EXPECT_GT(lat, 0) << "SRC_SINC_MEDIUM_QUALITY should report positive latency";
    EXPECT_GE(lat, 30) << "expected at least 30 input-rate samples";
    EXPECT_LE(lat, 100) << "expected at most 100 input-rate samples (96k->48k worst case ~92)";
}

// Multiple sample rates and block sizes: reported latency in expected range.
// Worst case: 96k->48k downsampling -> half_filter_chan_len = 22438/491/0.5 ≈ 92 input samples.
TEST(LatencyCompensation, Resampler_GetLatencyInSamples_VariousSRPairs) {
    juce::ScopedJuceInitialiser_GUI init;
    for (double hostSR : { 44100.0, 48000.0, 96000.0 }) {
        for (uint32_t blockSize : { 32u, 64u, 2048u, 8192u }) {
            ResamplingProcessor up;
            juce::dsp::ProcessSpec spec{ hostSR, blockSize, 1 };
            up.prepare(spec, 48000.0, "up");
            int lat = up.getLatencyInSamples();
            EXPECT_GT(lat, 0) << "hostSR=" << hostSR << " blockSize=" << blockSize;
            EXPECT_GE(lat, 30) << "expected at least 30 samples, hostSR=" << hostSR << " blockSize=" << blockSize;
            EXPECT_LE(lat, 100) << "expected at most 100 samples, hostSR=" << hostSR << " blockSize=" << blockSize;
        }
    }
}

// --- Resampler round-trip: impulse up then down; peak near upLat + round(downLat*hostSR/48k) ---
// Pre-roll with silence first to reach steady-state streaming behavior, then inject impulse.

TEST(LatencyCompensation, Resampler_RoundTrip_PeakWithinTolerance) {
    juce::ScopedJuceInitialiser_GUI init;
    const double hostSR = 44100.0;
    const uint32_t blockSize = 512;
    auto chain = prepareRoundTripChain(hostSR, static_cast<int>(blockSize));
    const int upLat = chain.up.getLatencyInSamples();
    const int downLat = chain.down.getLatencyInSamples();
    const int expectedPeak = upLat + static_cast<int>(std::round(static_cast<double>(downLat) * hostSR / kOnnxRate));

    runSilencePreRoll(chain, 4);

    chain.hostBuffer.setSample(0, 0, 1.0f);
    const int collectSamples = expectedPeak + static_cast<int>(blockSize);
    std::vector<float> collected;
    collected.reserve(static_cast<size_t>(collectSamples));

    while (static_cast<int>(collected.size()) < collectSamples) {
        juce::AudioBuffer<float>& upOut = chain.up.processBlock(chain.hostBuffer);
        juce::AudioBuffer<float>& downOut = chain.down.processBlock(upOut);
        for (int i = 0; i < downOut.getNumSamples(); ++i) {
            collected.push_back(downOut.getSample(0, i));
        }
        chain.hostBuffer.clear();
    }

    const int searchStart = std::max(0, expectedPeak - 5);
    const int searchEnd = std::min(static_cast<int>(collected.size()), expectedPeak + static_cast<int>(blockSize));
    const int peakPos = findImpulsePeak(collected, searchStart, searchEnd);
    EXPECT_GE(peakPos, expectedPeak - 5) << "peak within 5 samples (early), expectedPeak=" << expectedPeak;
    EXPECT_LE(peakPos, expectedPeak + 5) << "peak within 5 samples (late), expectedPeak=" << expectedPeak;
}

TEST(LatencyCompensation, Resampler_SteadyState_FullFillAfterPreRoll) {
    juce::ScopedJuceInitialiser_GUI init;
    auto chain = prepareRoundTripChain(44100.0, 512);
    runSilencePreRoll(chain, 4);
    chain.hostBuffer.clear();
    for (int n = 0; n < 4; ++n) {
        juce::AudioBuffer<float>& upOut = chain.up.processBlock(chain.hostBuffer);
        assertSteadyStateFullOutput(chain.up);
        juce::AudioBuffer<float>& downOut = chain.down.processBlock(upOut);
        assertSteadyStateFullOutput(chain.down);
        EXPECT_EQ(downOut.getNumSamples(), 512);
    }
}

// --- Dry/wet alignment: up -> passthrough -> down; mix dry+wet with setWetLatency(total) ---
// Pre-roll with silence first to avoid startup transient, then inject impulse and verify alignment.

static void dryWetAlignmentMultiBlock(double hostSR, uint32_t blockSize) {
    auto chain = prepareRoundTripChain(hostSR, static_cast<int>(blockSize));
    PassthroughProcessor passthrough;
    const int totalLatency = utils::computeTotalLatencyInSamples(
        chain.up.getLatencyInSamples(), 0, chain.down.getLatencyInSamples(), kOnnxRate, hostSR);

    runSilencePreRoll(chain, 4);

    const int collectSamples = totalLatency + static_cast<int>(blockSize);
    std::vector<float> wetCollected;
    std::vector<float> dryCollected;
    wetCollected.reserve(static_cast<size_t>(collectSamples));
    dryCollected.reserve(static_cast<size_t>(collectSamples));

    chain.hostBuffer.setSample(0, 0, 1.0f);

    while (static_cast<int>(wetCollected.size()) < collectSamples) {
        for (int i = 0; i < chain.hostBuffer.getNumSamples(); ++i) {
            dryCollected.push_back(chain.hostBuffer.getSample(0, i));
        }

        juce::AudioBuffer<float>& upOut = chain.up.processBlock(chain.hostBuffer);
        passthrough.processBlock(upOut);
        juce::AudioBuffer<float>& wetOut = chain.down.processBlock(upOut);
        for (int i = 0; i < wetOut.getNumSamples(); ++i) {
            wetCollected.push_back(wetOut.getSample(0, i));
        }

        chain.hostBuffer.clear();
    }

    const int mixLen = std::min(static_cast<int>(wetCollected.size()),
                                static_cast<int>(dryCollected.size()));
    juce::AudioBuffer<float> dryBuf(1, mixLen);
    juce::AudioBuffer<float> mixBuf(1, mixLen);
    for (int i = 0; i < mixLen; ++i) {
        dryBuf.setSample(0, i, dryCollected[static_cast<size_t>(i)]);
        mixBuf.setSample(0, i, wetCollected[static_cast<size_t>(i)]);
    }
    DryWetMixer mixer;
    mixer.prepare(juce::dsp::ProcessSpec{ hostSR, static_cast<uint32_t>(mixLen), 1 });
    mixer.setWetLatency(totalLatency);
    mixer.setDryWetProportion(0.5f);
    mixer.setDrySamples(dryBuf);
    mixer.setWetSamples(mixBuf);

    const int searchStart = std::max(0, totalLatency - 10);
    const int searchEnd = std::min(mixLen, totalLatency + 11);
    std::vector<float> mixed(mixBuf.getReadPointer(0), mixBuf.getReadPointer(0) + mixLen);
    const int peakPos = findImpulsePeak(mixed, searchStart, searchEnd);
    EXPECT_GE(mixed[static_cast<size_t>(std::max(0, peakPos))], 0.01f)
        << "mixed signal should have a detectable peak; totalLatency=" << totalLatency;
    EXPECT_GE(peakPos, totalLatency - 5) << "peak aligned within 5 samples (early); totalLatency=" << totalLatency;
    EXPECT_LE(peakPos, totalLatency + 5) << "peak aligned within 5 samples (late); totalLatency=" << totalLatency;
}

TEST(LatencyCompensation, DryWetAlignment_BlockSize512_44100) {
    juce::ScopedJuceInitialiser_GUI init;
    dryWetAlignmentMultiBlock(44100.0, 512);
}

TEST(LatencyCompensation, DryWetAlignment_BlockSize2048_44100) {
    juce::ScopedJuceInitialiser_GUI init;
    dryWetAlignmentMultiBlock(44100.0, 2048);
}

TEST(LatencyCompensation, DryWetAlignment_BlockSize512_48000) {
    juce::ScopedJuceInitialiser_GUI init;
    dryWetAlignmentMultiBlock(48000.0, 512);
}

TEST(LatencyCompensation, DryWetAlignment_BlockSize2048_48000) {
    juce::ScopedJuceInitialiser_GUI init;
    dryWetAlignmentMultiBlock(48000.0, 2048);
}

TEST(LatencyCompensation, DryWetAlignment_BlockSize32) {
    juce::ScopedJuceInitialiser_GUI init;
    dryWetAlignmentMultiBlock(44100.0, 32);
}

TEST(LatencyCompensation, DryWetAlignment_BlockSize64) {
    juce::ScopedJuceInitialiser_GUI init;
    dryWetAlignmentMultiBlock(44100.0, 64);
}

TEST(LatencyCompensation, DryWetAlignment_BlockSize2048) {
    juce::ScopedJuceInitialiser_GUI init;
    dryWetAlignmentMultiBlock(44100.0, 2048);
}

TEST(LatencyCompensation, DryWetAlignment_BlockSize8192) {
    juce::ScopedJuceInitialiser_GUI init;
    dryWetAlignmentMultiBlock(44100.0, 8192);
}

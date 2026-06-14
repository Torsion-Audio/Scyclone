// Uncoupled libsamplerate sizing (not production mode).
// Documents the ceil +1 footgun; production avoids this via forcedOutputBlockSize.

#include <gtest/gtest.h>
#include "ResamplingTopology.h"

using namespace scyclone::test::resampling;

// Signal: host N → up → ONNX-sized block. Asserts prepare() reports ceil(host→48k) sizing.
TEST(ResamplingIsolation, UpOnly_OutputSize) {
    const auto ratioCase = makeProductionRatioCase(44100.0, 512);
    ResamplingProcessor up;
    const int upOut = prepareUpOnly(ratioCase.hostSR, ratioCase.hostBlock, up);
    EXPECT_EQ(upOut, ratioCase.expectedUpOut);
    EXPECT_EQ(up.getOutputBufferSize(), ratioCase.expectedUpOut);
}

// Signal: ONNX N_up → down (uncoupled, no forced host block) → ceil(host/48k)×N_up (+1 at 44.1k/512).
TEST(ResamplingIsolation, DownOnly_UncoupledCeilProducesPlusOne) {
    const auto ratioCase = makeProductionRatioCase(44100.0, 512);
    ResamplingProcessor down;
    const int downOut = prepareDownOnly(ratioCase.hostSR, ratioCase.expectedUpOut, ratioCase.hostBlock, down);
    const int expectedUncoupled = uncoupledDownOutputSize(ratioCase.hostSR, ratioCase.expectedUpOut);
    EXPECT_EQ(downOut, expectedUncoupled) << "uncoupled ceil on down produces +1 at 44.1k/512";
    EXPECT_EQ(down.getOutputBufferSize(), expectedUncoupled);
}

// Signal: 48 kHz host — up and down are 1:1; both buffers equal host block.
TEST(ResamplingIsolation, SameRate_Passthrough) {
    const auto ratioCase = makeProductionRatioCase(48000.0, 512);
    ResamplingProcessor up, down;
    prepareUpOnly(ratioCase.hostSR, ratioCase.hostBlock, up);
    prepareDownOnly(ratioCase.hostSR, ratioCase.hostBlock, ratioCase.hostBlock, down);
    EXPECT_EQ(up.getOutputBufferSize(), ratioCase.hostBlock);
    EXPECT_EQ(down.getOutputBufferSize(), ratioCase.hostBlock);
}

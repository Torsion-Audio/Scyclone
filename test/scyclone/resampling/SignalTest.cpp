// Resampler-only signal quality — FFT SNR gate (libsamplerate snr_bw_test pattern).

#include <gtest/gtest.h>
#include "JuceFixture.h"
#include "ResamplingMeasurements.h"
#include "ResamplingSignalUtils.h"
#include "ResamplingTopology.h"

using namespace torsion::test;
using namespace scyclone::test::resampling;

class ResamplingSignalTest : public JuceAudioTest, public ::testing::WithParamInterface<SnrCase>
{
};

// Signal: Hanning windowed sine → up only → FFT peak SNR after latency skip.
TEST_P(ResamplingSignalTest, UpOnly_WindowedSine_SnrAboveThreshold)
{
    const auto &snrCase = GetParam();
    if (!isFeasibleHostConfig(snrCase.hostSR, snrCase.hostBlock))
    {
        GTEST_SKIP();
    }

    const double snr = measureUpSnrDb(snrCase.hostSR, snrCase.hostBlock, snrCase.passBandPeaks);
    ASSERT_GE(snr, 0.0) << "SNR measurement failed";
    EXPECT_GE(snr, snrCase.minUpSnrDb)
        << "hostSR=" << snrCase.hostSR << " block=" << snrCase.hostBlock;
}

// Signal: Hanning windowed sine @ 48 kHz block → down (forced host N) → FFT peak SNR.
TEST_P(ResamplingSignalTest, DownOnly_WindowedSine_SnrAboveThreshold)
{
    const auto &snrCase = GetParam();
    if (!isFeasibleHostConfig(snrCase.hostSR, snrCase.hostBlock))
    {
        GTEST_SKIP();
    }

    const double snr = measureDownSnrDb(snrCase.hostSR, snrCase.hostBlock, snrCase.passBandPeaks);
    ASSERT_GE(snr, 0.0) << "SNR measurement failed";
    EXPECT_GE(snr, snrCase.minDownSnrDb)
        << "hostSR=" << snrCase.hostSR << " block=" << snrCase.hostBlock;
}

namespace
{

    std::string snrCaseName(const ::testing::TestParamInfo<SnrCase> &info)
    {
        return std::to_string(static_cast<int>(info.param.hostSR)) + "_" + std::to_string(info.param.hostBlock);
    }

} // namespace

INSTANTIATE_TEST_SUITE_P(ResamplingSignal, ResamplingSignalTest,
                         ::testing::ValuesIn(defaultCiSnrCases()), snrCaseName);

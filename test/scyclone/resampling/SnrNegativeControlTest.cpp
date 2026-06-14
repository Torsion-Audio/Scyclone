// SNR gate sensitivity — non-linear corruption must drop measured SNR below CI floors.
// Why: linear gain alone is a weak negative control

#include <gtest/gtest.h>
#include "JuceFixture.h"
#include "ResamplingMeasurements.h"
#include "ResamplingSignalUtils.h"
#include "ResamplingTopology.h"

using namespace scyclone::test::resampling;

class SnrNegativeControlTest : public torsion::test::JuceAudioTest
{
};

TEST_F(SnrNegativeControlTest, UpOnly_HardClip_DropsBelowCiFloor)
{
    for (const auto &snrCase : defaultCiSnrCases())
    {
        if (!isFeasibleHostConfig(snrCase.hostSR, snrCase.hostBlock))
        {
            continue;
        }

        const double clean = measureUpSnrDb(
            snrCase.hostSR, snrCase.hostBlock, snrCase.passBandPeaks, SnrCorruptionKind::None);
        const double corrupt = measureUpSnrDb(
            snrCase.hostSR, snrCase.hostBlock, snrCase.passBandPeaks, SnrCorruptionKind::HardClip);

        ASSERT_GE(clean, 0.0) << "hostSR=" << snrCase.hostSR << " block=" << snrCase.hostBlock;
        EXPECT_GE(clean, snrCase.minUpSnrDb)
            << "hostSR=" << snrCase.hostSR << " block=" << snrCase.hostBlock;
        EXPECT_LT(corrupt, snrCase.minUpSnrDb)
            << "hard clip must fail SNR gate; hostSR=" << snrCase.hostSR << " block=" << snrCase.hostBlock
            << " clean=" << clean << " corrupt=" << corrupt;
    }
}

TEST_F(SnrNegativeControlTest, DownOnly_AdditiveNoise_DropsBelowCiFloor)
{
    for (const auto &snrCase : defaultCiSnrCases())
    {
        if (!isFeasibleHostConfig(snrCase.hostSR, snrCase.hostBlock))
        {
            continue;
        }

        const double clean = measureDownSnrDb(
            snrCase.hostSR, snrCase.hostBlock, snrCase.passBandPeaks, SnrCorruptionKind::None);
        const double corrupt = measureDownSnrDb(
            snrCase.hostSR, snrCase.hostBlock, snrCase.passBandPeaks, SnrCorruptionKind::AdditiveNoise);

        ASSERT_GE(clean, 0.0) << "hostSR=" << snrCase.hostSR << " block=" << snrCase.hostBlock;
        EXPECT_GE(clean, snrCase.minDownSnrDb)
            << "hostSR=" << snrCase.hostSR << " block=" << snrCase.hostBlock;
        EXPECT_LT(corrupt, snrCase.minDownSnrDb)
            << "additive noise must fail SNR gate; hostSR=" << snrCase.hostSR << " block=" << snrCase.hostBlock
            << " clean=" << clean << " corrupt=" << corrupt;
    }
}

// Calibration probe — measures worst-case tolerances for behavioral contract tests.
// Re-run when resampler or ONNX latency config changes; update ResamplingContractAssertions.h / ResamplingSignalUtils.h.
//
// Run: Test.exe --gtest_filter=*PrintToleranceMeasurements* --gtest_also_run_disabled_tests
// SNR:  Test.exe --gtest_filter=*PrintSnrMeasurements* --gtest_also_run_disabled_tests

#include <gtest/gtest.h>
#include <iostream>
#include <limits>
#include "DryWetContract.h"
#include "HostConfig.h"
#include "ImpulseMetrics.h"
#include "JuceFixture.h"
#include "PassthroughProcessor.h"
#include "ResamplingMeasurements.h"
#include "ResamplingTopology.h"
#include "ScycloneHostPresets.h"

using namespace torsion::test;
using namespace scyclone::test::mixer;
using namespace scyclone::test::resampling;

namespace
{

    void probeConfig(double hostSR, int hostBlock,
                     float &worstRoundTrip, HostConfig &worstRoundTripCfg,
                     float &worstProduction, HostConfig &worstProductionCfg,
                     int &worstDiracJitter, HostConfig &worstDiracCfg)
    {
        if (!isFeasibleHostConfig(hostSR, hostBlock))
        {
            return;
        }

        auto chain = prepareRoundTripChain(hostSR, hostBlock);
        PassthroughProcessor passthrough;
        const float roundRms = measureRoundTripRmsError(chain, hostSR, hostBlock, passthrough);
        std::cout << "CALIBRATION round_trip_rms hostSR=" << static_cast<int>(hostSR)
                  << " block=" << hostBlock << " measured=" << roundRms << "\n";
        if (roundRms > worstRoundTrip)
        {
            worstRoundTrip = roundRms;
            worstRoundTripCfg = {hostSR, hostBlock};
        }

        auto prod = prepareProductionChain(hostSR, hostBlock);
        const float prodRms = measureProductionRmsError(prod, hostSR, hostBlock);
        std::cout << "CALIBRATION production_rms hostSR=" << static_cast<int>(hostSR)
                  << " block=" << hostBlock << " measured=" << prodRms << "\n";
        if (prodRms > worstProduction)
        {
            worstProduction = prodRms;
            worstProductionCfg = {hostSR, hostBlock};
        }

        const int jitter = measureDryWetDiracJitter(hostSR, hostBlock, kCalibrationDiracSearchHalfWindow);
        std::cout << "CALIBRATION dirac_jitter hostSR=" << static_cast<int>(hostSR)
                  << " block=" << hostBlock << " measured=" << jitter << "\n";
        if (jitter < std::numeric_limits<int>::max() && jitter > worstDiracJitter)
        {
            worstDiracJitter = jitter;
            worstDiracCfg = {hostSR, hostBlock};
        }
    }

} // namespace

class CalibrationProbeTest : public JuceAudioTest
{
};

// Probe: impulse through production chain; compare wide vs narrow peak search vs reported latency.
TEST_F(CalibrationProbeTest, DISABLED_LatencyAudit)
{
    for (const auto &cfg : defaultCiHostConfigs())
    {
        if (!isFeasibleHostConfig(cfg.hostSR, cfg.hostBlock))
        {
            continue;
        }

        auto prod = prepareProductionChain(cfg.hostSR, cfg.hostBlock);
        const int reported = productionChainTotalLatency(prod, cfg.hostSR);
        const int upLat = prod.resamplers.up.getLatencyInSamples();
        const int onnxLat = prod.onnx.getLatencyInSamples();
        const int downLat = prod.resamplers.down.getLatencyInSamples();
        const int summedRounded = upLat + static_cast<int>(std::round(static_cast<double>(onnxLat) * cfg.hostSR / kOnnxRate)) + static_cast<int>(std::round(static_cast<double>(downLat) * cfg.hostSR / kOnnxRate));

        const ImpulseResponse impulse = measureProductionImpulse(prod, cfg.hostSR, cfg.hostBlock);

        std::cout << "AUDIT impulse hostSR=" << static_cast<int>(cfg.hostSR)
                  << " block=" << cfg.hostBlock
                  << " reported=" << reported
                  << " summedRounded=" << summedRounded
                  << " up=" << upLat << " onnx=" << onnxLat << " down=" << downLat
                  << " widePeak=" << impulse.widePeakPos
                  << " wideDelta=" << (impulse.widePeakPos - reported)
                  << " narrowPeak=" << impulse.narrowPeakPos
                  << " narrowDelta=" << (impulse.narrowPeakPos - reported)
                  << " narrowPeakVal=" << impulse.narrowPeakVal
                  << " widePeakVal=" << impulse.widePeakVal
                  << " maxOutside=" << impulse.maxOutsideMainWindow
                  << " narrowWouldPass="
                  << (impulse.narrowPeakPos >= reported - kImpulsePeakToleranceSamples && impulse.narrowPeakPos <= reported + kImpulsePeakToleranceSamples)
                  << " (see docs/resampling_architecture.md open cross-rate group-delay issue)\n";
    }
}

// Probe: sweep default CI + alignment edge configs; print worst round-trip RMS, production RMS, dirac jitter for tolerance tuning.
TEST_F(CalibrationProbeTest, DISABLED_PrintToleranceMeasurements)
{
    float worstRoundTrip = 0.0f;
    float worstProduction = 0.0f;
    int worstDiracJitter = 0;
    HostConfig worstRoundTripCfg{};
    HostConfig worstProductionCfg{};
    HostConfig worstDiracCfg{};

    const auto configs = dryWetHostConfigs();

    for (const auto &cfg : configs)
    {
        probeConfig(cfg.hostSR, cfg.hostBlock,
                    worstRoundTrip, worstRoundTripCfg,
                    worstProduction, worstProductionCfg,
                    worstDiracJitter, worstDiracCfg);
    }

    std::cout << "CALIBRATION worst_round_trip_rms measured=" << worstRoundTrip
              << " config=" << static_cast<int>(worstRoundTripCfg.hostSR)
              << "/" << worstRoundTripCfg.hostBlock << "\n";
    std::cout << "CALIBRATION worst_production_rms measured=" << worstProduction
              << " config=" << static_cast<int>(worstProductionCfg.hostSR)
              << "/" << worstProductionCfg.hostBlock << "\n";
    std::cout << "CALIBRATION worst_dirac_jitter measured=" << worstDiracJitter
              << " config=" << static_cast<int>(worstDiracCfg.hostSR)
              << "/" << worstDiracCfg.hostBlock << "\n";
    std::cout << "CALIBRATION suggested_round_trip_tolerance=" << worstRoundTrip * 1.5f << "\n";
    std::cout << "CALIBRATION suggested_production_tolerance=" << worstProduction * 1.5f << "\n";
    std::cout << "CALIBRATION suggested_dirac_tolerance=" << (worstDiracJitter + 2) << "\n";
}

// Probe: measure up/down FFT SNR per config; print suggested floors (measured − 3 dB).
TEST_F(CalibrationProbeTest, DISABLED_PrintSnrMeasurements)
{
    double worstUp = 0.0;
    double worstDown = 0.0;
    HostConfig worstUpCfg{};
    HostConfig worstDownCfg{};

    const auto configs = dryWetHostConfigs();

    for (const auto &cfg : configs)
    {
        if (!isFeasibleHostConfig(cfg.hostSR, cfg.hostBlock))
        {
            continue;
        }

        const double upSnr = measureUpSnrDb(cfg.hostSR, cfg.hostBlock);
        std::cout << "CALIBRATION up_snr hostSR=" << static_cast<int>(cfg.hostSR)
                  << " block=" << cfg.hostBlock << " measured=" << upSnr << " dB\n";
        if (upSnr > worstUp)
        {
            worstUp = upSnr;
            worstUpCfg = cfg;
        }

        const double downSnr = measureDownSnrDb(cfg.hostSR, cfg.hostBlock);
        std::cout << "CALIBRATION down_snr hostSR=" << static_cast<int>(cfg.hostSR)
                  << " block=" << cfg.hostBlock << " measured=" << downSnr << " dB\n";
        if (downSnr > worstDown)
        {
            worstDown = downSnr;
            worstDownCfg = cfg;
        }
    }

    std::cout << "CALIBRATION worst_up_snr measured=" << worstUp
              << " config=" << static_cast<int>(worstUpCfg.hostSR)
              << "/" << worstUpCfg.hostBlock << "\n";
    std::cout << "CALIBRATION worst_down_snr measured=" << worstDown
              << " config=" << static_cast<int>(worstDownCfg.hostSR)
              << "/" << worstDownCfg.hostBlock << "\n";
    std::cout << "CALIBRATION suggested_up_snr_floor=" << (worstUp - 3.0) << " dB\n";
    std::cout << "CALIBRATION suggested_down_snr_floor=" << (worstDown - 3.0) << " dB\n";
}

// Probe: sweep alignment lag ±256 around reported latency at 44.1k/128; find best swept-sine RMS lag.
TEST_F(CalibrationProbeTest, DISABLED_ProductionSineLagSweep)
{
    const HostConfig cfg{44100.0, 128};
    auto chain = prepareProductionChain(cfg.hostSR, cfg.hostBlock);
    const int reported = productionChainTotalLatency(chain, cfg.hostSR);

    float bestRms = std::numeric_limits<float>::max();
    int bestLag = reported;
    for (int lag = reported - 256; lag <= reported + 256; ++lag)
    {
        if (lag < 0)
        {
            continue;
        }
        auto sweepChain = prepareProductionChain(cfg.hostSR, cfg.hostBlock);
        const float err = measureProductionRmsErrorAtLatency(sweepChain, cfg.hostSR, cfg.hostBlock, lag);
        if (err < bestRms)
        {
            bestRms = err;
            bestLag = lag;
        }
    }
    std::cout << "LAG_SWEEP hostSR=44100 block=128 reported=" << reported
              << " bestLag=" << bestLag << " bestRms=" << bestRms << "\n";

    auto atReportedChain = prepareProductionChain(cfg.hostSR, cfg.hostBlock);
    const float atReported = measureProductionRmsErrorAtLatency(
        atReportedChain, cfg.hostSR, cfg.hostBlock, reported);
    std::cout << "LAG_SWEEP atReported=" << atReported << "\n";
}

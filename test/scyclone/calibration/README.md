# Resampling calibration probes

`ResamplingProbeTest.cpp` contains **DISABLED** gtest cases used to measure tolerances and latency — not part of normal CI.

Run when:

- The libsamplerate / resampler configuration changes
- `SimulatedOnnxProcessor` or `InferenceThread` model latency changes
- Production chain tests fail and you need to distinguish measurement drift from real regressions

Configure first with `cmake --preset default` and `cmake --build --preset test` (see [test/README.md](../README.md)).

```powershell
# RMS / dirac tolerance table (update ResamplingContractAssertions.h / DryWetContract.h)
.\build\Test.exe --gtest_filter=*PrintToleranceMeasurements* --gtest_also_run_disabled_tests

# SNR floors (update defaultCiSnrCases() in ResamplingSignalUtils.h — measured − 3 dB)
.\build\Test.exe --gtest_filter=*PrintSnrMeasurements* --gtest_also_run_disabled_tests

# Impulse peak vs reported latency (narrow search window validation)
.\build\Test.exe --gtest_filter=*LatencyAudit* --gtest_also_run_disabled_tests

# Swept-sine lag sweep (group delay vs productionChainTotalLatency)
.\build\Test.exe --gtest_filter=*ProductionSineLagSweep* --gtest_also_run_disabled_tests
```

Policy: do **not** relax production swept-sine / impulse CI tests based on probe output alone. Production signal suites use `productionSignalContractConfigs()` (48 kHz only); see architecture doc for cross-rate group-delay issue. Probes track drift until product latency reporting is fixed.

# Calibration probes

`ResamplingProbeTest.cpp` and `OnnxLatencyProbeTest.cpp` contain **DISABLED** gtest cases used to measure tolerances and latency — not part of normal CI.

Run when:

- The libsamplerate / resampler configuration changes
- `ScycloneModelConfig`, anira, or the ONNX Runtime version changes — re-measure `kOnnxInferenceLatencySamples`
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

# anira ONNX latency across host block sizes (update kOnnxInferenceLatencySamples in
# source/dsp/onnx/OnnxInferenceLatency.h from the 48 kHz / 512 row)
.\build\Test.exe --gtest_filter=*PrintOnnxLatencyMeasurements* --gtest_also_run_disabled_tests
```

`OnnxProcessorContractTest.ReportedLatency_MatchesCalibratedConstantAtReferenceConfig` pins the
constant against the live backend, so drift fails the build rather than going unnoticed.

Policy: do **not** relax production swept-sine / impulse CI tests based on probe output alone. Production signal suites use `productionSignalContractConfigs()` (48 kHz only); see architecture doc for cross-rate group-delay issue. Probes track drift until product latency reporting is fixed.

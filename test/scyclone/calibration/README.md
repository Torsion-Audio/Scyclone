# Calibration probes

`ResamplingProbeTest.cpp`, `OnnxLatencyProbeTest.cpp` and `OnnxWetLagProbeTest.cpp` contain **DISABLED** gtest cases used to measure tolerances and latency — not part of normal CI.

Run when:

- The libsamplerate / resampler configuration changes
- `ScycloneModelConfig`, anira, the ONNX Runtime version or a bundled model changes — re-measure `kOnnxInferenceLatencySamples` and the models' real wet latency (`internal_model_latency`)
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

# Real wet-signal lag of each RAVE model vs the reported latency, from decaying noise bursts at
# eight positions within the 2048-sample hop (update internal_model_latency in
# ScycloneModelConfig.cpp if hop_aligned_onset drifts away from the reported latency)
.\build\Test.exe --gtest_filter=*PrintWetLagMeasurements* --gtest_also_run_disabled_tests
```

`OnnxProcessorContractTest.ReportedLatency_MatchesCalibratedConstantAtReferenceConfig` pins the
constant against the live backend, so drift fails the build rather than going unnoticed.
`AniraWetAlignmentTest` pins the real FunkDrum wet signal against the reported latency, so a wrong
`internal_model_latency` fails too.

Measured with ORT 1.26.0 / anira v2.3.0 (48 kHz, 512-sample blocks, reported 3584): FunkDrum
emits each transient at the start of its output hop (hop-aligned onset within about ±100 samples
of the reported latency; the per-burst lag runs from about one hop early to on time depending on
where in the hop the transient fell). Djembe's responses are too mixed with transients the model
generates itself for this estimator to pin.

Policy: do **not** relax production swept-sine / impulse CI tests based on probe output alone. Production signal suites use `productionSignalContractConfigs()` (48 kHz only); see architecture doc for cross-rate group-delay issue. Probes track drift until product latency reporting is fixed.

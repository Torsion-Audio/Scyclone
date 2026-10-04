#ifndef SCYCLONE_ONNXINFERENCELATENCY_H
#define SCYCLONE_ONNXINFERENCELATENCY_H

// Raw ONNX-island latency in samples, as reported by anira's InferenceHandler::get_latency().
//
// This is NOT a property of the model alone: anira derives latency from the HostConfig, so it
// falls as the host block grows. The model itself adds no whole-hop delay (internal_model_latency
// is 0, see ScycloneModelConfig.cpp), so this is anira's buffering only. Measured with
// test/scyclone/calibration/OnnxLatencyProbeTest.cpp (FunkDrum and Djembe report identical
// values):
//
//   48 kHz, block   32 -> 4064      48 kHz, block  512 -> 3584  <-- the value below
//   48 kHz, block   64 -> 4032      48 kHz, block 1024 -> 3072
//   48 kHz, block  128 -> 3968      48 kHz, block 2048 -> 2048
//   48 kHz, block  256 -> 3840
//
// Defined at 48 kHz / 512-sample blocks, the reference host configuration. Production code
// reads the live value from the backend; this constant exists only for the sanitizer stub and
// the resampling contract tests, which need a fixed, block-size-independent latency model.
// OnnxProcessorContractTest.ReportedLatency_MatchesCalibratedConstantAtReferenceConfig pins it
// against the real backend, and AniraWetAlignmentTest checks the real wet signal lines up with it.
//
// Re-run both probes (OnnxLatencyProbe, OnnxWetLagProbe) after changing ScycloneModelConfig,
// anira, or the ONNX Runtime version.
constexpr int kOnnxInferenceLatencySamples = 3584;

/// Host configuration kOnnxInferenceLatencySamples was measured at.
constexpr double kOnnxInferenceLatencyReferenceSampleRate = 48000.0;
constexpr int kOnnxInferenceLatencyReferenceBlockSize = 512;

#endif // SCYCLONE_ONNXINFERENCELATENCY_H

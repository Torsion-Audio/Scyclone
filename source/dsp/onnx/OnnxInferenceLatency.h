#ifndef SCYCLONE_ONNXINFERENCELATENCY_H
#define SCYCLONE_ONNXINFERENCELATENCY_H

// Raw ONNX-island latency in samples, as reported by anira's InferenceHandler::get_latency().
//
// This is NOT a property of the model alone: anira derives latency from the HostConfig, so it
// falls as the host block grows. Measured with
// test/scyclone/calibration/OnnxLatencyProbeTest.cpp (FunkDrum and Djembe report identical
// values):
//
//   48 kHz, block   32 -> 6112      48 kHz, block  512 -> 5632  <-- the value below
//   48 kHz, block   64 -> 6080      48 kHz, block 1024 -> 5120
//   48 kHz, block  128 -> 6016      48 kHz, block 2048 -> 4096
//   48 kHz, block  256 -> 5888
//
// Defined at 48 kHz / 512-sample blocks, the reference host configuration. Production code
// reads the live value from the backend; this constant exists only for the sanitizer stub and
// the resampling contract tests, which need a fixed, block-size-independent latency model.
// OnnxLatencyContractTest pins it against the real backend so drift fails the build.
//
// Re-run the probe after changing ScycloneModelConfig, anira, or the ONNX Runtime version.
constexpr int kOnnxInferenceLatencySamples = 5632;

/// Host configuration kOnnxInferenceLatencySamples was measured at.
constexpr double kOnnxInferenceLatencyReferenceSampleRate = 48000.0;
constexpr int kOnnxInferenceLatencyReferenceBlockSize = 512;

#endif // SCYCLONE_ONNXINFERENCELATENCY_H

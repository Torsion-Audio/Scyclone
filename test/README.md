# Scyclone test suite

Behavioral contracts for resampling, dry/wet alignment, and plugin smoke tests.

See [docs/resampling_architecture.md](../docs/resampling_architecture.md) for the production graph.

## Layout

```
test/
  support/
    TestInfrastructure.h          Compatibility shim (resampling_test aliases)
    HostConfigCatalog.h           Sample-rate/block axes, composable presets, gtest adapters
    audio/                        Domain-neutral audio test utilities (scyclone::test)
      JuceFixture.h               JuceAudioTest
      BlockStreaming.h            Pre-roll, stream-in-blocks, collectProcessorOutput
      LongRun.h                   longRunTolerance, runLongBlockLoop
      SignalGenerators.h          Swept sine, Hanning windowed tones
      SignalMetrics.h             rms, impulse peak, maxOutsideWindow
      SignalMetrics.cpp           FFT peak SNR (calculateSnrDb)
    processors/                   Injectable IProcessor mocks (scyclone::test)
      PassthroughProcessor.h      Zero-latency middle stage
      DelayLineProcessor.h        Configurable FIFO delay @ processing rate
    mixer/                        Dry/wet contracts (scyclone::test::mixer)
      DryWetMeasurements.h        measureDryWetDiracPeak, measureDryWetDiracJitter
      DryWetAssertions.h          assertDryWetDiracAligned
    resampling/                   Resampler domain (scyclone::test::resampling)
      ResamplingTopology.h        Chain structs, prepare*, ONNX constants, feasibility
      ResamplingRunner.h          Unified chain block runners, collect* helpers
      ResamplingChainHelpers.h    Shim → Topology + Runner
      ResamplingFixtures.h        ChainContractTest, ProcessorStructuralTest, case builders
      ResamplingSignalUtils.h     SnrCase, defaultCiSnrCases()
      ResamplingMeasurements.h    measure* only (ImpulseResponse, RMS, SNR)
      ResamplingContractAssertions.h  assert* only (pure gtest contracts)
      SimulatedOnnxProcessor.h    ONNX-shaped DelayLineProcessor preset
  resampling/
    IsolationTest.cpp             Uncoupled up/down sizing (not production mode)
    StructuralTest.cpp            Per-processor frame invariants, prepare/release
    SignalTest.cpp                FFT peak SNR on windowed sines
    ChainContractTest.cpp         Round-trip + production chain contracts (ChainKind param)
  mixer/
    DryWetAlignmentTest.cpp       Dry/wet dirac peak alignment
    GrainDryWetContractTest.cpp   Grain dry-buffer aliasing contract
  plugin/
    PluginIntegrationTest.cpp     Full graph prepare/processBlock smoke
  calibration/
    ResamplingProbeTest.cpp       DISABLED tolerance / SNR / latency probes
  benchmark/
    benchmark.cpp                 Separate Benchmark target (not in ctest)
```

## Namespaces

| Namespace | Contents |
|-----------|----------|
| `scyclone::test` | JuceAudioTest, signal generators/metrics, block streaming, processor mocks |
| `scyclone::test::resampling` | ONNX-path topology, chain runners, resampler measurements/assertions |
| `scyclone::test::mixer` | Dry/wet dirac alignment measure/assert |
| `resampling_test` | **Compatibility alias** — `using` re-exports the above; existing `.cpp` files keep `using namespace resampling_test` |

## Test layers

Inspired by [libsamplerate tests](../modules/libsamplerate/tests/):

| Layer | Files | What we check |
|-------|-------|---------------|
| **Structural** | `resampling/StructuralTest.cpp` | Per-block frame invariants (up + down), warmup partial-block tail zeroed, long-run frame counts, prepare/release stability |
| **Signal quality** | `resampling/SignalTest.cpp` | FFT peak SNR (dB) on windowed sines — up-only and down-only |
| **Contracts** | `resampling/ChainContractTest.cpp` | Block size, RMS, impulse — round-trip (all rates) + production (48 kHz impulse/RMS only) |
| **Alignment** | `mixer/DryWetAlignmentTest.cpp` | Dry/wet dirac peak at `diracPos + totalLatency` |
| **Plugin** | `plugin/PluginIntegrationTest.cpp` | Full graph prepare/processBlock smoke |

## Core contracts

| Contract | What we check | Why |
|----------|---------------|-----|
| **Block size** | Host block in == host block out | Fixed block graph contract |
| **Signal fidelity** | FFT SNR on windowed sines (up/down); swept-sine RMS round-trip + production (48 kHz only) | Resampler quality + latency alignment |
| **Dry/wet dirac** | Wet peak in mix at `diracPos + totalLatency` | User-audible time alignment |
| **Impulse side lobes** | No secondary peak above 15% of main peak outside main window (round-trip + 48 kHz production) | Duplicate/truncated block content |
| **Warmup partial tail** | Samples `[framesGen..outSize)` zero when SRC underfills | Stale buffer must not reach ONNX path |

## Host config presets

| Preset | Configs | Used by |
|--------|---------|---------|
| `defaultCiHostConfigs()` | 44.1/48 kHz × 128/512 | Chain contracts, structural, round-trip long-run |
| `alignmentEdgeHostConfigs()` | 32, 64, 2048, 8192 (44.1), 2048 (48) | Composed into dry/wet matrix |
| `dryWetHostConfigs()` | default CI + alignment edge blocks | Dry/wet dirac alignment (block-center sensitive) |
| `extendedHostMatrixConfigs()` | 14 curated host/block pairs | `ExtendedHostMatrix` suite only |

Axes and combinators (`cartesianHostConfigs`, `mergeHostConfigs`, `dedupeHostConfigs`) live in `HostConfigCatalog.h`.

## Includes

Prefer **targeted includes** — avoid pulling the full resampling stack when a test needs one contract.

```cpp
// Resampling contract tests
#include "ResamplingFixtures.h"
#include "ResamplingContractAssertions.h"

// Resampling signal quality (SNR)
#include "ResamplingFixtures.h"
#include "ResamplingMeasurements.h"

// Dry/wet alignment only (no FFT SNR)
#include "DryWetAssertions.h"
#include "HostConfigCatalog.h"
#include "ResamplingFixtures.h"
#include "TestInfrastructure.h"

// Plugin / grain smoke
#include "TestInfrastructure.h"

// Calibration probes
#include "DryWetAssertions.h"
#include "DryWetMeasurements.h"
#include "HostConfigCatalog.h"
#include "PassthroughProcessor.h"
#include "ResamplingMeasurements.h"
#include "TestInfrastructure.h"
```

Tolerance constants: RMS/impulse in `ResamplingContractAssertions.h`, dirac in `DryWetAssertions.h`, SNR floors in `ResamplingSignalUtils.h`. Measurements live in `ResamplingMeasurements.h` and `DryWetMeasurements.h`; tests call `measure*` then `assert*` for debuggable failures.

### Injectable pipeline layers

| Layer | Middle mock | What it validates |
|-------|-------------|-------------------|
| Unit | — | `ResamplingProcessor` structural (partial tail, steady IO, SNR) |
| Chain | `PassthroughProcessor` | SRC block contract, round-trip impulse |
| Chain | `SimulatedOnnxProcessor` | Full latency formula @ 48 kHz |
| Mixer | `SimulatedOnnxProcessor` + `DryWetMixer` | User-facing mix alignment (default CI rates) |

Resampler regressions should fail at **Passthrough** middle, not require ONNX-shaped latency.

## Migration notes

| Old | New |
|-----|-----|
| `ResamplingChainHelpers.h` (monolith) | `ResamplingTopology.h` + `ResamplingRunner.h` |
| Signal math in `ResamplingSignalUtils.h` | `audio/SignalGenerators.h`, `audio/SignalMetrics.h` |
| `measureDryWetDiracPeak` in resampling headers | `mixer/DryWetMeasurements.h` |
| `assertDryWetDiracAligned` in resampling headers | `mixer/DryWetAssertions.h` |
| `namespace resampling_test` for everything | Layered `scyclone::test::*` + `resampling_test` alias shim |

`TestInfrastructure.h` is now a thin compatibility header. Prefer `JuceFixture.h` for new non-resampling tests.

## Calibration policy

**Cross-rate production chain:** swept-sine RMS and impulse peak are **`GTEST_SKIP`** in default CI (host rate ≠ 48 kHz). Round-trip impulse and dry/wet dirac still run at all default CI rates.

**Open product issue:** bulk `productionChainTotalLatency` ≠ measured group delay at cross-rate (~90 samples @ 44.1 kHz / 128). See [docs/resampling_architecture.md](../docs/resampling_architecture.md). Do not widen CI tolerance to hide this — use `DISABLED_LatencyAudit` / `DISABLED_ProductionSineLagSweep` probes.

See [calibration/README.md](calibration/README.md) for when to run DISABLED probes.

```powershell
# RMS / dirac tolerances
.\build\Test.exe --gtest_filter=*PrintToleranceMeasurements* --gtest_also_run_disabled_tests

# SNR floors (update defaultCiSnrCases() — measured − 3 dB)
.\build\Test.exe --gtest_filter=*PrintSnrMeasurements* --gtest_also_run_disabled_tests
```

**Re-run and update tolerances** if the resampler library, `SimulatedOnnxProcessor` latency config, or `InferenceThread` model sizes change.

## CI policy

| Label | When | Purpose |
|-------|------|---------|
| `default` | Every PR / develop push | Excludes `ExtendedHostMatrix` suite |
| `extended-matrix` | Release tags (`v*`) and manual | Extended host/block matrix only |

```powershell
cmake --preset default
cmake --build --preset test

# Default (CI)
ctest --test-dir build -L default --output-on-failure

# Extended matrix (release / local)
ctest --test-dir build -L extended-matrix --output-on-failure
```

If `ctest -L` is unavailable, use `ctest --label-regex "default"`.

## CMake presets

Shared presets live in [`CMakePresets.json`](../CMakePresets.json). Machine-specific overrides (e.g. macOS arch) belong in a gitignored `CMakeUserPresets.json`.

| Configure preset | Build dir | Purpose |
|------------------|-----------|---------|
| `default` | `build/` | Debug — tests, `ctest`, clangd |
| `release` | `build-release/` | Release — VST3 / Standalone (required on Windows) |
| `asan-ubsan` | `build-asan-ubsan/` | Linux/macOS ASan + UBSan (`Test` only) |
| `asan` | `build-asan/` | Windows MSVC ASan (`Test` only) |
| `tsan` | `build-tsan/` | Linux/macOS ThreadSanitizer (`Test` only) |

```powershell
cmake --preset default
cmake --build --preset test

cmake --preset release
cmake --build --preset release

# Linux/macOS sanitizer (after configure)
cmake --preset asan-ubsan
cmake --build --preset asan-ubsan
ctest --test-dir build-asan-ubsan -L default --output-on-failure

# Windows sanitizer
cmake --preset asan
cmake --build --preset asan
ctest --test-dir build-asan -L default --output-on-failure
```

MSan and LeakSanitizer are not preset-wired (extra toolchain setup). See maintainer doc below.

## Sanitizer CI

AddressSanitizer, UndefinedBehaviorSanitizer, and ThreadSanitizer run on every push/PR to `develop` via [`.github/workflows/sanitizers.yml`](../.github/workflows/sanitizers.yml). Maintainer details: [docs/maintainer/TESTING_SANITIZERS.md](../docs/maintainer/TESTING_SANITIZERS.md).

## IDE / IntelliSense

After clone, run `cmake --preset default` and `cmake --build --preset test`. Squiggles on test includes before configure are normal. With clangd, [`.clangd`](../.clangd) picks up `build/compile_commands.json` automatically. If you use the Microsoft C/C++ extension instead, set `C_Cpp.default.compileCommands` locally to `build/compile_commands.json` (optional, IntelliSense-only — ctest is the source of truth).

CMake include roots for the `Test` target: `test/support`, `test/support/audio`, `test/support/mixer`, `test/support/processors`, `test/support/resampling`.

## Targets

- **Test** — gtest behavioral contracts (this document)
- **Benchmark** — construction benchmarks; not run by `ctest`

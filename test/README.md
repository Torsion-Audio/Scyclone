# Scyclone test suite

Behavioral contracts for resampling, dry/wet alignment, and plugin smoke tests.

See [docs/resampling_architecture.md](../docs/resampling_architecture.md) for the production graph.

## Layout

```
test/
  torsion/                              # torsion::test — reusable across Torsion plugins
    audio/
      JuceFixture.h                     JuceAudioTest
      BlockStreaming.h                  Pre-roll, stream-in-blocks, collectProcessorOutput
      LongRun.h                         longRunTolerance, runLongBlockLoop
      TestTiming.h                      kPreRollBlocks, kSteadyBlocks, latencyPreRollBlocks
      SignalGenerators.h                Swept sine, Hanning windowed tones
      SignalMetrics.h / .cpp            rms, impulse peak, FFT SNR
      SignalFidelity.h                  sineWarmupBlocks, worstSweptSineRmsError
      ImpulseMetrics.h                  ImpulseResponse, fillImpulsePeakMetrics
    processors/
      PassthroughProcessor.h            Zero-latency middle stage
      DelayLineProcessor.h              Configurable FIFO delay @ processing rate
    gtest/
      HostConfig.h                      HostConfig struct
      HostConfigCatalog.h               cartesian, merge, dedupe, filter, param naming
      HostConfigFixtures.h              HostConfigParamTest base
      ImpulseAssertions.h               assertImpulse* (peak tolerance, side lobes)
  scyclone/                             # scyclone::test::* — Scyclone-specific
    resampling/
      ScycloneHostPresets.h             defaultCiHostConfigs, extended matrix, dry/wet presets
      ResamplingTopology.h              Chain structs, prepare*, ONNX constants, feasibility
      ResamplingRunner.h                Unified chain block runners, collect* helpers
      ResamplingChainDriver.h            Steady-state chain drivers (block loops — single SoT)
      ResamplingFixtures.h              Chain contract + structural fixtures, case builders
      ResamplingSignalUtils.h           SnrCase, defaultCiSnrCases()
      ResamplingMeasurements.h          Chain-specific measure* (RMS, SNR, impulse)
      ResamplingContractAssertions.h    Resampler assert* (block size, RMS, processor I/O)
      SimulatedOnnxProcessor.h          ONNX-shaped DelayLineProcessor preset
      IsolationTest.cpp                 Uncoupled up/down sizing (not production mode)
      StructuralTest.cpp                Per-processor frame invariants, prepare/release
      SignalTest.cpp                    FFT peak SNR on windowed sines
      ChainContractTest.cpp             Round-trip + production chain contracts (split suites)
    mixer/
      DryWetContract.h                  Fixture, measure*, assertDryWetDiracAligned
      DryWetAlignmentTest.cpp           Dry/wet dirac peak alignment
      GrainDryWetContractTest.cpp       Grain dry-buffer aliasing contract
    plugin/
      PluginIntegrationTest.cpp         Full graph prepare/processBlock smoke
    calibration/
      ResamplingProbeTest.cpp           DISABLED tolerance / SNR / latency probes
      README.md
  benchmark/
    benchmark.cpp                       Separate Benchmark target (not in ctest)
```

## Namespaces

| Namespace | Contents |
|-----------|----------|
| `torsion::test` | JuceAudioTest, signal generators/metrics, block streaming, processor mocks, host-config matrices, impulse metrics/assertions |
| `scyclone::test::resampling` | ONNX-path topology, chain runners, Scyclone CI presets, resampler measurements/assertions |
| `scyclone::test::mixer` | Dry/wet dirac alignment measure/assert |

## Torsion vs Scyclone boundary

| Concern | Torsion | Scyclone |
|---------|---------|----------|
| Host SR/block struct, cartesian matrices | yes | CI presets only |
| Swept-sine RMS, impulse peak analysis | yes | chain runners that produce samples |
| ONNX rate, `ResamplingProcessor` chains | — | yes |
| SNR floors tuned for this resampler | — | yes |

## Test layers

Inspired by [libsamplerate tests](../modules/libsamplerate/tests/):

| Layer | Files | What we check |
|-------|-------|---------------|
| **Structural** | `scyclone/resampling/StructuralTest.cpp` | Per-block frame invariants (up + down), warmup partial-block tail zeroed, long-run frame counts, prepare/release stability |
| **Signal quality** | `scyclone/resampling/SignalTest.cpp` | FFT peak SNR (dB) on windowed sines — up-only and down-only |
| **Contracts** | `scyclone/resampling/ChainContractTest.cpp` | Block size, RMS, impulse — `RoundTripChainContractTest`, `ProductionChainContractTest`, `Production48kSignalContractTest` |
| **Alignment** | `scyclone/mixer/DryWetAlignmentTest.cpp` | Dry/wet dirac peak at `diracPos + totalLatency` |
| **Plugin** | `scyclone/plugin/PluginIntegrationTest.cpp` | Full graph prepare/processBlock smoke |

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
| `productionSignalContractConfigs()` | 48 kHz subset of default CI | Production swept-sine + impulse only |
| `alignmentEdgeHostConfigs()` | 32, 64, 2048, 8192 (44.1), 2048 (48) | Composed into dry/wet matrix |
| `dryWetHostConfigs()` | default CI + alignment edge blocks | Dry/wet dirac alignment (block-center sensitive) |
| `extendedHostMatrixConfigs()` | 14 curated host/block pairs | `ExtendedHostMatrix` suite only |

Combinators (`cartesianHostConfigs`, `mergeHostConfigs`, `dedupeHostConfigs`, `filterHostConfigs`) live in `torsion/gtest/HostConfigCatalog.h`. Scyclone CI presets live in `scyclone/resampling/ScycloneHostPresets.h`.

## Includes

Prefer **targeted includes** — avoid pulling the full resampling stack when a test needs one contract.

```cpp
// Resampling contract tests
#include "ResamplingFixtures.h"
#include "ResamplingContractAssertions.h"

// Resampling signal quality (SNR)
#include "ResamplingMeasurements.h"
#include "ResamplingSignalUtils.h"

// Dry/wet alignment only (no FFT SNR)
#include "DryWetContract.h"
#include "ScycloneHostPresets.h"
#include "ResamplingTopology.h"

// Plugin smoke
#include "JuceFixture.h"
#include "TestTiming.h"

// Calibration probes
#include "DryWetContract.h"
#include "ImpulseMetrics.h"
#include "PassthroughProcessor.h"
#include "ResamplingMeasurements.h"
#include "ScycloneHostPresets.h"
```

Tolerance constants: RMS in `ResamplingContractAssertions.h`, dirac in `DryWetContract.h`, impulse peak in `ImpulseAssertions.h`, SNR floors in `ResamplingSignalUtils.h`. Chain block loops live in `ResamplingChainDriver.h` + `ResamplingRunner.h`; assert/measure headers delegate to them.

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
| `test/support/resampling/ResamplingChainHelpers.h` | `scyclone/resampling/ResamplingTopology.h` + `ResamplingRunner.h` |
| Signal math in `ResamplingSignalUtils.h` / `ResamplingMeasurements.h` | `torsion/audio/SignalGenerators.h`, `SignalMetrics.h`, `SignalFidelity.h`, `ImpulseMetrics.h` |
| Host-config combinators + CI presets in `HostConfigCatalog.h` | `torsion/gtest/HostConfigCatalog.h` + `scyclone/resampling/ScycloneHostPresets.h` |
| `measureDryWetDiracPeak` in resampling headers | `scyclone/mixer/DryWetContract.h` |
| `assertDryWetDiracAligned` in resampling headers | `scyclone/mixer/DryWetContract.h` |
| `namespace resampling_test` alias shim | `torsion::test` + `scyclone::test::*` (shim removed) |
| `TestInfrastructure.h` umbrella include | Targeted includes from `torsion/` and `scyclone/` headers |

Prefer `JuceFixture.h` for new non-resampling tests. Test cases live under `scyclone/{resampling,mixer,plugin,calibration}/`.

## Calibration policy

**Cross-rate production chain:** production swept-sine and impulse use `productionSignalContractConfigs()` (48 kHz only) — not runtime `GTEST_SKIP`. Round-trip impulse and dry/wet dirac still run at all default CI rates.

**Open product issue:** bulk `productionChainTotalLatency` ≠ measured group delay at cross-rate (~90 samples @ 44.1 kHz / 128). See [docs/resampling_architecture.md](../docs/resampling_architecture.md). Do not widen CI tolerance to hide this — use `DISABLED_LatencyAudit` / `DISABLED_ProductionSineLagSweep` probes.

See [scyclone/calibration/README.md](scyclone/calibration/README.md) for when to run DISABLED probes.

```powershell
# RMS / dirac tolerances
.\build\Test.exe --gtest_filter=*PrintToleranceMeasurements* --gtest_also_run_disabled_tests

# SNR floors (update defaultCiSnrCases() per OS — measured − 3 dB from PrintSnrMeasurements)
.\build\Test.exe --gtest_filter=*PrintSnrMeasurements* --gtest_also_run_disabled_tests
```

**Re-run and update tolerances** if the resampler library, `SimulatedOnnxProcessor` latency config, or `InferenceThread` model sizes change.

## CI policy

| Label | When | Purpose |
|-------|------|---------|
| `default` | Every PR / develop push | Excludes `ExtendedHostMatrix` suite |
| `extended-matrix` | Release tags (`v*`) and manual | Extended host/block matrix only |

Sanitizer CI builds (`SCYCLONE_SANITIZERS` preset) skip the Google Benchmark dependency; see [docs/maintainer/TESTING_SANITIZERS.md](../docs/maintainer/TESTING_SANITIZERS.md).

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

Shared presets live in [`CMakePresets.json`](../CMakePresets.json). Machine-specific overrides belong in a gitignored `CMakeUserPresets.json` at the repo root (e.g. macOS `CMAKE_OSX_ARCHITECTURES`).

| Configure preset | Build dir | Purpose |
|------------------|-----------|---------|
| `default` | `build/` | Debug — tests, `ctest`, clangd |
| `release` | `build-release/` | Release — VST3 / Standalone plugin |
| `asan-ubsan` | `build-asan-ubsan/` | Linux/macOS ASan + UBSan (`Test` only) |
| `asan` | `build-asan/` | Windows MSVC ASan (`Test` only) |
| `tsan` | `build-tsan/` | Linux/macOS ThreadSanitizer (`Test` only) |

### Windows (MSVC)

See [docs/maintainer/windows-build.md](../docs/maintainer/windows-build.md) for MSVC toolset requirements. Run [`cmake/windows/configure.ps1`](../cmake/windows/configure.ps1) once, then use the shared presets below.

For **tests and Debug/IDE** work (not the shipping plugin build):

```powershell
.\cmake\windows\configure.ps1
cmake --build --preset test
ctest --test-dir build -L default --output-on-failure

# Windows sanitizer (advisory in CI)
.\cmake\windows\configure.ps1 -Preset asan
cmake --build --preset asan
ctest --test-dir build-asan -L default --output-on-failure
```

For **Release plugin** builds:

```powershell
.\cmake\windows\configure.ps1 -Preset release
cmake --build --preset release
```

```powershell
# Linux/macOS sanitizer (matches CI — use runtime env vars on Linux for leak detection)
cmake --preset asan-ubsan
cmake --build --preset asan-ubsan
# Linux only:
#   export ASAN_OPTIONS=detect_leaks=1:abort_on_error=1
#   export UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1
# macOS Apple Clang: omit detect_leaks from ASAN_OPTIONS
ctest --test-dir build-asan-ubsan -L default --output-on-failure
```

MSan and the macOS Homebrew leak probe are not in CMakePresets.json — see [docs/maintainer/TESTING_SANITIZERS.md](../docs/maintainer/TESTING_SANITIZERS.md).

## Sanitizer CI

Required gates (ASan+UBSan Linux/macOS, TSan Linux/macOS) run on every PR/push to `develop` and on `v*` tags via [`.github/workflows/sanitizers.yml`](../.github/workflows/sanitizers.yml) — require job **`sanitizers-required`** in branch protection.

Advisory jobs (Windows ASan, Linux MSan, macOS leak probe) run in [`.github/workflows/sanitizers-advisory.yml`](../.github/workflows/sanitizers-advisory.yml) and do not block merge.

Also see: [docs/maintainer/TESTING_SANITIZERS.md](../docs/maintainer/TESTING_SANITIZERS.md).

## IDE / IntelliSense

After clone, run `cmake --preset default` and `cmake --build --preset test` (on Windows, run [`cmake/windows/configure.ps1`](../cmake/windows/configure.ps1) first). Squiggles on test includes before configure are normal. With clangd, [`.clangd`](../.clangd) picks up `build/compile_commands.json` automatically. If you use the Microsoft C/C++ extension instead, set `C_Cpp.default.compileCommands` locally to `build/compile_commands.json` (optional, IntelliSense-only — ctest is the source of truth).

CMake include roots for the `Test` target: `test/torsion`, `test/torsion/audio`, `test/torsion/processors`, `test/torsion/gtest`, `test/scyclone/resampling`, `test/scyclone/mixer`, plus plugin source includes from the main target.

## Targets

- **Test** — gtest behavioral contracts (this document)
- **Benchmark** — construction benchmarks; not run by `ctest`

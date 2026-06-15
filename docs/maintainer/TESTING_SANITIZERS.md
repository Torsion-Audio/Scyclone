# Sanitizer CI — maintainers

Policy and wiring for sanitizer builds. **Build & Test workflow:** [`.github/workflows/build-and-test.yml`](../../.github/workflows/build-and-test.yml). **Source of truth:** [`cmake/ScycloneSanitizers.cmake`](../../cmake/ScycloneSanitizers.cmake) (presets, flags, FATAL_ERROR rules), [`.github/workflows/sanitizers.yml`](../../.github/workflows/sanitizers.yml) (required jobs), [`.github/workflows/sanitizers-advisory.yml`](../../.github/workflows/sanitizers-advisory.yml) (advisory jobs).

Local configure/build: [test/README.md](../../test/README.md) and [`CMakePresets.json`](../../CMakePresets.json). Required CI jobs use CMake presets (`asan-ubsan`, `tsan`, `asan`). Only **`Test`** is built/run — not plugin formats.

## CI jobs

### Required ([`sanitizers.yml`](../../.github/workflows/sanitizers.yml))

Branch protection: require job **`sanitizers-required`** (aggregates the four jobs below).

| Job | Preset | Gate |
|-----|--------|------|
| `ASan+UBSan (Linux)` | `asan-ubsan` + `detect_leaks=1` | **required** (authoritative leak gate) |
| `ASan+UBSan (macOS)` | `asan-ubsan` | **required** |
| `TSan (Linux)` / `TSan (macOS)` | `tsan` | **required** |

### Advisory ([`sanitizers-advisory.yml`](../../.github/workflows/sanitizers-advisory.yml))

All jobs use `continue-on-error: true` — **do not block merge**.

| Job | Preset | Gate |
|-----|--------|------|
| `[advisory] ASan (Windows MSVC)` | `asan` | advisory |
| `[advisory] MSan (Linux)` | `MEMORY` (raw cmake, `build-msan`) | advisory |
| `[advisory] ASan leaks probe (macOS)` | `ASAN_UBSAN` + Homebrew Clang + `detect_leaks=1` | advisory (experimental probe) |

All jobs: `ctest -L default` (see [test/README.md](../../test/README.md) for labels).

## Advisory → required promotion criteria

Before moving an advisory job into `sanitizers.yml` and requiring it in branch protection:

| Job | Promote when |
|-----|----------------|
| Windows MSVC ASan | ≥ 14 consecutive green runs; no flaky ONNX-stub skips masking regressions |
| Linux MSan | Stable libc++ cache; tests pass without false positives from distro libs |
| macOS leak probe | Clean exit without `LeakSanitizer: CHECK failed` / `Subprocess aborted` ([llvm#117476](https://github.com/llvm/llvm-project/issues/117476)); suppressions tuned for Scyclone-only leaks |

## Leak detection policy

**Automated (CI):** Linux `ASan+UBSan (Linux)` runs with `ASAN_OPTIONS=detect_leaks=1:abort_on_error=1`. This is the **required** leak gate.

**macOS Apple Clang (`ASan+UBSan (macOS)`):** do **not** set `detect_leaks` — Apple Clang aborts with `detect_leaks is not supported on this platform`.

**macOS experimental probe (`[advisory] ASan leaks probe (macOS)`):** Homebrew **`llvm@18`**, `ASAN_UBSAN`, ONNX stub (automatic for open-source `Clang` on macOS — prebuilt ORT does not link), `ASAN_OPTIONS=detect_leaks=1:abort_on_error=0`, `UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=0`, suppressions in [`test/sanitizer/lsan-macos.supp`](../../test/sanitizer/lsan-macos.supp). Non-blocking. Standalone `-fsanitize=leak` was retired — it crashed at tear-down with JUCE GUI init ([llvm#117476](https://github.com/llvm/llvm-project/issues/117476)).

**Manual macOS (before release / lifetime changes):**

1. Build `Test` locally (Debug or RelWithDebInfo).
2. Run under Xcode **Instruments → Leaks**, or:
   ```bash
   leaks --atExit -- ./build/Test --gtest_filter=-*ExtendedHostMatrix*
   ```
3. Run when touching processors, ONNX thread lifetime, or JUCE-owned UI state.

## Decisions not obvious from CMake alone

**MSan requires full link instrumentation** — every object in the link must be MSan-instrumented (including FetchContent **gtest**; see [`setup_tests_and_benchmarks.cmake`](../../cmake/setup_tests_and_benchmarks.cmake)). This does **not** apply to standalone LSan (`LEAK` preset) or to macOS `detect_leaks` via ASan.

**MSan + ONNX stub** — prebuilt ORT is not MSan-instrumented. `MEMORY` forces stub ([`InferenceThreadStub.cpp`](../../source/dsp/onnx/InferenceThreadStub.cpp)); `PluginIntegrationTest` skips under `SCYCLONE_ONNX_STUB`.

**Linux ASAN_UBSAN + PluginIntegrationTest** — prebuilt ORT + Linux UBSan hits invalid-vptr in ORT during model load; `PluginIntegrationTest` skips (`SCYCLONE_SKIP_PLUGIN_INTEGRATION_TEST`). macOS Apple-Clang ASan still runs it.

**Windows MSVC ASan** — `ASAN` forces the same ONNX stub (LNK2038/LNK1319 without it); DSP/resampling tests run, `PluginIntegrationTest` skips.

**Homebrew Clang jobs** — pin **`llvm@18`** (unpinned `llvm` 22 breaks JUCE 7.0.5). ONNX stub is **automatic** when `CMAKE_CXX_COMPILER_ID` is `Clang` (not `AppleClang`) on macOS with `ASAN` or `ASAN_UBSAN` — prebuilt ORT does not link with Homebrew Clang.

**SNR floors** — Linux/macOS ASan jobs run `PrintSnrMeasurements` (`continue-on-error`) to calibrate per-OS floors in [`ResamplingSignalUtils.h`](../../test/scyclone/resampling/ResamplingSignalUtils.h). Procedure: [test/scyclone/calibration/README.md](../../test/scyclone/calibration/README.md).

## macOS leak probe — evaluate after first CI run

| Outcome | Action |
|---------|--------|
| Tests pass, clean exit, optional leak summary | Keep probe as warn-only; tune suppressions for Scyclone-only leaks |
| `LeakSanitizer: CHECK failed` or `Subprocess aborted` at exit | Remove `[advisory] ASan leaks probe (macOS)`; rely on Linux gate + manual macOS runbook |
| Tests pass but many `<unknown module>` leaks | Keep advisory; informational only — do not gate merges |

Check artifact `sanitize-asan-leaks-macos-log` on probe failure.

## When CI breaks

| Symptom | Check |
|---------|--------|
| Linux configure / `juceaide` | [`.github/actions/setup-linux-juce`](../../.github/actions/setup-linux-juce/action.yml) ran |
| macOS Apple-Clang ASan aborts before tests | `detect_leaks` must not be set on Apple Clang jobs |
| macOS leak probe: tests pass then `Subprocess aborted` | Known LSan + JUCE GUI/AppKit issue ([llvm#117476](https://github.com/llvm/llvm-project/issues/117476)); remove probe per table above |
| No sanitizer in stack traces | `Test` links `scyclone_sanitizer_flags`; `-fsanitize=` in `compile_commands.json` |
| Windows tests fail after link | MSVC ASan DLL on `PATH` (CI: [`.github/actions/setup-msvc`](../../.github/actions/setup-msvc/action.yml) with `export_asan_path`) |

# CI benchmark performance tracking

macOS-only Google Benchmark job with charts and step summaries via [github-action-benchmark](https://github.com/benchmark-action/github-action-benchmark).

## Workflow

[`.github/workflows/benchmark.yml`](../../.github/workflows/benchmark.yml) runs on:

- `push` to `develop` — updates `gh-pages` charts + step summary
- `pull_request` targeting `develop` — step summary only (no `gh-pages` push)

Composite actions: [`setup-macos-juce`](../../.github/actions/setup-macos-juce/action.yml), [`set-parallel-build-level`](../../.github/actions/set-parallel-build-level/action.yml).

Build: Release Ninja, native arm64, `cmake --build build --target Benchmark` (`SCYCLONE_SANITIZERS=NONE`, the default).

## Metrics

| Stage | Statistic | Source |
|-------|-----------|--------|
| Per-run value | Mean of 5 repetitions | Google Benchmark JSON aggregate (`cpu_time`, nanoseconds) |
| Compile time | Wall-clock seconds (one sample) | `cmake --build build --target Benchmark` in CI, appended to the same JSON |
| Cross-run history | All prior `develop` pushes | `dev/bench/data.js` on `gh-pages` (maintained by the action) |

CLI flags: `--benchmark_repetitions=5`, `--benchmark_min_time=2.0s`, aggregates only, random interleaving off.

## Benchmarks

Host-style configuration: **44.1 kHz**, **512 samples**, stereo. Expensive setup (processor construction, ONNX model load) runs in Google Benchmark **fixtures**, not in the timed loop.

| Name | Role |
|------|------|
| `BM_processor_prepare` | `prepareToPlay` + `releaseResources` per iteration (processor reused) |
| `BM_process_block` | `processBlock` on a prepared processor (512-sample stereo buffer) |
| `BM_editor` | Editor create/destroy per iteration (processor reused) |
| `BM_compile_benchmark_target` | CI wall-clock Release build of the Benchmark target (not a Google Benchmark case) |
| `BM_reference_cpu` | FMA loop — CPU / scheduler noise sentinel |
| `BM_reference_memory` | ~1 MiB fill + strided arithmetic — memory bandwidth sentinel |

Defined in [`test/benchmark/benchmark.cpp`](../../test/benchmark/benchmark.cpp) with `MinTime(2.0)`. Compile time is measured in the workflow build step and merged into `benchmark_result.json` with `jq` before upload.

## Dashboard (gh-pages)

| Item | Location |
|------|----------|
| Time series data | `gh-pages` → `dev/bench/data.js` (updated by CI) |
| Layout / CSS / fonts / lockup | [`.github/benchmark-dashboard/`](../../.github/benchmark-dashboard/) (`index.html`, `favicon.svg`, `fonts/`, `torsion-audio-lockup.svg`; copied to `gh-pages` once; **not** overwritten by the action) |
| Public URL | [torsion-audio.github.io/Scyclone/dev/bench/](https://torsion-audio.github.io/Scyclone/dev/bench/) |

**Custom styling:** edit the `<style>` block in `.github/benchmark-dashboard/index.html`. The action only auto-generates `index.html` when it is missing; after you seed the Scyclone layout, CI leaves it alone and updates `data.js` only.

Setup and local preview: [`.github/benchmark-dashboard/README.md`](../../.github/benchmark-dashboard/README.md).

## PR / CI feedback

- **Step summary** on every run (`summary-always: true`) — comparison vs previous `gh-pages` entry when available, plus a [dashboard link](https://torsion-audio.github.io/Scyclone/dev/bench/)
- **Artifacts:** `benchmark-results-macos-<sha>` (90 days)
- Built-in PR comments and `fail-on-alert` are **off** (advisory job)

To enable regression alerts later, set `fail-on-alert: true` and/or `comment-on-alert: true` on the benchmark-action step.

## gh-pages setup (one-time)

1. Enable GitHub Pages from the `gh-pages` branch (repo **Settings → Pages**).
2. Copy [`.github/benchmark-dashboard/index.html`](../../.github/benchmark-dashboard/index.html), [`favicon.svg`](../../.github/benchmark-dashboard/favicon.svg), [`fonts/`](../../.github/benchmark-dashboard/fonts/), and [`torsion-audio-lockup.svg`](../../.github/benchmark-dashboard/torsion-audio-lockup.svg) to `gh-pages:dev/bench/` before the first CI run if you want the Scyclone layout instead of the action default. See the [dashboard README](../../.github/benchmark-dashboard/README.md).

## Local benchmark run

```bash
cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target Benchmark
./build/Benchmark \
  --benchmark_format=json \
  --benchmark_out=benchmark_result.json \
  --benchmark_out_format=json \
  --benchmark_repetitions=5 \
  --benchmark_min_time=2.0s \
  --benchmark_report_aggregates_only=true \
  --benchmark_display_aggregates_only=true \
  --benchmark_enable_random_interleaving=false
```

## Branch protection

The Benchmark job is **advisory** by default. Add `benchmark (macOS)` to required checks only when you are happy with baseline noise on `develop`.

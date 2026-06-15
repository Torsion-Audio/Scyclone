# Benchmark dashboard (local preview & gh-pages layout)

CI-owned static layout for the public benchmark charts (lives next to [`.github/workflows/benchmark.yml`](../workflows/benchmark.yml)). CI updates **`data.js` only** on the `gh-pages` branch; this `index.html` is **not** overwritten once present.

Maintainer overview: [`docs/maintainer/benchmarks.md`](../../docs/maintainer/benchmarks.md).

## Preview locally

```powershell
cd .github/benchmark-dashboard
Copy-Item data.example.js data.js
python -m http.server 8080
```

Open `http://localhost:8080/`. Edit `index.html` (inline `<style>`) and refresh.

## One-time gh-pages setup (custom layout)

Before or after the first benchmark workflow run on `develop`:

```powershell
git fetch origin gh-pages 2>$null
git checkout -B gh-pages origin/gh-pages 2>$null
mkdir -Force dev/bench | Out-Null
mkdir -Force dev/bench/fonts | Out-Null
Copy-Item .github/benchmark-dashboard/index.html dev/bench/
Copy-Item .github/benchmark-dashboard/torsion-audio-lockup.svg dev/bench/
Copy-Item .github/benchmark-dashboard/favicon.svg dev/bench/
Copy-Item .github/benchmark-dashboard/fonts/*.ttf dev/bench/fonts/
git add dev/bench/index.html dev/bench/favicon.svg dev/bench/torsion-audio-lockup.svg dev/bench/fonts/
git commit -m "Add Scyclone benchmark dashboard layout"
git push -u origin gh-pages
git checkout develop
```

If `gh-pages` does not exist yet, create an orphan branch first, then copy `index.html` before the first CI push so the action never installs the generic default page.

## Updating styles later

1. Edit `.github/benchmark-dashboard/index.html` in this repo (CSS is in the `<style>` block).
2. Copy `index.html`, `favicon.svg`, `fonts/`, and `torsion-audio-lockup.svg` to `gh-pages:dev/bench/` and push (same paths as above).

Do **not** rename `data.js` or change `window.BENCHMARK_DATA` — [github-action-benchmark](https://github.com/benchmark-action/github-action-benchmark) owns that file.

# Benchmark dashboard (local preview & gh-pages layout)

CI-owned static layout for the public benchmark charts (lives next to [`.github/workflows/benchmark.yml`](../workflows/benchmark.yml)). CI updates **`data.js` only** on the `gh-pages` branch; layout files are **not** overwritten once present.

Important operational detail: the public dashboard URL will not work until `gh-pages` exists and has a seeded `dev/bench/` layout. Later layout edits in this repo still require a manual copy to `gh-pages:dev/bench/`.

Maintainer overview: [`docs/maintainer/benchmarks.md`](../../docs/maintainer/benchmarks.md).

## Layout

| Path | Role |
|------|------|
| `index.html` | Page shell (header, footer, script tags) |
| `styles.css` | All dashboard CSS |
| `app/*.js` | ES module app code (charts, KPIs, tooltips, render) |
| `data.js` | CI-owned benchmark time series (`window.BENCHMARK_DATA`) |
| `data.example.js` | Local preview fixture |
| `fonts/`, `favicon.svg`, `torsion-audio-lockup.svg` | Static assets |

## Preview locally

```powershell
cd .github/benchmark-dashboard
Copy-Item data.example.js data.js
python -m http.server 8080
```

Open `http://localhost:8080/`. ES modules require an HTTP server (do not open `index.html` via `file://`).

Edit `styles.css` for styling or `app/*.js` for behavior, then refresh.

If `data.js` is missing or malformed, the page now shows a friendly empty state instead of crashing, but it still cannot render charts until you copy the example fixture.

Optional pure-function smoke test (from repo root):

```powershell
node .github/benchmark-dashboard/app/verify-pure.mjs
```

## One-time gh-pages setup (custom layout)

Before or after the first benchmark workflow run on `develop`:

```powershell
git fetch origin gh-pages 2>$null
git checkout -B gh-pages origin/gh-pages 2>$null
mkdir -Force dev/bench | Out-Null
mkdir -Force dev/bench/fonts | Out-Null
mkdir -Force dev/bench/app | Out-Null
Copy-Item .github/benchmark-dashboard/index.html dev/bench/
Copy-Item .github/benchmark-dashboard/styles.css dev/bench/
Copy-Item .github/benchmark-dashboard/app/*.js dev/bench/app/
Copy-Item .github/benchmark-dashboard/torsion-audio-lockup.svg dev/bench/
Copy-Item .github/benchmark-dashboard/favicon.svg dev/bench/
Copy-Item .github/benchmark-dashboard/fonts/*.ttf dev/bench/fonts/
git add dev/bench/index.html dev/bench/styles.css dev/bench/app/ dev/bench/favicon.svg dev/bench/torsion-audio-lockup.svg dev/bench/fonts/
git commit -m "Add Scyclone benchmark dashboard layout"
git push -u origin gh-pages
git checkout develop
```

If `gh-pages` does not exist yet, create an orphan branch first, then copy layout files before the first CI push so the action never installs the generic default page.

## Updating layout later

1. Edit files under `.github/benchmark-dashboard/` in this repo (`styles.css`, `app/*.js`, `index.html`).
2. Copy `index.html`, `styles.css`, `app/`, `favicon.svg`, `fonts/`, and `torsion-audio-lockup.svg` to `gh-pages:dev/bench/` and push.
3. Verify the published page after the copy; the benchmark workflow updates `data.js` only and will not sync layout changes for you.

Do **not** rename `data.js` or change `window.BENCHMARK_DATA`. [github-action-benchmark](https://github.com/benchmark-action/github-action-benchmark) owns that file.

## Post-merge checklist

After merging dashboard layout changes to `develop`:

1. Copy the updated layout assets to `gh-pages:dev/bench/`.
2. Push the `gh-pages` update.
3. Open the public dashboard URL and confirm the new layout is live.

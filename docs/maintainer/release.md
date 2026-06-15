# Release process — maintainers only. 

> [!NOTE]
> The `develop` branch pipeline validates only (build + test). Pushes to `develop` do not produce zips.

## Publish an official release

Set `project(Scyclone VERSION X.Y.Z)` in [`CMakeLists.txt`](../../CMakeLists.txt) and merge to `develop`.

(Optional) Run pre-tag checks: `./.github/scripts/prepare-release.sh X.Y.Z` (`.ps1` on Windows).

Tag and push:

```bash
git tag vX.Y.Z && git push origin vX.Y.Z
```

CI then builds signed/notarized macOS (universal binary) and Windows zips and publishes them to [GitHub Releases](https://github.com/Torsion-Audio/Scyclone/releases) as `Scyclone-{macOS|Windows}-vX.Y.Z.zip`.

> [!NOTE]
> The tag `vX.Y.Z` must match the CMake `VERSION` field (without the `v`).

## Manual builds from Actions

Use this for test builds or sharing zips without cutting an official release.

**Signed zips only (no GitHub Release)**

1. Go to **Actions → [Build & Test](https://github.com/Torsion-Audio/Scyclone/actions/workflows/build-and-test.yml) → Run workflow**.
2. Choose branch (usually `develop`), `distribution_type` (`arm64` or `universal` on macOS).
3. Leave **create_release** off.
4. Download zips from that run’s **Artifacts** tab (kept ~90 days). Names look like `Scyclone-macOS-universal-manual.zip`.

**Publish a GitHub Release from Actions (no local `git tag`)**

Same workflow, but enable **create_release** and set **release_version** to `X.Y.Z` matching [`CMakeLists.txt`](../../CMakeLists.txt). CI creates tag `vX.Y.Z`, builds, and uploads to [Releases](https://github.com/Torsion-Audio/Scyclone/releases).

## Where things live


| What                  | Where                                                                                            |
| --------------------- | ------------------------------------------------------------------------------------------------ |
| Official release zips | [GitHub → Releases](https://github.com/Torsion-Audio/Scyclone/releases)                          |
| Manual / test zips    | Actions run → Artifacts                                                                          |
| Workflow definition   | `[.github/workflows/build-and-test.yml](../../.github/workflows/build-and-test.yml)` |
| Pre-tag checks        | `[.github/scripts/prepare-release.sh](../../.github/scripts/prepare-release.sh)`                 |



# CI overview — maintainers

## Workflows

| Workflow | Trigger | Purpose |
|----------|---------|---------|
| [Build & Test](../../.github/workflows/build-and-test.yml) | PR, `develop` push, `v*` tag, manual | Build + test on Linux/macOS/Windows; distribution zips on tags |
| [Sanitizers](../../.github/workflows/sanitizers.yml) | PR, `develop` push, `v*` tag, manual | **Required** ASan+UBSan and TSan gates |
| [Sanitizers (advisory)](../../.github/workflows/sanitizers-advisory.yml) | PR, `develop` push, `v*` tag, manual | Experimental sanitizer jobs — do not block merge |

## Branch protection (PRs to `develop`)

Require these check jobs:

- **Build & Test** — `ci-validation` (or the workflow’s aggregate job name on your branch)
- **Sanitizers** — `sanitizers-required` (aggregates four required sanitizer jobs)

Advisory sanitizer jobs run in parallel but use `continue-on-error` and must not be required.

## Shared composite actions

| Action | Used for |
|--------|----------|
| [setup-linux-juce](../../.github/actions/setup-linux-juce/action.yml) | apt build tools + JUCE system deps |
| [setup-macos-juce](../../.github/actions/setup-macos-juce/action.yml) | Homebrew ninja/osxutils |
| [setup-msvc](../../.github/actions/setup-msvc/action.yml) | vswhere + vcvars64 env export |
| [set-parallel-build-level](../../.github/actions/set-parallel-build-level/action.yml) | `CMAKE_BUILD_PARALLEL_LEVEL` from runner CPUs |
| [sanitizer-job-summary](../../.github/actions/sanitizer-job-summary/action.yml) | `GITHUB_STEP_SUMMARY` for sanitizer jobs |

Sanitizer policy and troubleshooting: [TESTING_SANITIZERS.md](TESTING_SANITIZERS.md). Release tags: [release.md](release.md).

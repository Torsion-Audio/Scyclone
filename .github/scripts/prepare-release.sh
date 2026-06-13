#!/usr/bin/env bash
set -euo pipefail

if [ $# -ne 1 ]; then
  echo "Usage: $0 X.Y.Z" >&2
  echo "Example: $0 0.0.4" >&2
  exit 1
fi

TARGET_VERSION="$1"
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
CMAKE_FILE="${ROOT}/CMakeLists.txt"

if ! [[ "$TARGET_VERSION" =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
  echo "Error: version must be semver X.Y.Z (got '$TARGET_VERSION')" >&2
  exit 1
fi

CMAKE_VERSION=$(grep -oE 'project\([[:space:]]*Scyclone[[:space:]]+VERSION[[:space:]]+[0-9.]+' "$CMAKE_FILE" | grep -oE '[0-9.]+$' || true)

if [ -z "$CMAKE_VERSION" ]; then
  echo "Error: could not read project(Scyclone VERSION ...) from CMakeLists.txt" >&2
  exit 1
fi

echo "CMake VERSION:  ${CMAKE_VERSION}"
echo "Target release: ${TARGET_VERSION}"
echo "Tag to push:    v${TARGET_VERSION}"
echo "Commit:         $(git -C "$ROOT" rev-parse --short HEAD)"
echo

if [ "$CMAKE_VERSION" != "$TARGET_VERSION" ]; then
  echo "Mismatch: update CMakeLists.txt to project(Scyclone VERSION ${TARGET_VERSION}) before tagging." >&2
  exit 1
fi

if ! git -C "$ROOT" diff --quiet || ! git -C "$ROOT" diff --cached --quiet; then
  echo "Error: working tree is not clean. Commit or stash changes before tagging." >&2
  exit 1
fi

if [ -n "$(git -C "$ROOT" tag -l "v${TARGET_VERSION}")" ]; then
  echo "Error: tag v${TARGET_VERSION} already exists locally." >&2
  exit 1
fi

if git -C "$ROOT" ls-remote --exit-code origin "refs/tags/v${TARGET_VERSION}" >/dev/null 2>&1; then
  echo "Error: tag v${TARGET_VERSION} already exists on origin." >&2
  exit 1
fi

CURRENT_BRANCH="$(git -C "$ROOT" branch --show-current)"
if [ "$CURRENT_BRANCH" != "develop" ]; then
  echo "Warning: current branch is '${CURRENT_BRANCH}', not 'develop'." >&2
fi

echo "Version check passed."
echo
echo "Next steps:"
echo "  git tag v${TARGET_VERSION}"
echo "  git push origin v${TARGET_VERSION}"
echo
echo "CI will validate, build signed zips, and publish a GitHub Release."

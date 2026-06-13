#!/usr/bin/env bash
set -euo pipefail

if [ $# -ne 1 ]; then
  echo "Usage: $0 X.Y.Z" >&2
  echo "Example: $0 0.0.4" >&2
  exit 1
fi

TARGET_VERSION="$1"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
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
echo

if [ "$CMAKE_VERSION" != "$TARGET_VERSION" ]; then
  echo "Mismatch: update CMakeLists.txt to project(Scyclone VERSION ${TARGET_VERSION}) before tagging." >&2
  exit 1
fi

echo "Version check passed."
echo
echo "Next steps:"
echo "  git tag v${TARGET_VERSION}"
echo "  git push origin v${TARGET_VERSION}"
echo
echo "CI will validate, build signed zips, and publish a GitHub Release."

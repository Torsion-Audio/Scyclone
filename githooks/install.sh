#!/bin/sh
# Point this repository at the version-controlled hooks in githooks/.
set -e
cd "$(git rev-parse --show-toplevel)"
git config core.hooksPath githooks
echo "Installed git hooks from: $(pwd)/githooks"

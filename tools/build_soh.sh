#!/usr/bin/env bash
# Copy the TrifoolPercent module into the Shipwright checkout and build Ship of Harkinian.
# Shipwright's CMake globs soh/soh/**, so no build files need editing. Run tools/setup.sh first.
#
# Building SoH also needs its usual setup (see third_party/Shipwright/docs/BUILDING.md); the game still
# extracts oot.o2r from your own ROM on first launch.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SOH="$ROOT/third_party/Shipwright"

rm -rf "$SOH/soh/soh/Enhancements/TrifoolPercent"
cp -r "$ROOT/soh/Enhancements/TrifoolPercent" "$SOH/soh/soh/Enhancements/TrifoolPercent"

cmake -S "$SOH" -B "$SOH/build" -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build "$SOH/build" -j"$(nproc)"

echo "Built $SOH/build/soh/soh.elf. Put build/TrifoolPercent.o2r in its mods folder."

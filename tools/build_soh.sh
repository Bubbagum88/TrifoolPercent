#!/usr/bin/env bash
# Copy the TrifoolPercent module into the Shipwright checkout and build Ship of Harkinian.
# Shipwright's CMake globs soh/soh/**, so no build files need editing. Run tools/setup.sh first.
#
# Building SoH also needs its usual setup (see third_party/Shipwright/docs/BUILDING.md); the game still
# extracts oot.o2r from your own ROM on first launch.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SOH="$ROOT/third_party/Shipwright"

# Reset the Shipwright checkout to the pinned commit before each build so engine
# patches re-apply cleanly (idempotent re-runs) and module edits don't leak.
# tools/setup.sh is responsible for the clone + checkout; here we just undo any
# previous build_soh.sh edits in tracked files.
git -C "$SOH" reset --hard HEAD >/dev/null

rm -rf "$SOH/soh/soh/Enhancements/TrifoolPercent"
cp -r "$ROOT/soh/Enhancements/TrifoolPercent" "$SOH/soh/soh/Enhancements/TrifoolPercent"

# Apply engine patches (if any) to the Shipwright checkout. A rejected hunk means
# SHIPWRIGHT_COMMIT in tools/setup.sh has drifted past the patch's anchor -- re-
# anchor the patch by hand and commit the fix. See patches/shipwright/README.md.
if compgen -G "$ROOT/patches/shipwright/*.patch" > /dev/null; then
    for p in "$ROOT"/patches/shipwright/*.patch; do
        echo "Applying $(basename "$p") to Shipwright..."
        patch -d "$SOH" -p1 < "$p" || {
            echo "FAILED to apply $(basename "$p"). Does SHIPWRIGHT_COMMIT in tools/setup.sh need to be bumped, or the patch re-anchored?" >&2
            exit 1
        }
    done
fi

cmake -S "$SOH" -B "$SOH/build" -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build "$SOH/build" -j"$(nproc)"

echo "Built $SOH/build/soh/soh.elf. Put build/TrifoolPercent.o2r in its mods folder."

#!/usr/bin/env bash
# Fetch everything the TrifoolPercent build needs, pinned to the versions it was tested with.
#
#   third_party/triforce-percent   Triforce% source (scenes, actors, text)
#   third_party/addons/fast64      HarbourMasters' Fast64 fork (exports SoH scene resources)
#   third_party/Shipwright         Ship of Harkinian source (game build, include tables, Torch)
#   .venv-blender                  Python venv with `bpy` (Blender as a Python module) for headless Fast64
#
# Linux or WSL. Needs git, python3 (3.13 for the pinned bpy wheel), and for the game build the packages in
# third_party/Shipwright/linux-build-deps/apt.txt.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
TP="$ROOT/third_party"

TRIFORCE_PERCENT_COMMIT=12a4bb30f13d97bf616521dbbf2263e9e322f3ab   # 2023-08-06, last upstream commit
HM_FAST64_COMMIT=a1808d8121713b2ab1496baceec3fcf2816625fe           # 2026-10-11
SHIPWRIGHT_COMMIT=f0599f96f8d442f26d0fa1c5cedafddacd56633f         # 2026-10-18, tag 9.3.1 (stock Dewey-Bravo). Previous pin 6b4edce (develop 10/14) built a Windows soh.exe that null-derefed in ResourceUnloadDirectory at startup on at least one box; this tag is known to launch via the stock HarbourMasters release.
BPY_VERSION=5.2.2

clone_at() {
    local url="$1" dir="$2" commit="$3"
    if [ ! -d "$dir/.git" ]; then
        git clone --filter=blob:none "$url" "$dir"
    fi
    git -C "$dir" fetch --quiet origin
    git -C "$dir" checkout --quiet "$commit"
}

mkdir -p "$TP/addons"
clone_at https://github.com/triforce-percent/triforce-percent "$TP/triforce-percent" "$TRIFORCE_PERCENT_COMMIT"
clone_at https://github.com/HarbourMasters/fast64 "$TP/addons/fast64" "$HM_FAST64_COMMIT"
clone_at https://github.com/HarbourMasters/Shipwright "$TP/Shipwright" "$SHIPWRIGHT_COMMIT"
git -C "$TP/Shipwright" submodule update --init --depth 1

# venv layout differs: POSIX uses bin/, Windows (Git Bash on the GH runner) uses Scripts/.
if [ ! -x "$ROOT/.venv-blender/bin/python" ] && [ ! -x "$ROOT/.venv-blender/Scripts/python.exe" ]; then
    python3 -m venv "$ROOT/.venv-blender"
fi
if [ -x "$ROOT/.venv-blender/Scripts/pip.exe" ]; then
    VENV_PIP="$ROOT/.venv-blender/Scripts/pip.exe"
else
    VENV_PIP="$ROOT/.venv-blender/bin/pip"
fi
"$VENV_PIP" install --quiet "bpy==$BPY_VERSION"

echo "Setup done. Next: tools/build_o2r.sh"

#!/usr/bin/env bash
# Build build/TrifoolPercent.o2r: the Triforce Room and Unicorn Fountain scenes, plus the Triforce piece,
# chest and light models. Run tools/setup.sh first.
#
# Published assets (the shipped textures and actor zobjs) live in this repo at $ROOT/assets/, mirrored from
# Triforce%'s repo so we don't depend on it staying up at build time. The scene .blend files are too big to
# mirror sensibly and are still iterable upstream, so they come from the Triforce% clone under
# $TP/triforce-percent/. Blender's Fast64 reads a .blend's textures from the folder next to it, so this
# script copies our mirror into that folder before each scene export.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
TP="$ROOT/third_party"
STAGE="$ROOT/build/stage"
PY="$ROOT/.venv-blender/bin/python"
TORCH="${TORCH:-$ROOT/build/torch/torch}"

rm -rf "$STAGE"
mkdir -p "$STAGE"

# Torch, HarbourMasters' asset converter, from the Shipwright checkout (OoT support only).
if [ ! -x "$TORCH" ]; then
    cmake -S "$TP/Shipwright/torch" -B "$ROOT/build/torch" -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_OOT=ON -DBUILD_SM64=OFF -DBUILD_MK64=OFF -DBUILD_SF64=OFF -DBUILD_PM64=OFF \
        -DBUILD_FZERO=OFF -DBUILD_BK64=OFF -DBUILD_MARIO_ARTIST=OFF -DBUILD_NAUDIO=OFF
    cmake --build "$ROOT/build/torch" --target torch -j"$(nproc)"
fi

# 1. Triforce Room scene and its title card. Published textures from assets/, missing ones from placeholders.
mkdir -p "$TP/triforce-percent/scene/TriforceRoom/textures"
cp -f "$ROOT/assets/scene/TriforceRoom/textures/"*.png "$TP/triforce-percent/scene/TriforceRoom/textures/"
python3 "$ROOT/scenes/triforce_chamber/make_placeholder_textures.py" \
    "$TP/triforce-percent/scene/TriforceRoom/textures"
"$PY" "$ROOT/scenes/triforce_chamber/export_room.py" \
    --blend "$TP/triforce-percent/scene/TriforceRoom/TriforceRoom.blend" \
    --fast64 "$TP/addons" \
    --soh-include "$TP/Shipwright/soh" \
    --out "$STAGE/scene"
python3 "$ROOT/scenes/triforce_chamber/make_title_card.py" "$STAGE/title"

# 2. Triforce actor objects, converted from Triforce%'s compiled zobj files mirrored at assets/actor/.
python3 "$ROOT/actors/triforce_actor/make_torch_input.py" "$ROOT/assets" "$STAGE/torch-in"
(cd "$STAGE" && "$TORCH" o2r "$STAGE/torch-in/triforce_objects.z64" -s "$STAGE/torch-in" -d "$STAGE/torch-out")

# 3. Unicorn Fountain scene, in the unused Test01 debug slot. Same shape as the Triforce Room: mirror ships
#    the four published textures, the placeholder script fills the ones Triforce% never published.
mkdir -p "$TP/triforce-percent/scene/UnicornFountain/textures"
cp -f "$ROOT/assets/scene/UnicornFountain/textures/"*.png "$TP/triforce-percent/scene/UnicornFountain/textures/"
python3 "$ROOT/scenes/unicorn_fountain/make_placeholder_textures.py" \
    "$TP/triforce-percent/scene/UnicornFountain/textures"
"$PY" "$ROOT/scenes/unicorn_fountain/export_fountain.py" \
    --blend "$TP/triforce-percent/scene/UnicornFountain/UnicornFountain.blend" \
    --fast64 "$TP/addons" \
    --soh-include "$TP/Shipwright/soh" \
    --out "$STAGE/fountain-scene"
python3 "$ROOT/scenes/unicorn_fountain/make_title_card.py" "$STAGE/fountain-title" \
    "$ROOT/assets/scene/UnicornFountain/title.ia8.png"

# 4. Hex Test Room: procedurally built on top of TriforceRoom.blend (we only reuse its F3D materials
#    and Scene / Room / Entrance empties). Lands in the unused Syotes slot.
"$PY" "$ROOT/scenes/TestRoom/export_test_room.py" \
    --template "$TP/triforce-percent/scene/TriforceRoom/TriforceRoom.blend" \
    --fast64 "$TP/addons" \
    --soh-include "$TP/Shipwright/soh" \
    --out "$STAGE/testroom-scene"
python3 "$ROOT/scenes/TestRoom/make_title_card.py" "$STAGE/testroom-title"

# 5. One archive for SoH's mods folder.
python3 "$ROOT/tools/pack_o2r.py" "$ROOT/build/TrifoolPercent.o2r" \
    "$STAGE/scene" "$STAGE/title" "$STAGE/torch-out/TrifoolPercent-objects.o2r" \
    "$STAGE/fountain-scene" "$STAGE/fountain-title" \
    "$STAGE/testroom-scene" "$STAGE/testroom-title"

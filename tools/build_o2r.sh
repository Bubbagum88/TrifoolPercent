#!/usr/bin/env bash
# Build build/TrifoolPercent.o2r: the Triforce Room scene plus the Triforce piece, chest and light models.
# Run tools/setup.sh first.
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

# 1. Triforce Room scene and its title card. Triforce% did not publish the room's textures, so stand-ins fill any that are missing.
python3 "$ROOT/scenes/triforce_chamber/make_placeholder_textures.py" \
    "$TP/triforce-percent/scene/TriforceRoom/textures"
"$PY" "$ROOT/scenes/triforce_chamber/export_room.py" \
    --blend "$TP/triforce-percent/scene/TriforceRoom/TriforceRoom.blend" \
    --fast64 "$TP/addons" \
    --soh-include "$TP/Shipwright/soh" \
    --out "$STAGE/scene"
python3 "$ROOT/scenes/triforce_chamber/make_title_card.py" "$STAGE/title"

# 2. Triforce actor objects, converted from Triforce%'s compiled zobj files.
python3 "$ROOT/actors/triforce_actor/make_torch_input.py" "$TP/triforce-percent" "$STAGE/torch-in"
(cd "$STAGE" && "$TORCH" o2r "$STAGE/torch-in/triforce_objects.z64" -s "$STAGE/torch-in" -d "$STAGE/torch-out")

# 3. Unicorn Fountain scene, in the unused Test01 debug slot. Its title card ships as a PNG in the Triforce% repo.
"$PY" "$ROOT/scenes/unicorn_fountain/export_fountain.py" \
    --blend "$TP/triforce-percent/scene/UnicornFountain/UnicornFountain.blend" \
    --fast64 "$TP/addons" \
    --soh-include "$TP/Shipwright/soh" \
    --out "$STAGE/fountain-scene"
python3 "$ROOT/scenes/unicorn_fountain/make_title_card.py" "$STAGE/fountain-title" \
    "$TP/triforce-percent/scene/UnicornFountain/title.ia8.png"

# 4. One archive for SoH's mods folder.
python3 "$ROOT/tools/pack_o2r.py" "$ROOT/build/TrifoolPercent.o2r" \
    "$STAGE/scene" "$STAGE/title" "$STAGE/torch-out/TrifoolPercent-objects.o2r" \
    "$STAGE/fountain-scene" "$STAGE/fountain-title"

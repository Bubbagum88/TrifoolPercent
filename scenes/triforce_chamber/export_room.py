"""Export the Triforce% Triforce Room as Ship of Harkinian scene resources, replacing the Goron Shop.

Run with the Python from tools/setup.sh (it has `bpy`, Blender as a module):

    .venv-blender/bin/python scenes/triforce_chamber/export_room.py \
        --blend third_party/triforce-percent/scene/TriforceRoom/TriforceRoom.blend \
        --fast64 third_party/addons \
        --soh-include third_party/Shipwright/soh \
        --out build/o2r

The resources land in <out>/scenes/shared/golon_scene/, the path SoH loads the Goron Shop from, so packing <out>
into an .o2r and dropping it in SoH's mods folder swaps the room in.

The .blend was made with a 2021 Fast64, so a few things are fixed up before exporting:
- The Triforce piece, chest and light actors used Triforce%'s hijacked actor ids (1, 3, 5). SoH registers them
  at runtime through ActorDB, so they are removed here and spawned by the TrifoolPercent code instead.
- The time warp pillar (Demo_Effect, type 0x0F) is kept; the chest actor shrinks it into a beam.
- The room's object list becomes just the time warp's object.
- The player entrance is re-parented to the room empty, which newer Fast64 requires.
- The room music is set to silence: Triforce% used its own sequence 0x71 (triforce.aseq), which vanilla lacks.
"""

import argparse
import os
import sys

import bpy
from mathutils import Matrix

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "tools", "fast64"))
import hm64_headless  # noqa: E402

SCENE_ID = "SCENE_GORON_SHOP"
CUSTOM_ACTORS = ["TriforceChest", "TriforceLight", "TriforceOfCourage", "TriforceOfPower", "TriforceOfWisdom"]


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--blend", required=True)
    parser.add_argument("--fast64", required=True, help="folder that contains the HM Fast64 addon as `fast64`")
    parser.add_argument("--soh-include", required=True, help="Shipwright's soh/ folder (for include/tables)")
    parser.add_argument("--out", required=True)
    args = parser.parse_args(sys.argv[sys.argv.index("--") + 1 :] if "--" in sys.argv else sys.argv[1:])

    hm64_headless.load(os.path.abspath(args.fast64))
    from fast64.fast64_internal.hm64.z64.scene import export_hm64_scene

    bpy.ops.wm.open_mainfile(filepath=os.path.abspath(args.blend))
    scene = bpy.context.scene
    scene.fast64.oot.feature_set = "hm64"
    scene.ootDecompPath = os.path.abspath(args.soh_include)

    objects = bpy.data.objects
    for name in CUSTOM_ACTORS:
        objects.remove(objects[name])

    time_warp = objects["DemoEffect"].ootActorProperty
    time_warp.actor_id = "ACTOR_DEMO_EFFECT"
    time_warp.params = "0x000F"

    room = objects["Room00"]
    entrance = objects["Entrance"]
    world = entrance.matrix_world.copy()
    entrance.parent = room
    entrance.matrix_world = world
    entrance.ootEntranceProperty.tiedRoom = room
    entrance.ootEntranceProperty.spawnIndex = 0
    entrance.ootEntranceProperty.actor.params = "0x0FFF"

    room.ootRoomHeader.objectList.clear()
    room.ootRoomHeader.objectList.add().objectKey = "obj_efc_tw"

    scene_obj = objects["Scene"]
    scene_obj.ootSceneHeader.musicSeq = "NA_BGM_NO_MUSIC"

    scene.ootSceneExportObj = scene_obj
    settings = scene.ootSceneExportSettings
    settings.option = SCENE_ID
    settings.customExport = False
    settings.exportPath = os.path.abspath(args.out)

    scale = scene.ootBlenderScale
    export_hm64_scene(scene_obj, Matrix.Diagonal((scale, scale, scale)).to_4x4(), settings)
    print(f"Exported Triforce Room to {settings.exportPath}/scenes/shared/golon_scene")
    return 0


if __name__ == "__main__":
    sys.exit(main())

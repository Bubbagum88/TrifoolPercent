"""Export the Triforce% Ending scene as Ship of Harkinian scene resources, in the unused debug slot SCENE_SYOTES2.

Run with the Python from tools/setup.sh (it has `bpy`, Blender as a module):

    .venv-blender/bin/python scenes/ending/export_ending.py \
        --blend scenes/Ending/Ending.blend \
        --fast64 third_party/addons \
        --soh-include third_party/Shipwright/soh \
        --out build/o2r

The resources land in <out>/scenes/shared/syotes2_scene/, the path SoH loads SCENE_SYOTES2 from. Besitu is the
Triforce Room (0x66), Test01 is the Unicorn Fountain (0x65), Syotes is the Test Room hex hub (0x68); Syotes2
(0x69) is the next unused slot.

Fixups, mirroring scenes/unicorn_fountain/export_fountain.py:
- Triforce%'s custom actors (anything the module registers at runtime through ActorDB) are stripped from the
  scene before export, so Fast64 does not try to look them up in the HM64 actor list.
- The player entrance is re-parented to the room empty, as newer Fast64 requires.
- The scene's exit list gets a single exit to Kokiri Forest by Link's house, matching the "test harness"
  convention: every TrifoolPercent room returns there on fade-out.
- Music is set to silence; Triforce%'s own ending sequence will land through the music port.
"""

import argparse
import os
import sys

import bpy
from mathutils import Matrix

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "tools", "fast64"))
import hm64_headless  # noqa: E402

SCENE_ID = "SCENE_SYOTES2"

# Triforce%'s Ending scene wires several custom-id actors inline (goddess terminators, staff roll triggers,
# etc.). TrifoolPercent registers whatever it ports at runtime through ActorDB, so remove them from the
# blend before export rather than teaching Fast64 their ids. Names to prune line up with the actor empties
# in Ending.blend; extend as the SoH-side port grows.
CUSTOM_ACTORS = [
    "GoddessTerminator",
    "StaffRollTrigger",
    "OkarinaTag",
    "PedestaloftheOcarina",
]


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--blend", required=True)
    parser.add_argument("--fast64", required=True, help="folder that contains the HM Fast64 addon as `fast64`")
    parser.add_argument("--soh-include", required=True, help="Shipwright's soh/ folder (for include/tables)")
    parser.add_argument("--out", required=True)
    args = parser.parse_args(sys.argv[sys.argv.index("--") + 1 :] if "--" in sys.argv else sys.argv[1:])

    hm64_headless.load(os.path.abspath(args.fast64))
    from fast64.fast64_internal.hm64.z64.scene import export_hm64_scene  # noqa: F401

    bpy.ops.wm.open_mainfile(filepath=os.path.abspath(args.blend))
    scene = bpy.context.scene
    scene.fast64.oot.feature_set = "hm64"
    scene.ootDecompPath = os.path.abspath(args.soh_include)

    objects = bpy.data.objects
    for name in CUSTOM_ACTORS:
        if name in objects:
            objects.remove(objects[name])

    room = objects["Room"] if "Room" in objects else next((o for o in objects if o.ootEmptyType == "Room"), None)
    if room is None:
        raise RuntimeError("Ending.blend: no object named Room (or empty type Room) found")

    if "Entrance" in objects:
        entrance = objects["Entrance"]
        world = entrance.matrix_world.copy()
        entrance.parent = room
        entrance.matrix_world = world
        entrance.ootEntranceProperty.tiedRoom = room
        entrance.ootEntranceProperty.spawnIndex = 0

    scene_obj = objects["Scene"] if "Scene" in objects else next((o for o in objects if o.ootEmptyType == "Scene"), None)
    if scene_obj is None:
        raise RuntimeError("Ending.blend: no object named Scene (or empty type Scene) found")

    scene_obj.ootSceneHeader.musicSeq = "NA_BGM_NO_MUSIC"
    scene_obj.ootSceneHeader.exitList.clear()
    kokiri_forest = scene_obj.ootSceneHeader.exitList.add()
    kokiri_forest.exitIndex = "Custom"
    kokiri_forest.exitIndexCustom = "ENTR_KOKIRI_FOREST_OUTSIDE_LINKS_HOUSE"

    scene.ootSceneExportObj = scene_obj
    settings = scene.ootSceneExportSettings
    settings.option = SCENE_ID
    settings.customExport = False
    settings.exportPath = os.path.abspath(args.out)

    bpy.ops.object.oot_export_level()
    return 0


if __name__ == "__main__":
    sys.exit(main())

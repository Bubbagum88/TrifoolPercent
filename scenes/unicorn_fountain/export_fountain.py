"""Export the Triforce% Unicorn Fountain as Ship of Harkinian scene resources, in the unused debug scene Test01.

Run with the Python from tools/setup.sh (it has `bpy`, Blender as a module):

    .venv-blender/bin/python scenes/unicorn_fountain/export_fountain.py \
        --blend third_party/triforce-percent/scene/UnicornFountain/UnicornFountain.blend \
        --fast64 third_party/addons \
        --soh-include third_party/Shipwright/soh \
        --out build/o2r

The resources land in <out>/scenes/shared/test01_scene/, the path SoH loads the Test01 debug scene from (map
select "115:Test Map 1"). Besitu is already taken by the Triforce Room; Test01 is the next unused slot.

A few things are fixed up before exporting, similar to the Triforce Room:
- Triforce%'s custom actors (BetaGreatFairy, PedestaloftheOcarina, OkarinaTag) used hijacked actor ids.
  SoH registers them at runtime through ActorDB, so they are removed here and spawned by the TrifoolPercent
  code instead (if and when ported).
- The player entrance is re-parented to the room empty, which newer Fast64 requires.
- The room's music is set to silence: Triforce%'s own sequence lives in its sound set, which vanilla lacks.
- The scene's exit list gets a loading zone out to Kokiri Forest by Link's house, matching the Triforce Room
  POC's test entrance. The actual loading zone's collision material is left to the room's own geometry.
"""

import argparse
import os
import sys

import bpy
from mathutils import Matrix

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "tools", "fast64"))
import hm64_headless  # noqa: E402

SCENE_ID = "SCENE_TEST01"
CUSTOM_ACTORS = ["BetaGreatFairy", "PedestaloftheOcarina", "OkarinaTag"]


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
        if name in objects:
            objects.remove(objects[name])

    room = objects["Room"]
    if "Entrance" in objects:
        entrance = objects["Entrance"]
        world = entrance.matrix_world.copy()
        entrance.parent = room
        entrance.matrix_world = world
        entrance.ootEntranceProperty.tiedRoom = room
        entrance.ootEntranceProperty.spawnIndex = 0

    scene_obj = objects["Scene"]
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

    scale = scene.ootBlenderScale
    export_hm64_scene(scene_obj, Matrix.Diagonal((scale, scale, scale)).to_4x4(), settings)
    print(f"Exported Unicorn Fountain to {settings.exportPath}/scenes/shared/test01_scene")
    return 0


if __name__ == "__main__":
    sys.exit(main())

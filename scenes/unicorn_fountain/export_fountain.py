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

# Visible placeholder body for the Beta Great Fairy. The Triforce% team never published the
# zobj behind their BetaGreatFairy DL, so Tf_Beta_Great_Fairy (soh/.../actors/z_tf_beta_great_fairy.c)
# runs as a mesh-less state machine driving golden sparkle particles. Spawning a vanilla Great
# Fairy at the fountain centre gives the scene an actual body on screen while the ported fairy
# keeps owning the Overture-of-Sages dialogue and the fade-back trigger — the two live side by
# side, the vanilla NPC is scenery only. Confirm the exact ACTOR_*/OBJECT_* identifiers against
# Shipwright's actor tables (soh/include/tables/) before trusting this export; the names below
# follow the brief and may need swapping for the correct Great-Fairy actor/object pair.
PLACEHOLDER_FAIRY_ACTOR_ID = "ACTOR_EN_GE1"
PLACEHOLDER_FAIRY_PARAMS = "0x0000"
PLACEHOLDER_FAIRY_OBJECT_KEY = "object_ge1"


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

    # Visible Great-Fairy placeholder body at the fountain centre. The Tf_Beta_Great_Fairy custom
    # actor (spawned from TrifoolPercent.cpp's SpawnUnicornFountain) stays — this is additive.
    placeholder = bpy.data.objects.new("BetaGreatFairyPlaceholder", None)
    bpy.context.scene.collection.objects.link(placeholder)
    placeholder.empty_display_type = "ARROWS"
    placeholder.ootEmptyType = "Actor"
    placeholder.parent = room
    placeholder.matrix_world = Matrix.Translation((0.0, 0.0, 0.0))
    placeholder.ootActorProperty.actor_id = PLACEHOLDER_FAIRY_ACTOR_ID
    placeholder.ootActorProperty.params = PLACEHOLDER_FAIRY_PARAMS

    # Make sure the fairy's object is in the room's object list. The room already lists whatever
    # the .blend's objectList had; append-only is fine when the key is new, idempotent otherwise.
    object_list = room.ootRoomHeader.objectList
    if not any(entry.objectKey == PLACEHOLDER_FAIRY_OBJECT_KEY for entry in object_list):
        object_list.add().objectKey = PLACEHOLDER_FAIRY_OBJECT_KEY

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

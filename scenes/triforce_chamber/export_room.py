"""Export the Triforce% Triforce Room as Ship of Harkinian scene resources, in the unused debug scene Besitu.

Run with the Python from tools/setup.sh (it has `bpy`, Blender as a module):

    .venv-blender/bin/python scenes/triforce_chamber/export_room.py \
        --blend third_party/triforce-percent/scene/TriforceRoom/TriforceRoom.blend \
        --fast64 third_party/addons \
        --soh-include third_party/Shipwright/soh \
        --out build/o2r

The resources land in <out>/scenes/shared/besitu_scene/, the path SoH loads the Besitu debug scene from (map
select "116:Test Room"). Triforce% used the Goron Shop's slot; Besitu leaves the Goron Shop vanilla. Packing <out>
into an .o2r and dropping it in SoH's mods folder swaps the room in.

The .blend was made with a 2021 Fast64, so a few things are fixed up before exporting:
- The Triforce piece, chest and light actors used Triforce%'s hijacked actor ids (1, 3, 5). SoH registers them
  at runtime through ActorDB, so they are removed here and spawned by the TrifoolPercent code instead.
- The time warp pillar (Demo_Effect, type 0x0F) is kept; the chest actor shrinks it into a beam.
- The room's object list becomes just the time warp's object.
- The player entrance is re-parented to the room empty, which newer Fast64 requires.
- The room music is set to silence: Triforce% used its own sequence 0x71 (triforce.aseq), which vanilla lacks.
- Collision is rebuilt from the visible room. Triforce%'s own collision mesh is only the floors and stairs (Link
  never walked there outside the cutscene), so its walls were missing. The new collision is the room mesh minus
  its water surface, and a water box fills the pool under that surface.
- The room is made indoor (no skybox, sun or moon, clock stopped), as Triforce% left the 2021 defaults.
- The end of the entry walkway gets a loading zone out to Kokiri Forest by Link's house, for quick testing
  together with the module's test entrance there. The room never had an exit.
"""

import argparse
import os
import sys

import bpy
import bmesh  # noqa: E402 (only importable once bpy is loaded)
from mathutils import Matrix

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "tools", "fast64"))
import hm64_headless  # noqa: E402

SCENE_ID = "SCENE_BESITU"
# Blender y of the exit line: the entry walkway runs from the back wall (y -56) to the room; Link spawns at y -46.
EXIT_Y = -53.0
CUSTOM_ACTORS = ["TriforceChest", "TriforceLight", "TriforceOfCourage", "TriforceOfPower", "TriforceOfWisdom"]


def rebuild_collision(room, mesh_obj, old_col_obj):
    """Replace Triforce%'s floor-only collision with the visible room's surfaces, and add a water box."""
    bpy.data.objects.remove(old_col_obj)

    col_mesh = mesh_obj.data.copy()
    water_index = next(i for i, slot in enumerate(mesh_obj.material_slots) if slot.material.name == "WaterMatl")
    bm = bmesh.new()
    bm.from_mesh(col_mesh)
    water_faces = [f for f in bm.faces if f.material_index == water_index]
    water_verts = [mesh_obj.matrix_world @ v.co for f in water_faces for v in f.verts]
    bmesh.ops.delete(bm, geom=water_faces, context="FACES_ONLY")

    # The far end of the entry walkway becomes a loading zone out to Kokiri Forest by Link's house (scene exit 1).
    bmesh.ops.bisect_plane(bm, geom=bm.verts[:] + bm.edges[:] + bm.faces[:], plane_co=(0, EXIT_Y, 0), plane_no=(0, 1, 0))
    exit_material = mesh_obj.material_slots["TileMatl"].material.copy()
    exit_material.name = "ExitToGoronCityMatl"
    exit_material.ootCollisionProperty.exitID = 1
    col_mesh.materials.append(exit_material)
    exit_faces = 0
    for face in bm.faces:
        if face.normal.z > 0.7 and all(v.co.y <= EXIT_Y + 0.01 for v in face.verts):
            face.material_index = len(col_mesh.materials) - 1
            exit_faces += 1
    assert exit_faces, "found no walkway floor to put the exit on"
    bm.to_mesh(col_mesh)
    bm.free()

    col_obj = bpy.data.objects.new("TriforceRoomCollision", col_mesh)
    bpy.context.scene.collection.objects.link(col_obj)
    col_obj.parent = room
    col_obj.matrix_world = mesh_obj.matrix_world
    col_obj.ignore_render = True
    col_obj.ignore_collision = False
    mesh_obj.ignore_collision = True

    # The pool: from the water surface down to the floor under it.
    floor_z = min((mesh_obj.matrix_world @ v.co).z for v in mesh_obj.data.vertices)
    x0, x1 = min(v.x for v in water_verts), max(v.x for v in water_verts)
    y0, y1 = min(v.y for v in water_verts), max(v.y for v in water_verts)
    surface_z = max(v.z for v in water_verts)
    water_box = bpy.data.objects.new("PoolWaterBox", None)
    bpy.context.scene.collection.objects.link(water_box)
    water_box.empty_display_type = "CUBE"
    water_box.ootEmptyType = "Water Box"
    water_box.parent = room
    water_box.matrix_world = Matrix.LocRotScale(
        ((x0 + x1) / 2, (y0 + y1) / 2, (surface_z + floor_z) / 2),
        None,
        ((x1 - x0) / 2, (y1 - y0) / 2, (surface_z - floor_z) / 2),
    )


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

    rebuild_collision(room, objects["TriforceRoomMeshObj"], objects["TriforceRoomColObj"])

    # An indoor room: no sky or sun, and the clock stands still while Link is inside.
    room.ootRoomHeader.disableSkybox = True
    room.ootRoomHeader.disableSunMoon = True
    room.ootRoomHeader.timeSpeed = 0.0

    room.ootRoomHeader.objectList.clear()
    room.ootRoomHeader.objectList.add().objectKey = "obj_efc_tw"

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
    print(f"Exported Triforce Room to {settings.exportPath}/scenes/shared/besitu_scene")
    return 0


if __name__ == "__main__":
    sys.exit(main())

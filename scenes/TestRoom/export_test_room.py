"""Build the TrifoolPercent hex Test Room from scratch and export it as SoH scene resources.

Unlike the Triforce Room and Unicorn Fountain, this scene has no upstream .blend of its own;
the hex is simple enough that we build it procedurally in `bpy` and feed it to Fast64's
exporter. The resources land in `<out>/scenes/shared/syotes_scene/`, which SoH loads for the
unused OoT debug scene `SCENE_SYOTES` (0x68, "118:Syotes" in the map-select).

Fast64 only exports meshes whose materials are F3D materials (its own node-tree-driven material
type), so this script opens TriforceRoom.blend as a scaffold -- Scene/Room/Entrance empties,
F3D materials, Fast64 scene settings -- then wipes its mesh and rebuilds a hex floor on top
using copies of those F3D materials. Six of those material copies have `ootCollisionProperty.exitID`
set to 1..6, one per doorway, so the floor patches at each doorway become loading zones in
the scene's exit list.

Run with the Python from tools/setup.sh (it has `bpy`, Blender as a Python module):

    .venv-blender/bin/python scenes/TestRoom/export_test_room.py \
        --template third_party/triforce-percent/scene/TriforceRoom/TriforceRoom.blend \
        --fast64 third_party/addons \
        --soh-include third_party/Shipwright/soh \
        --out build/stage/testroom-scene
"""

import argparse
import math
import os
import sys

import bpy
import bmesh
from mathutils import Matrix, Vector

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "tools", "fast64"))
import hm64_headless  # noqa: E402

SCENE_ID = "SCENE_SYOTES"

HEX_RADIUS = 40.0
DOORWAY_HALF_WIDTH = 7.0
WALL_HEIGHT = 14.0
DOOR_INSET = 8.0

# Names of F3D materials we expect to find in the template .blend. TileMatl is a plain opaque
# floor material (the entry walkway in the Triforce Room uses it); StoneMatl is the room walls.
FLOOR_MATL_NAME = "TileMatl"
WALL_MATL_NAME = "StoneMatl"


def _copy_f3d_material(src_name, new_name, exit_id=0):
    base = bpy.data.materials[src_name]
    mat = base.copy()
    mat.name = new_name
    if exit_id > 0:
        mat.ootCollisionProperty.exitID = exit_id
    return mat


def _build_floor_mesh(floor_mat, exit_mats):
    """Hex floor with six doorway exit patches. material_index 0 is the plain floor, 1..6 are the exits."""
    mesh = bpy.data.meshes.new("TestRoomFloorMesh")
    bm = bmesh.new()
    center = bm.verts.new((0.0, 0.0, 0.0))

    corner_verts = []
    for i in range(6):
        angle = math.radians(30 + 60 * i)
        corner_verts.append(bm.verts.new((HEX_RADIUS * math.cos(angle), HEX_RADIUS * math.sin(angle), 0.0)))

    doorway_quads = []  # (quad_verts, exit_id)
    tris = []
    for i in range(6):
        a = corner_verts[i].co
        b = corner_verts[(i + 1) % 6].co
        midpoint = (a + b) * 0.5
        along = (b - a).normalized()
        inward = Vector((-along.y, along.x, 0.0))
        if inward.dot(-midpoint) < 0:
            inward = -inward
        d1_outer = midpoint - along * DOORWAY_HALF_WIDTH
        d2_outer = midpoint + along * DOORWAY_HALF_WIDTH
        d1_inner = d1_outer + inward * DOOR_INSET
        d2_inner = d2_outer + inward * DOOR_INSET
        v_d1_outer = bm.verts.new(d1_outer)
        v_d2_outer = bm.verts.new(d2_outer)
        v_d1_inner = bm.verts.new(d1_inner)
        v_d2_inner = bm.verts.new(d2_inner)
        doorway_quads.append(((v_d1_outer, v_d2_outer, v_d2_inner, v_d1_inner), i + 1))

        tris.append((corner_verts[i], v_d1_outer, center))
        tris.append((v_d1_outer, v_d1_inner, center))
        tris.append((v_d2_inner, v_d2_outer, center))
        tris.append((v_d2_outer, corner_verts[(i + 1) % 6], center))

    for tri in tris:
        bm.faces.new(tri)
    for quad_verts, _ in doorway_quads:
        bm.faces.new(quad_verts)

    bm.normal_update()
    bm.to_mesh(mesh)
    bm.free()

    mesh.materials.append(floor_mat)
    for mat in exit_mats:
        mesh.materials.append(mat)

    for face in mesh.polygons:
        face.material_index = 0
    # The six doorway quads are the last six polygons.
    for i, (_, exit_id) in enumerate(doorway_quads):
        mesh.polygons[len(mesh.polygons) - 6 + i].material_index = exit_id

    return mesh


def _build_walls_mesh(wall_mat):
    mesh = bpy.data.meshes.new("TestRoomWallsMesh")
    bm = bmesh.new()
    half_opening = (HEX_RADIUS * math.sqrt(3) - 2 * DOORWAY_HALF_WIDTH) / 2
    for i in range(6):
        angle_a = math.radians(30 + 60 * i)
        angle_b = math.radians(30 + 60 * (i + 1))
        ax, ay = HEX_RADIUS * math.cos(angle_a), HEX_RADIUS * math.sin(angle_a)
        bx, by = HEX_RADIUS * math.cos(angle_b), HEX_RADIUS * math.sin(angle_b)
        along = Vector((bx - ax, by - ay, 0.0)).normalized()
        corner_a = Vector((ax, ay, 0))
        corner_b = Vector((bx, by, 0))
        for base_a, base_b in (
            (corner_a, corner_a + along * half_opening),
            (corner_b - along * half_opening, corner_b),
        ):
            v0 = bm.verts.new(base_a)
            v1 = bm.verts.new(base_b)
            v2 = bm.verts.new((base_b.x, base_b.y, WALL_HEIGHT))
            v3 = bm.verts.new((base_a.x, base_a.y, WALL_HEIGHT))
            bm.faces.new((v0, v1, v2, v3))
    bm.normal_update()
    bm.to_mesh(mesh)
    bm.free()
    mesh.materials.append(wall_mat)
    for face in mesh.polygons:
        face.material_index = 0
    return mesh


def _strip_template_meshes():
    """Delete every mesh, light, and camera from the template .blend; keep the Scene / Room / Entrance empties
    and the F3D materials. We also kill any extra empties from the Triforce Room (DemoEffect, actors) that
    don't belong in the hex hub."""
    keep = {"Scene", "Room00", "Entrance"}
    to_remove = []
    for obj in bpy.data.objects:
        if obj.type == "EMPTY" and obj.name in keep:
            continue
        to_remove.append(obj)
    for obj in to_remove:
        bpy.data.objects.remove(obj, do_unlink=True)


def _add_mesh(name, mesh, parent):
    obj = bpy.data.objects.new(name, mesh)
    bpy.context.scene.collection.objects.link(obj)
    obj.parent = parent
    return obj


def _configure_scene_and_room(soh_include):
    scene = bpy.context.scene
    scene.fast64.oot.feature_set = "hm64"
    scene.ootDecompPath = os.path.abspath(soh_include)

    objects = bpy.data.objects
    scene_root = objects["Scene"]
    room = objects["Room00"]
    entrance = objects["Entrance"]

    # Reset room header to indoor defaults (same as the Triforce Room). The sign actors use
    # OBJECT_GAMEPLAY_KEEP so they don't need a scene object; the beta-content live exhibits
    # (En_Bird, En_Vase, En_Nwc) spawned by TestRoom.cpp do, so their objects go in the room's
    # object list here. Fast64 caps the list at 16; three is fine.
    room.ootRoomHeader.disableSkybox = True
    room.ootRoomHeader.disableSunMoon = True
    room.ootRoomHeader.timeSpeed = 0.0
    room.ootRoomHeader.objectList.clear()
    for object_key in ("obj_bird", "obj_vase", "obj_nwc"):
        entry = room.ootRoomHeader.objectList.add()
        entry.objectKey = object_key

    # Entrance at the room centre, tied to room 0, same spawn index Fast64 writes for the first entry.
    entrance.location = (0.0, 0.0, 0.1)
    entrance.rotation_euler = (0.0, 0.0, 0.0)
    entrance.parent = room
    entrance.ootEntranceProperty.tiedRoom = room
    entrance.ootEntranceProperty.spawnIndex = 0
    entrance.ootEntranceProperty.actor.params = "0x0FFF"

    scene_root.ootSceneHeader.musicSeq = "NA_BGM_NO_MUSIC"
    exits = scene_root.ootSceneHeader.exitList
    exits.clear()
    # Six exit entries, one per doorway. Order matches collision materials' exitID 1..6.
    for entrance_name in (
        "ENTR_KOKIRI_FOREST_OUTSIDE_LINKS_HOUSE",
        "ENTR_BESITU_0",
        "ENTR_CHAMBER_OF_THE_SAGES_0",
        "ENTR_TEST01_0",
        "ENTR_KOKIRI_FOREST_OUTSIDE_LINKS_HOUSE",
        "ENTR_KOKIRI_FOREST_OUTSIDE_LINKS_HOUSE",
    ):
        entry = exits.add()
        entry.exitIndex = "Custom"
        entry.exitIndexCustom = entrance_name

    return scene_root, room


def _build_hex(room):
    floor_mat = _copy_f3d_material(FLOOR_MATL_NAME, "TestRoomFloorMatl")
    exit_mats = [
        _copy_f3d_material(FLOOR_MATL_NAME, f"TestRoomDoorway{i + 1}Matl", exit_id=i + 1)
        for i in range(6)
    ]
    wall_mat = _copy_f3d_material(WALL_MATL_NAME, "TestRoomWallMatl")

    floor = _add_mesh("TestRoomFloor", _build_floor_mesh(floor_mat, exit_mats), room)
    floor.ignore_collision = False
    walls = _add_mesh("TestRoomWalls", _build_walls_mesh(wall_mat), room)
    walls.ignore_collision = False


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--template", required=True,
                        help="A .blend with Fast64 F3D materials to copy (we use TriforceRoom.blend)")
    parser.add_argument("--fast64", required=True, help="folder that contains the HM Fast64 addon as `fast64`")
    parser.add_argument("--soh-include", required=True, help="Shipwright's soh/ folder (for include/tables)")
    parser.add_argument("--out", required=True)
    args = parser.parse_args(sys.argv[sys.argv.index("--") + 1 :] if "--" in sys.argv else sys.argv[1:])

    hm64_headless.load(os.path.abspath(args.fast64))
    from fast64.fast64_internal.hm64.z64.scene import export_hm64_scene

    bpy.ops.wm.open_mainfile(filepath=os.path.abspath(args.template))
    _strip_template_meshes()
    scene_root, room = _configure_scene_and_room(args.soh_include)
    _build_hex(room)

    scene = bpy.context.scene
    scene.ootSceneExportObj = scene_root
    settings = scene.ootSceneExportSettings
    settings.option = SCENE_ID
    settings.customExport = False
    settings.exportPath = os.path.abspath(args.out)

    scale = scene.ootBlenderScale
    export_hm64_scene(scene_root, Matrix.Diagonal((scale, scale, scale)).to_4x4(), settings)
    print(f"Exported Test Room to {settings.exportPath}/scenes/shared/syotes_scene")
    return 0


if __name__ == "__main__":
    sys.exit(main())

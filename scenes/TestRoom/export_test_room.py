"""Build the TrifoolPercent hex Test Room from scratch and export it as SoH scene resources.

Unlike the Triforce Room and Unicorn Fountain, this scene has no upstream .blend to lean on;
the hex is simple enough that we build it procedurally in `bpy`, drop it into a fresh Blender
scene, and feed it to Fast64's exporter. The resources land in
`<out>/scenes/shared/syotes_scene/`, which SoH loads for the unused OoT debug scene
`SCENE_SYOTES` (0x68, "118:Syotes" in the map-select).

Each of the six floor patches at the room's doorways gets its own collision material whose
`ootCollisionProperty.exitID` is 1..6; the scene's exit list is filled in the same order so
walking off a doorway sends Link to the matching destination. Actor empties for the six signs
are placed at the inside edge of each doorway and will be filled in by the C++ glue
(`TestRoom.cpp`) at scene-load time; the signs are not baked into the scene's actor list so
their text ids can live beside the rest of the TrifoolPercent CustomMessage table.

Run with the Python from tools/setup.sh (it has `bpy`, Blender as a Python module):

    .venv-blender/bin/python scenes/TestRoom/export_test_room.py \
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

# Hex room geometry, in Blender units (Fast64 scales these up by ootBlenderScale on export,
# the same factor the TriforceRoom .blend uses). Numbers are hand-picked so Link (~40 units
# tall at scale 1 ≈ 40 OoT units) has room to walk a doorway without catching on the wall.
HEX_RADIUS = 40.0      # center to vertex, in Blender units
DOORWAY_HALF_WIDTH = 7.0
WALL_HEIGHT = 14.0
DOOR_INSET = 8.0       # how far inside the room each doorway floor-patch extends
SIGN_INSET = 14.0      # how far inside the room each sign sits (closer than the doorway)


def _make_material(name, rgb, exit_id=0):
    mat = bpy.data.materials.new(name)
    mat.diffuse_color = (*rgb, 1.0)
    mat.use_nodes = False
    if exit_id > 0:
        mat.ootCollisionProperty.exitID = exit_id
    return mat


def _build_floor_mesh():
    """Return a bmesh for the hex floor with 6 doorway-exit patches and 1 center floor material.

    Material slot order: 0 = floor, 1..6 = exit1..exit6.
    """
    mesh = bpy.data.meshes.new("TestRoomFloorMesh")
    bm = bmesh.new()

    center = bm.verts.new((0.0, 0.0, 0.0))

    # Six corners of the hex, rotated so flat sides face outward (so each side is a wall and
    # each corner points along +X, +Y, ...). We offset by 30° so there is a flat wall facing
    # +Y, which also places doorways at the midpoints of six walls.
    corner_verts = []
    for i in range(6):
        angle = math.radians(30 + 60 * i)
        corner_verts.append(bm.verts.new((HEX_RADIUS * math.cos(angle), HEX_RADIUS * math.sin(angle), 0.0)))

    # For each hex side build: a wall-adjacent floor triangle pair + a doorway floor patch.
    # The patch is a narrow rectangle straddling the midpoint of the side, extending inward.
    bm.verts.ensure_lookup_table()
    doorway_patches = []  # list of (verts_quad, exit_id)
    side_floors = []

    for i in range(6):
        a = corner_verts[i].co
        b = corner_verts[(i + 1) % 6].co
        midpoint = (a + b) * 0.5
        along = (b - a).normalized()
        inward = Vector((-along.y, along.x, 0.0))
        # Make sure inward points toward the room center (0,0).
        if inward.dot(-midpoint) < 0:
            inward = -inward

        # Doorway patch: 4 verts, two on the hex edge, two inside the room.
        d1_outer = midpoint - along * DOORWAY_HALF_WIDTH
        d2_outer = midpoint + along * DOORWAY_HALF_WIDTH
        d1_inner = d1_outer + inward * DOOR_INSET
        d2_inner = d2_outer + inward * DOOR_INSET
        v_d1_outer = bm.verts.new(d1_outer)
        v_d2_outer = bm.verts.new(d2_outer)
        v_d1_inner = bm.verts.new(d1_inner)
        v_d2_inner = bm.verts.new(d2_inner)
        doorway_patches.append(((v_d1_outer, v_d2_outer, v_d2_inner, v_d1_inner), i + 1))

        # Floor triangles filling the rest of this side: corner_a -> d1_outer -> center,
        # and center -> d1_inner -> d2_inner (door-adjacent), and corner_b -> center -> d2_outer.
        side_floors.append((corner_verts[i], v_d1_outer, center))
        side_floors.append((v_d1_outer, v_d1_inner, center))
        side_floors.append((v_d2_inner, v_d2_outer, center))
        side_floors.append((v_d2_outer, corner_verts[(i + 1) % 6], center))

    for tri in side_floors:
        bm.faces.new(tri)
    for quad_verts, _ in doorway_patches:
        bm.faces.new(quad_verts)

    bm.normal_update()
    bm.to_mesh(mesh)
    bm.free()

    # Materials: slot 0 is the plain floor, slot 1..6 are the exit floor patches.
    mesh.materials.append(_make_material("TestRoomFloorMatl", (0.65, 0.55, 0.45)))
    for i in range(6):
        mesh.materials.append(_make_material(f"TestRoomDoorway{i + 1}Matl", (0.80, 0.65, 0.30), exit_id=i + 1))

    # First (6*4=24) tris after the center are the side floor tris (slot 0); the last 6 quads
    # (converted to tris internally) are the doorway patches (slots 1..6).
    for face in mesh.polygons:
        face.material_index = 0
    for i, (_, exit_id) in enumerate(doorway_patches):
        # The N-th doorway quad is at polygon index (len - 6 + N).
        poly_index = len(mesh.polygons) - 6 + i
        mesh.polygons[poly_index].material_index = exit_id

    return mesh


def _build_walls_mesh():
    """Simple wall ribbon between every pair of adjacent corners, with a gap for the doorway."""
    mesh = bpy.data.meshes.new("TestRoomWallsMesh")
    bm = bmesh.new()
    for i in range(6):
        angle_a = math.radians(30 + 60 * i)
        angle_b = math.radians(30 + 60 * (i + 1))
        ax = HEX_RADIUS * math.cos(angle_a)
        ay = HEX_RADIUS * math.sin(angle_a)
        bx = HEX_RADIUS * math.cos(angle_b)
        by = HEX_RADIUS * math.sin(angle_b)
        along = Vector((bx - ax, by - ay, 0.0)).normalized()
        # Two wall halves per side, each from a corner to the near edge of the doorway.
        for base_a, base_b in (
            (Vector((ax, ay, 0)), Vector((ax, ay, 0)) + along * ((HEX_RADIUS * math.sqrt(3) - 2 * DOORWAY_HALF_WIDTH) / 2)),
            (Vector((bx, by, 0)) - along * ((HEX_RADIUS * math.sqrt(3) - 2 * DOORWAY_HALF_WIDTH) / 2), Vector((bx, by, 0))),
        ):
            v0 = bm.verts.new(base_a)
            v1 = bm.verts.new(base_b)
            v2 = bm.verts.new((base_b.x, base_b.y, WALL_HEIGHT))
            v3 = bm.verts.new((base_a.x, base_a.y, WALL_HEIGHT))
            bm.faces.new((v0, v1, v2, v3))
    bm.normal_update()
    bm.to_mesh(mesh)
    bm.free()
    mesh.materials.append(_make_material("TestRoomWallMatl", (0.35, 0.30, 0.25)))
    for face in mesh.polygons:
        face.material_index = 0
    return mesh


def _door_frame_transforms():
    """Yield (sign_pos, exit_label, exit_id, forward_angle) for each of the six doorways."""
    for i in range(6):
        angle_a = math.radians(30 + 60 * i)
        angle_b = math.radians(30 + 60 * (i + 1))
        midpoint = Vector(
            ((HEX_RADIUS * (math.cos(angle_a) + math.cos(angle_b))) / 2,
             (HEX_RADIUS * (math.sin(angle_a) + math.sin(angle_b))) / 2,
             0.0)
        )
        inward = -midpoint.normalized()
        sign_pos = midpoint + inward * SIGN_INSET
        # Face into the room, with the sign offset to one side of the doorway so Link can walk
        # past it; placing them directly on the inward normal is fine for v1.
        forward_angle = math.atan2(inward.y, inward.x)
        yield sign_pos, i + 1, forward_angle


def _add_empty(name, parent, location=(0.0, 0.0, 0.0), rotation_z=0.0):
    bpy.ops.object.empty_add(type="PLAIN_AXES", location=location)
    obj = bpy.context.object
    obj.name = name
    if parent is not None:
        obj.parent = parent
    if rotation_z:
        obj.rotation_euler = (0.0, 0.0, rotation_z)
    return obj


def _add_mesh(name, mesh, parent):
    obj = bpy.data.objects.new(name, mesh)
    bpy.context.scene.collection.objects.link(obj)
    obj.parent = parent
    return obj


def _reset_scene():
    bpy.ops.wm.read_homefile(use_empty=True)


def _configure_fast64(scene, soh_include):
    scene.fast64.oot.feature_set = "hm64"
    scene.ootDecompPath = os.path.abspath(soh_include)


def _build_scene():
    scene_root = _add_empty("Scene", None)
    scene_root.ootEmptyType = "Scene"
    scene_root.ootSceneHeader.musicSeq = "NA_BGM_NO_MUSIC"

    room = _add_empty("Room00", scene_root)
    room.ootEmptyType = "Room"
    room.ootRoomHeader.disableSkybox = True
    room.ootRoomHeader.disableSunMoon = True
    room.ootRoomHeader.timeSpeed = 0.0

    floor = _add_mesh("TestRoomFloor", _build_floor_mesh(), room)
    walls = _add_mesh("TestRoomWalls", _build_walls_mesh(), room)
    floor.ignore_collision = False
    walls.ignore_collision = False

    entrance = _add_empty("Entrance", room, location=(0.0, 0.0, 0.05))
    entrance.ootEmptyType = "Entrance"
    entrance.ootEntranceProperty.tiedRoom = room
    entrance.ootEntranceProperty.spawnIndex = 0
    entrance.ootEntranceProperty.actor.params = "0x0FFF"

    # Signs are spawned from code (TestRoom.cpp) rather than baked into the scene's actor list,
    # so their text ids live beside the rest of TrifoolPercent's CustomMessage table and the
    # hex scene itself needs no sign object in its room object list.

    return scene_root, room


def _configure_exits(scene_root):
    # Exit list is per-scene; it maps 1-based collision-material exit IDs to a destination
    # entrance. The actual destinations live in TestRoom.cpp (CVar-driven), which overrides
    # these at runtime; the baked-in defaults send Link somewhere safe for a bare-o2r build.
    exits = scene_root.ootSceneHeader.exitList
    exits.clear()
    defaults = [
        "ENTR_KOKIRI_FOREST_OUTSIDE_LINKS_HOUSE",
        "ENTR_BESITU_0",
        "ENTR_CHAMBER_OF_THE_SAGES_0",
        "ENTR_TEST01_0",
        "ENTR_KOKIRI_FOREST_OUTSIDE_LINKS_HOUSE",
        "ENTR_KOKIRI_FOREST_OUTSIDE_LINKS_HOUSE",
    ]
    for entrance in defaults:
        entry = exits.add()
        entry.exitIndex = "Custom"
        entry.exitIndexCustom = entrance


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--fast64", required=True, help="folder that contains the HM Fast64 addon as `fast64`")
    parser.add_argument("--soh-include", required=True, help="Shipwright's soh/ folder (for include/tables)")
    parser.add_argument("--out", required=True)
    args = parser.parse_args(sys.argv[sys.argv.index("--") + 1 :] if "--" in sys.argv else sys.argv[1:])

    hm64_headless.load(os.path.abspath(args.fast64))
    from fast64.fast64_internal.hm64.z64.scene import export_hm64_scene

    _reset_scene()
    scene = bpy.context.scene
    _configure_fast64(scene, args.soh_include)
    scene_root, _ = _build_scene()
    _configure_exits(scene_root)

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

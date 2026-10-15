"""Load HarbourMasters' Fast64 fork into a headless `bpy` (Blender as a Python module).

Fast64 is written for the Blender UI. Two things need help when there is no window:
- the addon has to be imported from a folder on sys.path and registered by hand;
- the scene exporter un-hides objects through `bpy.ops.object.hide_view_clear()`, whose poll fails without a 3D
  view, so it is swapped for a version that only touches object properties.

Usage from a script run with the bpy venv's python:

    import hm64_headless
    fast64 = hm64_headless.load(addons_dir)   # folder that contains the `fast64` addon folder
"""

import importlib
import sys


def _unhide_all(scene):
    state = []
    for obj in scene.objects:
        state.append((obj, obj.hide_get(), obj.hide_viewport))
        obj.hide_set(False)
        obj.hide_viewport = False
    return state


def _restore_hidden(state):
    for obj, hidden, hidden_viewport in state:
        obj.hide_set(hidden)
        obj.hide_viewport = hidden_viewport


def load(addons_dir: str):
    if addons_dir not in sys.path:
        sys.path.insert(0, addons_dir)
    fast64 = importlib.import_module("fast64")
    fast64.register()

    exporter = importlib.import_module("fast64.fast64_internal.z64.exporter")
    exporter.unhideAllAndGetHiddenState = _unhide_all
    exporter.restoreHiddenState = _restore_hidden
    return fast64

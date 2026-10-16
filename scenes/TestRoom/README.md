# TestRoom (hex hub)

A six-doored hexagon that acts as the TrifoolPercent dev harness: Link spawns in
the middle, picks a doorway, and the loading zone there warps him to one of the
scenes we are iterating on. Everything about the room is built fresh at
`.o2r` time: no .blend is checked in because the geometry is simple enough that
`make_test_room_blend.py` can build it from scratch each run.

Scene slot: `SCENE_SYOTES` (unused OoT debug slot 0x68 — "118:Syotes" in the
SoH map-select). The hex hub lives there so the already-released Triforce
Room (Besitu / 0x66) and Unicorn Fountain (Test01 / 0x65) stay put.

Doorway → destination map (edit `TestRoom.cpp` to re-wire):

| # | Label                 | Entrance                                             |
|---|-----------------------|------------------------------------------------------|
| 1 | Kokiri Forest         | `ENTR_KOKIRI_FOREST_OUTSIDE_LINKS_HOUSE`             |
| 2 | Triforce Room         | `ENTR_BESITU_0`                                      |
| 3 | Modified Sacred Realm | placeholder — warps to vanilla `ENTR_CHAMBER_OF_THE_SAGES_0` until the modified room lands |
| 4 | Unicorn Fountain      | `ENTR_TEST01_0`                                      |
| 5 | (future)              | inert — a "coming soon" sign                         |
| 6 | (future)              | inert — a "coming soon" sign                         |

Signs are vanilla En_Kanban actors; each sign's text id is overridden with the
label above through `CustomMessageManager` the same way the Beta Kokiri lines
land in `Beat1KeyOfCourage.cpp`.

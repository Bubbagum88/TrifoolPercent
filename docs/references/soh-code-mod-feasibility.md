# Can stock SoH load compiled code from an `.o2r`?

**Finding, as of 2026-10-15:** No. Stock Ship of Harkinian through 9.2.3 ("Ackbar Delta") treats `.o2r` strictly as a resource archive (textures, sequences, models, scenes). There is no C mod loader, no manifest format for native code, and no sign upstream intends to add one.

This answers the Phase 2 roadmap item ("Check whether SoH can load compiled code from a mod package") in the negative. The TrifoolPercent module keeps shipping inside a SoH fork build.

## What `.o2r` actually is

Shipwright's `soh/Enhancements/mod_menu.cpp` enables a `.o2r` by calling
`GetArchiveManager()->AddArchive(...)` and nothing more. libultraship's
`ArchiveManager` / `ResourceManager` only know how to serve resources from the
archive to the game's existing loaders. The packer at
`soh/assets/tools/soh-o2r-packer/main.cpp` only knows resource types.

GitHub code search for `dlopen`, `LoadLibrary`, and `manifest.json` across libultraship and Shipwright: **0 hits.** No PRs or issues proposing a C mod system. The ambitious SoH-adjacent forks (Starship, 2Ship2Harkinian) share the same asset-only resource model.

## What a code mod system would actually need

If we wanted to build this inside a fork of SoH, the moving parts are:

1. **A `manifest.json` schema inside the `.o2r`.** Per-platform entry filename (`.dll` / `.so` / `.dylib`), ABI version, required SoH / libultraship version, name of an `extern "C"` entry symbol.
2. **A loader step** next to `AddArchive` in `mod_menu.cpp`:
   - Extract the native binary to a temp dir (`dlopen` can't load from inside an archive).
   - `dlopen` / `LoadLibrary` the extracted binary.
   - Resolve the entry symbol, e.g. `extern "C" void SohModInit(SohModHostApi*)`.
   - Call it with a vtable of host-API callbacks.
3. **A stable public header** for the pieces mods would need: `GameInteractor` (currently internal), `ResourceManager`, `CVar`, a shared ImGui context, maybe `CustomMessageManager` and `ActorDB`.
4. **ABI discipline across SoH versions and compilers** (MSVC / GCC / Clang).

## Caveats worth flagging even if we did build it

- **No ABI promise from SoH.** Every point release can silently break the `.so`.
- **Switch and Wii U** use static builds — no dynamic loading at all. Those platforms can't ship code mods, period.
- **macOS Gatekeeper and Windows SmartScreen** flag unsigned extracted binaries.
- **No upstream appetite.** Zero issues, zero PRs. Any loader we add is a fork we carry.

## Where this leaves TrifoolPercent

Drop "ship C++ inside `.o2r`" from the plan. The realistic shipping options are:

1. **Ship a prebuilt SoH fork binary beside the asset `.o2r`.** What every ambitious SoH mod does today (Shoda, 2Ship, etc.). The `.o2r` lands in `mods/`, the fork binary ships separately as a prebuilt download.
2. **Keep the module small** and port the parts that *need* to live in-engine (actors, hooks) to whatever scripted surface lands upstream later. Everything that can be data (scenes, textures, text, cutscenes) stays in the `.o2r`, which drops into stock SoH too — just without the gameplay logic.

If we still want the drag-and-drop UX, option 1 is the realistic answer. Building the loader ourselves is a real project — adding `manifest.json`, the extractor, `dlopen`, the host-API header, and the ABI versioning to a SoH fork — not a configuration flip.

## Key references

- `soh/soh/Enhancements/mod_menu.cpp` — what enabling an `.o2r` actually does today.
- `soh/assets/tools/soh-o2r-packer/main.cpp` — the packer only knows resource types.
- libultraship's `ArchiveManager` and `ResourceManager` — resource serving, no code loading.
- `third_party/Shipwright/` — pinned to `6b4edce` in `tools/setup.sh`; this finding stands against that commit.

# TrifoolPercent

A **Ship of Harkinian** mod project inspired by the legendary Triforce% ACE/TAS run — but with a playful twist.  
No ACE. No memory corruption. Just pure modding magic.

## Goals
- Recreate the mythical Triforce quest inside SoH
- Port/rebuild key assets (Triforce object, Unicorn Fountain, Beta Fairy, etc.)
- Add custom cutscenes, actors, and quest logic
- Make it fun, cinematic, and community-friendly

## Folder Structure
- **assets/** — models, textures, animations
- **scenes/** — custom and ported scenes
- **actors/** — custom actor definitions
- **cutscenes/** — cutscene scripts
- **scripts/** — quest logic and triggers
- **tools/** — development tools (Fast64, Blender, etc.)
- **docs/** — documentation and roadmap
- **build/** — final mod files (.o2r)

## Tools
These are the existing tools we use to rebuild Triforce% natively inside Ship of Harkinian. Every link below was checked against a live repository in October 2026.

### Ship of Harkinian
| Tool | What we use it for |
|------|--------------------|
| [Shipwright](https://github.com/HarbourMasters/Shipwright) | The SoH source. Code-side changes (custom actors, quest flags, entrance rerouting) are built against it. Start with its [modding guide](https://github.com/HarbourMasters/Shipwright/blob/develop/docs/MODDING.md) and [building guide](https://github.com/HarbourMasters/Shipwright/blob/develop/docs/BUILDING.md). |
| [libultraship](https://github.com/HarbourMasters/libultraship) | The engine layer under SoH. It loads `.o2r`/`.otr` archives, so a mod archive can replace base game resources. Its [scripting guide](https://github.com/HarbourMasters/libultraship/blob/main/docs/SCRIPTING.md) covers the C mod system (`manifest.json`, events) in newer versions; check that the SoH build you target includes it. |

### Building mod archives (`.o2r`)
| Tool | What we use it for |
|------|--------------------|
| [Retro](https://github.com/HarbourMasters/retro) | GUI that packs custom textures, sequences and other files into `.o2r`/`.otr` mods for SoH's `mods` folder. |
| [Torch](https://github.com/HarbourMasters/Torch) | HarbourMasters' asset converter that turns ROM asset definitions into archive resources. |
| [ZAPDTR](https://github.com/HarbourMasters/ZAPDTR) and [OTRExporter](https://github.com/HarbourMasters/OTRExporter) | What SoH uses to build `oot.o2r` from your ROM. Useful for seeing how a scene, object or cutscene is stored as a resource. |

### Models, scenes and actors
| Tool | What we use it for |
|------|--------------------|
| [Fast64](https://github.com/Fast-64/fast64) | Blender plugin that exports OoT models, skeletons, collision, scenes and cutscenes. Triforce% built its Unicorn Fountain, Triforce Room and other scenes with it. HarbourMasters keeps a fork at [HarbourMasters/fast64](https://github.com/HarbourMasters/fast64). |
| [SharpOcarina](https://github.com/Nokaubure/SharpOcarina) | Windows scene and room editor for OoT, for placing actors, exits and cameras. |
| [z64scene](https://github.com/z64me/z64scene) | Cross-platform Zelda 64 scene editor (work in progress). |
| [z64convert](https://github.com/z64me/z64convert) | Converts 3D models into Zelda 64 object files. |
| [Z64Utils](https://github.com/Random06457/Z64Utils) | ROM object viewer for browsing vanilla display lists, textures, skeletons and animations, including unused beta objects. |

### Audio
| Tool | What we use it for |
|------|--------------------|
| [SEQ64](https://github.com/sauraen/seq64) | Sequence (music) editor. SoH imports Seq64 sequences through Retro, as described in its [custom music guide](https://github.com/HarbourMasters/Shipwright/blob/develop/docs/CUSTOM_MUSIC.md). |
| [z64audio](https://github.com/z64tools/z64audio) | Converts WAV/AIFF audio into N64 sample formats. |

### Reference code
| Project | Why it matters |
|---------|----------------|
| [triforce-percent](https://github.com/triforce-percent/triforce-percent) | The original TASBot Triforce% code, scenes and assets. Breakdown is in [docs/references/triforcepercent](docs/references/triforcepercent/index.md). |
| [zeldaret/oot](https://github.com/zeldaret/oot) | The OoT decompilation. SoH's game code comes from it, so actor and scene structures match. |
| [HackerOoT](https://github.com/HackerN64/HackerOoT) | Decomp-based OoT modding base, a good reference for custom actor and scene patterns. |

## Status
Early development — foundation laid.

## How to Use
1. Clone this repo
2. Build assets into `.o2r` archives using the [tools](#tools) above (for example Retro)
3. Place them in your Ship of Harkinian `mods` folder
4. Launch SoH and test!

## Inspiration
- Original Triforce% project
- OoT beta content
- Ship of Harkinian modding community

Contributions welcome!

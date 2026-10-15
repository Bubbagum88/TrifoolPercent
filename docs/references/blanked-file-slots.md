# Blanked file slots we can repurpose

Vanilla OoT ships these files as empty (zeroed) entries in the DMA table. Upstream Triforce% stole real-but-unused scene slots (Granny's Potion Shop → Unicorn Fountain, Goron Shop → Triforce Room, Dampé's Hut → Ending) and so overwrote scenes that still work in the base game. TrifoolPercent can skip that trade-off by dropping its own resources into these blanked slots instead — nothing in the vanilla ROM references them, so a replacement displaces nothing.

Source: [TCRF — Ocarina of Time § Blanked Files](https://tcrf.net/The_Legend_of_Zelda:_Ocarina_of_Time#Blanked_Files).

| Table idx | Original filename                 | Good TrifoolPercent use |
|-----------|-----------------------------------|-------------------------|
| 0x01F5    | `gameplay_object_exchange_static` | Shared "always-resident" object for TrifoolPercent actors (Triforce pieces, chest, light, Sages' Charm icon) so they do not have to piggyback `gameplay_keep`. |
| 0x025F    | `object_warp2`                    | Second warp/portal object — pair with upstream's `z_oceff_wipe` patch for the Overture-of-Sages warp. |
| 0x0270    | `object_efc_flash`                | FX object — hold the Triforce scene-transition textures and the "pieces combine" flash DL. |
| 0x0289    | `object_demo_tre_lgt`             | Treasure light / demo-light object — Triforce Light sparkle, HolyLight VFX. |
| 0x0349    | `object_spot01_objects2`          | Kakariko-area object slot — Granny's door turnaround / Zora's Domain reroute scenery lives here cleanly. |
| 0x03EE    | `elf_message_ydan`                | Navi hint data for the Deku Tree — reuse to carry TrifoolPercent's custom Navi lines without touching the real `elf_message_field` / `_grotto`. |
| 0x05EF–0x05F4 | `anime_model_1-6_static`      | Six empty "animated model" static slots — ideal for the custom Link animations (Meditate, ReachUp, LookToMeditate) and the BotW Link/Zelda baked anims. |
| 0x05F5–0x05FA | `anime_texture_1-6_static`    | Six empty "animated texture" slots — Chamber-of-Sages water, light rays, cel-shaded tunic textures, Twitch emote strips. |

Rules of thumb when claiming one:

- Build the slot's resource under `assets/<category>/<name>/` and reference it by `__OTR__objects/<slot_name>/<sym>` in the SoH module header, same pattern as [`TrifoolPercent.h`](../../soh/Enhancements/TrifoolPercent/TrifoolPercent.h).
- Keep the SoH-side name on the TrifoolPercent symbol (`gTrifool…`) rather than the vanilla slot name, so the slot number stays an implementation detail.
- Note the claim here (table above) so two beats don't try to grab the same slot.

# Acknowledgements & Special Thanks

> See also: [CREDITS.md](../CREDITS.md) for the per-contributor roll.

**Contents**
- [The Triforce% Team](#the-triforce-team)
- [Special Thanks — the OoT community](#special-thanks--the-oot-community)
- [Inspiration](#inspiration)

## The Triforce% Team

TrifoolPercent would not exist without the pioneering work of **The Triforce% Team (2019–2022)**. Their original ACE/TAS showcase, source code and custom assets are the foundation this project ports onto Ship of Harkinian.

- Upstream repository: [triforce-percent/triforce-percent](https://github.com/triforce-percent/triforce-percent)
- Upstream site: [gettriforce.link](https://gettriforce.link)
- Live run: [TASBot's Triforce% at AGDQ 2023](https://www.youtube.com/watch?v=2x_pqyrf9lA)
- A vendored copy of their LICENSE and README lives under [`upstream/`](../upstream/).

Vendored source trees from the upstream repo in this project (all authored by the Triforce% Team, no Nintendo content):

- [`actors/`](../actors/) — custom and patched actor sources
- [`scenes/`](../scenes/) — custom scenes and vanilla scene patches
- [`assets/anim`](../assets/anim/), [`assets/model`](../assets/model/), [`assets/music`](../assets/music/), [`assets/textures`](../assets/textures/), [`assets/voice`](../assets/voice/), [`assets/text`](../assets/text/) — animations, models, music, textures, voice clips and message text
- [`scripts/statics/`](../scripts/statics/) — engine-glue statics (quest state, scene wiring, ocarina, audio, interface, messages)

Per the upstream license, members of the Triforce% Team **must be credited in the TrifoolPercent in-game Staff Roll** when the mod ships. Builds of TrifoolPercent do not and will not redistribute ROM data or any assets copyrighted by Nintendo.

## Special Thanks — the OoT community

Huge thanks to the broader **Ocarina of Time modding and preservation community** — decades of their reverse engineering, cataloguing and documentation of OoT's unused content is what makes a project like this feasible for one person. In particular, these references saved an enormous amount of work:

- [**TCRF — Unused Actors & Objects**](https://tcrf.net/The_Legend_of_Zelda:_Ocarina_of_Time/Unused_Actors_%26_Objects) — every unused actor and object listed with its ID/address. Goated.
- [**TCRF — Unused Content in Used Actors & Objects**](https://tcrf.net/The_Legend_of_Zelda:_Ocarina_of_Time/Unused_Content_in_Used_Actors_%26_Objects) — same, for content hiding inside shipped actors and objects.
- [**Zelda Wiki — Unused Content (OoT)**](https://zelda.fandom.com/wiki/Unused_Content) — overview, screenshots and historical context for beta/cut material.
- [**The Spriters Resource — Ocarina of Time models**](https://models.spriters-resource.com/nintendo_64/thelegendofzeldaocarinaoftime/) — downloadable rips of OoT models (shipped, unused and beta) with ready-to-use geometry. Massive for iterating on this mod without hunting addresses.

And of course the people and projects behind the tooling TrifoolPercent is built on: **HarbourMasters** (Ship of Harkinian, Fast64 fork, Torch, Retro), the **OoT decompilation team** at [zeldaret/oot](https://github.com/zeldaret/oot), **HackerOoT**, **Fast64**, **z64me**, **Sauraen** (SEQ64), **Random06457** (Z64Utils), and everyone contributing to the SoH modding community.

## Inspiration
- Original Triforce% project
- OoT beta content
- Ship of Harkinian modding community

See also [CREDITS.md](../CREDITS.md) at the repo root for the per-contributor roll.

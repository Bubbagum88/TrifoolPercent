# GitHub "About" sidebar — recommended config

GitHub shows an **About** sidebar at the top-right of the repo home page. It's where visitors read the one-line description, the website link, and the topic tags that make the repo show up in GitHub's own search. It's not controlled by any file in the repo — you edit it from the UI (repo home → gear icon next to **About**). This doc is a copy-paste cheat sheet for what to put there.

> Why bother: the About sidebar and topics drive discovery. Searching GitHub for `ocarina-of-time` or `ship-of-harkinian` turns up repos *because they tagged themselves*, not because of README text. One minute of config here is the single biggest "findability" win.

## Description (160-char limit)

> A Ship of Harkinian mod that rebuilds the legendary Triforce% ACE/TAS run as a native SoH experience — custom scenes, actors, and cutscenes, no ACE.

## Website

Pick one:

- `https://github.com/Bubbagum88/TrifoolPercent/blob/main/docs/roadmap.md` — if the roadmap is the main thing to show
- `https://gettriforce.link` — if you'd rather send people to the original Triforce% site
- A dedicated project site later, if one exists

## Topics (max 20; pick 5–10)

Core identifiers:
- `ocarina-of-time`
- `ship-of-harkinian`
- `oot-mod`
- `n64-mod`
- `rom-hack`
- `zelda`
- `triforce-percent`

Audience / tech:
- `modding`
- `fast64`
- `blender`
- `n64`

> Add topics one at a time — the sidebar autocompletes against the global topic registry, so already-popular ones (like `ocarina-of-time`) are what you want.

## Toggles under the About gear

- [x] **Releases** — on, so the latest build is one click away.
- [x] **Packages** — leave off for now; no packages yet.
- [x] **Deployments** — leave off; CI only uploads workflow artifacts.
- [ ] **Use your GitHub Pages website** — off for now (no Pages site yet).

## Repo settings → Features

- [x] **Issues** — on.
- [ ] **Projects** — optional; a Project board is a nice way to track the porting checklist, but a `docs/porting-checklist.md` is already serving that role.
- [x] **Discussions** — recommend turning on. It's the right home for "how do I add X" / asset-sharing threads that aren't bugs or feature requests.
- [ ] **Wiki** — off. We're using `docs/` instead.
- [x] **Preserve this repository** — on if you want it archived with the Software Heritage / Arctic Code Vault programs.

## Social preview image

Settings → General → **Social preview** → upload a 1280×640 PNG. This is what shows up when the repo URL is pasted into Discord/Twitter/etc. A stylized title card using the in-game Triforce% font would be perfect; a cropped Triforce Room screenshot works too.

# Handoff: Linux/macOS Port of The Simpsons: Hit & Run

Last updated: 2026-07-19. This document is the current port handoff for
`main`. Older proof-of-concept notes are preserved in `initial-poc-plan.md` and
are historical unless explicitly called out here.

## Goal

Make the restored full-game target (`SRR2`) reproducibly playable on Linux, then
continue desktop stabilization on Linux/macOS.

## Current state (TL;DR)

- `main` builds the adopted full-game tree: `code/SRR2` links against restored
  middleware in `libs/` with SDL2/OpenGL, OpenAL, and FFmpeg.
- Desktop uses the upstream convention `RAD_WIN32 + RAD_CONSOLE + RAD_SDL`.
  Keep `RAD_PC` undefined.
- SDL2 is the supported desktop path. SDL3 remains opt-in and is not part of the
  current validation milestone.
- Linux retail-asset smoke tests now pass under local Xvfb/llvmpipe/null audio:
  normal boot reaches `FRONTEND`, and `skipfe` reaches Level 1 `GAMEPLAY`.
- The old Linux `licensePC.p3d` investigation is no longer the active blocker:
  current `libs/radcore/src/radfile/sdl/sdldrive.cpp` does not lowercase
  `m_DrivePath`, and validation opened `licensePC.p3d` successfully.
- Runtime breadcrumbs are printed to stdout on `RAD_SDL` builds:
  `GameFlow context: OLD -> NEW`.
- A local non-Docker Xvfb harness exists at
  `tools/linux-runtime/run-local-game.sh`. The Docker harness remains at
  `tools/linux-runtime/run-game.sh`.
- Save-path policy is WIP on Linux SDL: saves are routed through a synthetic
  `SAVE:` drive to `$XDG_DATA_HOME/srr2`, `$HOME/.local/share/srr2`, or local
  `save/` only when HOME/XDG are unavailable. Build/path-selection smoke passed;
  UI save creation/load still needs validation before treating this as complete.

## Runtime validation completed on 2026-07-18

Host: Arch Linux, Release build, SDL2, Mesa llvmpipe, Xvfb, OpenAL null driver.
Docker was not usable on this machine (`/var/run/docker.sock` missing), so local
Xvfb was installed/used instead.

Retail assets were extracted outside the repo:

```sh
7z x -y -o/tmp/shar-assets "The Simpsons - Hit & Run.rar"
ASSET_ROOT="/tmp/shar-assets/The Simpsons - Hit & Run"
```

Required files verified:

```text
art/
movies/
scripts/
sound/
simpsons.ini
art/frontend/dynaload/images/license/licensePC.p3d
art/frontend/scrooby/frontend.p3d
```

Clean build command used:

```sh
cmake -S . -B build/native -DCMAKE_BUILD_TYPE=Release -DSRR2_BUILD_TESTS=OFF -DSRR2_USE_PCH=OFF
cmake --build build/native -j$(nproc)
```

Smoke commands used:

```sh
ASSET_ROOT="/tmp/shar-assets/The Simpsons - Hit & Run" \
OUT_DIR=/tmp/srr2-smoke/frontend-final \
RUN_SECONDS=60 \
SKIP_BUILD=1 \
tools/linux-runtime/run-local-game.sh

ASSET_ROOT="/tmp/shar-assets/The Simpsons - Hit & Run" \
OUT_DIR=/tmp/srr2-smoke/skipfe-final \
RUN_SECONDS=120 \
GAME_ARGS=skipfe \
SKIP_BUILD=1 \
tools/linux-runtime/run-local-game.sh
```

Frontend result:

- Log: `/tmp/srr2-smoke/frontend-final/game.log`
- Screenshot: `/tmp/srr2-smoke/frontend-final/screenshot.png`
- Expected timeout status: `124`
- Key log lines:

```text
OpenGL - Vendor: Mesa, Renderer: llvmpipe (LLVM 22.1.8, 256 bits), Version: 4.6 (Compatibility Profile) Mesa 26.1.5-arch1.1
GameFlow context: ENTRY -> BOOTUP
GameFlow context: BOOTUP -> FRONTEND
```

Screenshot showed the title/frontend "PRESS START" screen. There was no
`FileNotFound` for `licensePC.p3d` or `frontend.p3d`.

`skipfe` gameplay result:

- Log: `/tmp/srr2-smoke/skipfe-final/game.log`
- Screenshot: `/tmp/srr2-smoke/skipfe-final/screenshot.png`
- Expected timeout status: `124`
- Key log lines:

```text
OpenGL - Vendor: Mesa, Renderer: llvmpipe (LLVM 22.1.8, 256 bits), Version: 4.6 (Compatibility Profile) Mesa 26.1.5-arch1.1
GameFlow context: ENTRY -> BOOTUP
GameFlow context: BOOTUP -> LOADING_GAMEPLAY
GameFlow context: LOADING_GAMEPLAY -> GAMEPLAY
```

Screenshot showed Level 1 gameplay outside the Simpsons house with the desktop
keyboard tutorial overlay. No pause-settings camera layout crash occurred.

A quick 30s `skipfe` run without `ALSOFT_DRIVERS=null` also reached gameplay;
subjective audio quality was not validated in headless Xvfb.

## First blocker found and fixed

Initial local Xvfb screenshots were black and `xwininfo` showed no child window
inside Xvfb. The host session was Wayland, so SDL could choose Wayland and create
the game window on the user's compositor while the harness captured the Xvfb root
window.

Fixes applied:

- `tools/linux-runtime/run-game.sh` now forces `SDL_VIDEODRIVER=x11` and unsets
  `WAYLAND_DISPLAY` before launching under Xvfb.
- `tools/linux-runtime/run-local-game.sh` does the same for non-Docker smoke
  tests.
- `code/gameflow/gameflow.cpp` now prints `GameFlow context: OLD -> NEW` via
  stdout on `RAD_SDL`, because `rReleasePrintf` is compiled out in this Release
  configuration.

## How to repeat the current Linux smoke tests

Preferred local path when Docker is unavailable:

```sh
# One-time asset extraction, outside the repo.
7z x -y -o/tmp/shar-assets "The Simpsons - Hit & Run.rar"

# Build.
cmake -S . -B build/native -DCMAKE_BUILD_TYPE=Release -DSRR2_BUILD_TESTS=OFF -DSRR2_USE_PCH=OFF
cmake --build build/native -j$(nproc)

# Frontend smoke.
ASSET_ROOT="/tmp/shar-assets/The Simpsons - Hit & Run" \
OUT_DIR=/tmp/srr2-smoke/frontend \
RUN_SECONDS=60 \
SKIP_BUILD=1 \
tools/linux-runtime/run-local-game.sh

# Gameplay shortcut smoke.
ASSET_ROOT="/tmp/shar-assets/The Simpsons - Hit & Run" \
OUT_DIR=/tmp/srr2-smoke/skipfe \
RUN_SECONDS=120 \
GAME_ARGS=skipfe \
SKIP_BUILD=1 \
tools/linux-runtime/run-local-game.sh
```

Required local packages include CMake/build dependencies plus Xvfb and xwd
(`xorg-server-xvfb xorg-xwd` on Arch, `xvfb x11-apps` on Debian/Ubuntu).

Docker path once the daemon is available:

```sh
docker build -t srr2-linux-runtime tools/linux-runtime
mkdir -p /tmp/srr2-docker-out
docker run --rm \
  -v "$PWD:/src:ro" \
  -v "/tmp/shar-assets/The Simpsons - Hit & Run:/assets:ro" \
  -v /tmp/srr2-docker-out:/out \
  -e RUN_SECONDS=120 \
  -e GAME_ARGS=skipfe \
  srr2-linux-runtime
```

## Important Linux-specific behavior to preserve

- Do not reintroduce lowercasing of `m_DrivePath` on Linux. Linux asset lookup
  should stay case-sensitive and deterministic.
- Keep boot FMVs skipped on `RAD_SDL`; failed `.rmv`/FFmpeg setup should not
  block startup.
- Keep FFmpeg movie loading tolerant of unavailable streams/conversions.
- Keep the keyboard/mouse virtual controller exposed as `Port0\\Slot0` so a
  physical gamepad is not required.
- Keep SDL3 opt-in until the desktop OpenGL backend has a verified SDL3 path.

## Remaining work / next triage targets

1. Manual input pass: verify keyboard/mouse can move, drive, pause, reset car,
   and navigate the frontend on a real desktop session.
2. Audio pass: compare real OpenAL output with `ALSOFT_DRIVERS=null`; log music,
   dialog, and movie-audio issues separately from gameplay blockers.
3. Save/load pass: if saving is broken, define a Linux save directory policy
   (likely `$XDG_DATA_HOME/srr2` with a local `save/` fallback) and wire it
   through the RAD drive model.
4. Longer mission playthrough: start/fail/restart/complete at least the first
   Level 1 mission, then expand to all retail levels.
5. Optional later work: SDL3 backend repair, packaging, and CI workflow
   activation.

## Historical notes

- The pre-adoption Linux PoC (`srr2-linux-poc`) was useful for early vertical
  slicing and asset/case probes, but it is not the current port target.
  Current work should stabilize `SRR2`.
- The old handoff's Docker/Xvfb `licensePC.p3d` failure should not be pursued
  unless it reproduces with the commands above. If a future file lookup fails,
  instrument path composition around `radSdlDrive::BuildFileSpec`/open results
  before changing lookup behavior; avoid case-folding as a first response.

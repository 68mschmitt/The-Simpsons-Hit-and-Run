# Handoff: Linux/macOS Port of The Simpsons: Hit & Run

Last updated: 2026-07-07. This document hands off the port effort with all
context, decisions, lessons, and the exact state of open problems. Read this
first; details live in the linked docs.

## Goal

Make the game fully playable on Linux and macOS. Set as a standing goal on
2026-07-07; substantial progress was made in one session but gameplay
verification is not finished.

## Where Things Stand (TL;DR)

- Branch **`zenoarrows-base`** (current) builds the **complete game** (`SRR2`)
  on macOS (clang/arm64, Homebrew SDL2) and Ubuntu 24.04 (gcc/x86-64) —
  `cmake -S . -B build/desktop -DSRR2_BUILD_TESTS=OFF && cmake --build ...`
  works with no special flags. See `port-docs/desktop-build.md`.
- On macOS the game **boots and renders**: SDL window, OpenGL PDDI, Pure3D
  GUI. Verified visually to reach the license screen with official demo
  assets, then stop at `FileNotFound: frontend.p3d` — which is expected
  because the demo assets have no main menu (see "Demo assets" below).
- The prepared next step on macOS: run with the `skipfe` argument from the
  demo asset root to jump directly into Level 1 gameplay. **Not yet
  verified** — the user runs GUI sessions themselves (see "Working
  agreements").
- On Linux (Docker/Xvfb/llvmpipe harness) the game starts, initializes GL
  ("Mesa llvmpipe ... 4.5 Compatibility Profile"), but fails to find
  `licensePC.p3d` that demonstrably exists and is readable. **This is the
  open bug**; investigation state below.
- Branch **`main`** holds the earlier "Linux PoC" (`srr2-linux-poc`): a
  minimal harness that compiles a slice of original code against stubs. Its
  lasting value is the **asset-manifest / case-sensitivity probe** tool
  (`--data-root`, `--asset-manifest`). The full-game branch supersedes it for
  everything else.

## Repository Layout (zenoarrows-base)

- `code/` — original game code (GCC/Clang-portable, SDL-based platform layer).
- `libs/` — restored middleware: pure3d (PDDI GL/GLES/GXM/DX8 backends),
  radcore, radmath, radsound (OpenAL), radmovie (FFmpeg/Bink), radmusic,
  radscript, radcontent, choreo, poser, sim, scrooby (GUI).
- `CMakeLists.txt` — top-level; `code/CMakeLists.txt` builds the `SRR2` exe.
- `port-docs/` — all porting docs. `tools/linux-runtime/` — headless Linux
  run harness. `ci/github-build.yml` — CI workflow for both OSes (parked; see `ci/README.md` — activating it in `.github/workflows/` needs a push token with the `workflow` scope).
- Upstream provenance: vendored from ZenoArrows/The-Simpsons-Hit-and-Run @
  `07951a8d210c56eccbfd25e7756eedcbe7c8b850` (Switch/Vita port of the same
  leaked source; full game playable there). Upstream README:
  `port-docs/upstream-readme.md`. Evaluation and alternatives:
  `port-docs/high-level/pure3d-runtime-options.md`.

## Key Decisions And Why

1. **Adopted the ZenoArrows tree wholesale** instead of continuing the
   stub-first PoC or porting diffs incrementally. A structured diff showed
   527 changed game files are ~90% prerequisite compiler-portability fixes,
   inseparable from the middleware (generated sound scripts couple
   `code/sound` to `libs/radsound`; `Locator::SetUserData` couples meta to
   Pure3D). Their desktop SDL+OpenGL path is the default CMake config.
2. **SDL2, not SDL3**: upstream's SDL3 path has API drift in
   `libs/pure3d/pddi/gl/display_win32/gldisplay.cpp` (gamma ramp APIs removed,
   renamed constants, changed `SDL_GLContext` type). SDL3 is gated behind
   `-DSRR2_USE_SDL3=ON`; fixing that file would re-enable it.
3. **openal-soft over the macOS system OpenAL framework** (deprecated, lacks
   `efx.h`); CMake auto-prefers it on APPLE.
4. The game builds as `RAD_WIN32 + RAD_CONSOLE + RAD_SDL` on desktop — that is
   upstream's convention ("Win32-flavored console"), not a mistake. `RAD_PC`
   must stay **undefined** (it gates dead PC-only display/config code).

## Portability Fixes Applied (lessons)

All committed on `zenoarrows-base` with detailed messages:

- `malloc.h` does not exist on macOS → `__APPLE__ → stdlib.h` guards (9 files
  in libs + `code/memory/{memoryutilities,srrmemory}.cpp`).
- Clang rejects explicit specialization of `radLinkedClass<T>` statics after
  instantiation; GNU ld then hits strong/weak duplicate definitions. Fix:
  generic definitions in `libs/radcore/inc/radlinkedclass.hpp`, all per-type
  specializations removed (17 sites). Lesson: fix must be verified on BOTH
  compilers — the first (clang-only) version broke GCC.
- **Pre-main allocation crash (the big one)**: the game overrides global
  `operator new`, which lazily boots the whole memory system. On desktop,
  shared-library static initializers (FFmpeg's libx265) allocate during dyld
  init — before SDL is usable — crashing in `SDL_CreateMutex` /
  `SDL_SetThreadPriority`. Fixes in `libs/radcore/src/radthread/`:
  statically-initialized recursive pthread mutex for the internal lock,
  `pthread_self()` for thread identity, and the main-thread constructor
  records PriorityNormal without calling SDL. Consoles never see this
  (nothing allocates pre-main there). Any new desktop platform work must
  keep the pre-main path SDL-free.
- Missing `<cstddef>` for `NULL` in 3 actor-animation headers.

## Demo Assets (how runtime testing works without retail data)

The official free 2003 demo ("Special Edition for The Sun") provides real
retail-format assets, Level 1 only (missions 0-3 + races):

- Source: https://archive.org/details/hitandrun_demo → `hitandrun.iso`
  (661,723,136 bytes, sha256
  `225cc0305e7c5e93dd42c3b12683eb3b135938ad2e5cb639c7761fc4f18e2824`).
- Extraction: mount/untar the ISO (bsdtar works), then `unshield -d out x
  data1.cab` (InstallShield 6 cabs; `brew install unshield`).
- Assemble a game root: `art/`, `movies/`, `scripts/`, `sound/` dirs plus the
  seven `.rcf` files at the root (mapping recovered from `data1.hdr`; RCF
  names must match `code/sound/soundrenderer/soundrenderingmanager.cpp`).
- Current assembled copy (session-scoped scratchpad — REBUILD IT if gone):
  `/private/tmp/claude-502/-Users-michael-schmitt-projects-The-Simpsons-Hit-and-Run/86780f71-94ce-4acd-93b2-c522a27cf3b0/scratchpad/demo/gameroot`
- **Limitations**: no `art/frontend/scrooby/frontend.p3d` (no main menu — use
  the `skipfe` command-line option to jump straight into L1 gameplay; the
  original demo exe itself shipped with SKIPFE/SKIPMOVIE baked in);
  `burns_mansion_interior_loop.rsd` was deleted from `ambience.rcf` (a
  file-not-found near Burns Mansion is an asset gap, not a port bug).
- Full playability (all 7 levels) requires the user's retail PC assets; none
  were found on this machine.

## OPEN BUG: Linux cannot find files that exist

Symptom: in the Docker harness (`tools/linux-runtime/`), the game renders the
error screen `FileNotFound: licensePC.p3d` in an endless loop (log
`.../scratchpad/linux-run/game.log`), while the identical binary logic works
on macOS from the same asset tree.

Ruled out so far:

- The file exists at the exactly-matching case:
  `gameroot/art/frontend/dynaload/images/license/licensePC.p3d`.
- Not container permissions: the file lists and reads fine as root inside the
  same bind mount.
- GL works (llvmpipe compatibility profile 4.5) — rendering isn't the issue.

Path resolution facts (read `libs/radcore/src/radfile/sdl/sdldrive.cpp`):

- `BuildFileSpec` = `m_DrivePath` + filename with `\` → `/`; no case folding
  on open (macOS APFS masks case errors; Linux does not).
- `m_DrivePath` comes from `getcwd()` (SDL2 path) at drive-mount time, or
  from the drive spec **lowercased** (`SDL_strlwr`) when a request names a
  non-default drive. `radGetDefaultDrive` and the drive-spec parsing in
  `libs/radcore/src/radfile/` are the prime suspects: if game paths carry a
  drive spec (see `PROJECT_DRIVE_SPEC` in `code/presentation/gui/guisystem.cpp`
  line ~223) that mismatches the "default drive", the lowercased-drive-spec
  branch could produce a wrong root ON LINUX while APFS forgives it on macOS.

Next diagnostic step (command was prepared but not yet run): strace the game
in the container and look at the actual `openat` paths:

```sh
SCRATCH=<scratchpad>; docker run --rm --cap-add=SYS_PTRACE \
  -v "$SCRATCH/demo/gameroot:/assets" -v "$SCRATCH/linux-run:/out" srr2-linux-runtime bash -c '
apt-get update -qq >/dev/null 2>&1; apt-get install -y -qq strace >/dev/null 2>&1
export LIBGL_ALWAYS_SOFTWARE=1 ALSOFT_DRIVERS=null
cd /assets; Xvfb :99 -screen 0 640x480x24 >/dev/null 2>&1 & sleep 1; export DISPLAY=:99
strace -f -e trace=openat -o /out/strace.log timeout 8 /out/build/code/SRR2 skipfe >/dev/null 2>&1
grep -iE "p3d|rcf|spt" /out/strace.log | head -20'
```

(The harness image purges apt lists — `apt-get update` first is required.)
Whatever the wrong path turns out to be, prefer fixing the path composition
(drive spec / default drive) over adding case-folding; keep behavior identical
on macOS.

## Remaining Roadmap To "Fully Playable"

1. Fix the Linux path-resolution bug (above), then re-run the harness with
   `GAME_ARGS=skipfe` — success = game.log shows gameplay load, screenshot.xwd
   shows the world. (`RUN_SECONDS=120` was used; llvmpipe is slow, be patient.)
2. macOS gameplay: user runs `SRR2 skipfe` from the demo gameroot and reports
   (license screen was already verified). Watch for: FFmpeg .rmv playback,
   OpenAL device init, input mapping (radcore `sdlcontroller.cpp` handles
   gamepads; keyboard is a PC-only path that upstream compiled out — a
   keyboard input story may need reviving for desktop).
3. Sound/movies/input verification in gameplay; then mission completability
   (start → fail → restart → complete) on L1 with demo assets.
4. Full retail assets from the user → all levels, frontend menu
   (`frontend.p3d` exists there), save data, full mission sweep on both OSes.
5. Quality: SDL3 path fix (optional), GL 2.1 deprecation on macOS (works
   today; ANGLE/GLES2 via `-DSRR2_P3D_PDDI=GLES2` is the fallback), CI workflow is written but parked (see `ci/README.md`), packaging per `port-docs/high-level/porting-strategy.md` Phase 8.

## Working Agreements (important)

- **The user launches GUI apps on macOS themselves** and pastes screenshots;
  prepare exact commands for them instead of executing the game directly.
  Headless Docker runs are fine to execute autonomously.
- cmake lives at `/opt/homebrew/bin/cmake` (not on default PATH).
- Docker Desktop is available and working for Linux verification.
- Temporary/large files go to the session scratchpad, not the repo.

## Reference Docs

- `port-docs/desktop-build.md` — build/run instructions both OSes.
- `port-docs/high-level/pure3d-runtime-options.md` — middleware landscape,
  license notes, why ZenoArrows.
- `port-docs/high-level/porting-strategy.md`, `workstreams.md`,
  `challenges.md` — original phased plan (Phases 0-3 done, 4-6 largely
  satisfied by adoption, 7-8 remain).
- `port-docs/initial-poc-plan.md` — the pre-adoption PoC record (main branch
  tooling).

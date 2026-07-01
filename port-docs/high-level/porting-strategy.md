# Porting Strategy

## Summary

The port should proceed in phases that reduce unknowns early. The initial work should not attempt to make the game playable. It should prove that the code can compile and that the top-level loop can run under Linux. Once that is stable, the port can replace subsystems with real implementations.

## Phase 0: Repository And Build Scaffolding

Purpose: create a Linux build target without disturbing the original build files.

Deliverables:

- Root `CMakeLists.txt` or isolated Linux CMake entry.
- `RAD_LINUX` platform define.
- Minimal source list for the PoC target.
- Stub include directory that takes precedence only for Linux PoC builds.
- Documented build command.

Exit criteria:

- CMake configures on Linux.
- The target reaches compile errors in game code rather than missing build infrastructure.

## Phase 1: Non-Rendering Boot PoC

Purpose: prove native process startup, singleton lifetime, context flow, and fixed frame update.

Deliverables:

- `LinuxPlatform` no-op implementation.
- `linuxmain.cpp` entry point.
- Reduced singleton creation.
- No-op `RenderFlow`, `SoundManager`, and loading behavior.
- Fixed frame loop and clean exit.

Exit criteria:

- Native Linux binary runs without assets.
- Logs startup, frames, and shutdown.

## Phase 2: SDL Platform Shell

Purpose: replace no-op platform shell with a real Linux host window and input pump.

Deliverables:

- SDL2 window creation.
- Event polling.
- Keyboard and gamepad mapping skeleton.
- Real monotonic timer.
- Controlled shutdown from window close or escape key.

Exit criteria:

- Binary opens a window.
- Binary clears or presents frames.
- Input events reach the platform/input shim.

## Phase 3: Asset Filesystem And Loading

Purpose: make game asset requests visible and route them through a native filesystem abstraction.

Deliverables:

- Path normalization for Windows-style paths.
- Case-sensitivity handling strategy.
- Async or staged loading equivalent for `LoadingManager`.
- Logging for all requested files.
- Missing-file diagnostics with actionable output.

Exit criteria:

- The game can request asset files from a Linux directory tree.
- Loading errors identify exact missing files and source callers.

## Phase 4: Rendering Bring-Up

Purpose: replace no-op rendering with enough visible output to validate camera/frame flow.

Options:

- Implement a compatibility layer for the subset of Pure3D/PDDI used by the game.
- Convert Pure3D assets to an intermediate format and build a new renderer.
- Integrate an existing Pure3D-compatible implementation if legally and technically viable.

Recommended first rendering target:

- Clear screen.
- Draw debug text.
- Draw simple colored geometry.
- Load one static mesh or test asset.
- Connect camera transforms.

Exit criteria:

- A Linux window presents frames through the game loop.
- At least one world or test object renders through the chosen pipeline.

## Phase 5: Input And Gameplay Interaction

Purpose: make player input reach the original gameplay control path.

Deliverables:

- SDL keyboard/gamepad mapping to the existing `InputManager` concepts.
- Controller connection model.
- Rumble no-op or SDL haptics implementation.
- Frontend/gameplay input mode transitions.

Exit criteria:

- Input can drive a test mappable or player controller path.

## Phase 6: Audio And Movies

Purpose: restore sound effects, music, dialog, and FMV behavior.

Deliverables:

- Replacement sound renderer interface.
- Streaming music support.
- Positional sound approximation.
- Dialog playback coordination.
- Bink or replacement FMV handling strategy.

Exit criteria:

- Frontend/menu sounds play.
- One gameplay sound effect plays from a game event.
- FMV calls either play video or degrade cleanly.

## Phase 7: Full Gameplay Stabilization

Purpose: make real missions and levels playable.

Deliverables:

- Real asset loading coverage.
- Physics and simulation compatibility.
- Mission script loading.
- GUI and HUD behavior.
- Save data handling.
- Crash and leak tracking.

Exit criteria:

- One level loads.
- One mission can be started, played, failed, restarted, and completed.

## Phase 8: Polish, Packaging, And Tooling

Purpose: make the port usable outside a developer checkout.

Deliverables:

- Runtime asset packaging.
- Config files.
- Logging controls.
- Controller remapping.
- Build presets.
- CI build for Linux.
- Developer documentation.

Exit criteria:

- A fresh Linux checkout can configure, build, and run with documented data inputs.

## Architecture Recommendation

Keep three layers separate:

| Layer | Responsibility |
| --- | --- |
| Original game code | Gameplay, missions, world simulation, context flow. |
| Compatibility layer | RAD/Pure3D/Scrooby/PDDI-like interfaces used by game code. |
| Native backend | SDL, renderer, audio, filesystem, threading, and packaging. |

This separation prevents early PoC hacks from becoming permanent engine design.

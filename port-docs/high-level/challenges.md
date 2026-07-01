# Porting Challenges

## Missing Middleware

The largest technical blocker is the missing middleware stack. The game references RAD libraries, Pure3D, PDDI, Scrooby, Poser, Choreo, Sim, Bink, EAX, and platform SDKs.

Impact:

- The original project cannot compile without replacements or recovered compatible libraries.
- Many game systems are coupled directly to middleware types.
- A large compatibility surface may be needed before gameplay can run.

Mitigation:

- Start with no-op shims.
- Implement only the API surface used by the current milestone.
- Keep compatibility code isolated.

## Runtime Data And Assets

The checkout does not include `game/cd`, `game/libs`, or large art export folders. The executable expects runtime data.

Impact:

- A playable port cannot be validated without usable asset data.
- Loading code references original Windows-style paths.
- Asset formats may require Pure3D tooling or reverse engineering.

Mitigation:

- Make the first PoC asset-free.
- Add missing-file logging before implementing real loaders.
- Decide early whether assets will stay packed, be unpacked, or be converted.

## Legal And Licensing Constraints

Only proceed with code, assets, libraries, and SDKs that you have rights to use. Middleware and game assets may carry separate licensing constraints.

Impact:

- Original proprietary libraries may not be redistributable.
- Replacement libraries may impose new license obligations.
- Asset distribution may be more constrained than source compilation.

Mitigation:

- Document dependency provenance.
- Prefer clean replacement implementations where possible.
- Keep porting code separate from proprietary binaries.

## Platform Assumptions

The codebase has platform-specific branches for Win32, PS2, Xbox, and GameCube. Linux has no existing branch.

Impact:

- Required PC behavior may be hidden under `RAD_WIN32`.
- Defining `RAD_WIN32` on Linux would pull in Windows APIs.
- Defining only `RAD_PC` may leave behavior missing.

Mitigation:

- Add `RAD_LINUX`.
- Audit `#ifdef RAD_WIN32` usage and split PC behavior from Windows behavior.
- Keep platform-specific changes narrow and explicit.

## Case-Sensitive Filesystem

Linux paths are case-sensitive. Original includes and asset paths may rely on Windows case-insensitive behavior.

Impact:

- Includes may fail even when files exist.
- Runtime asset requests may fail due to case mismatches.

Mitigation:

- Add include path checks during build bring-up.
- Normalize asset paths.
- Generate a case mismatch report for requested files.

## Path Separators

The code and scripts use Windows-style backslashes in many asset paths.

Impact:

- Native file lookup will fail unless paths are normalized.
- Logs may contain mixed path styles.

Mitigation:

- Normalize path separators at the filesystem abstraction boundary.
- Avoid rewriting every string literal early.

## Old C++ And Compiler Differences

The code targets old MSVC and console compilers. Modern GCC/Clang will reject or warn on some constructs.

Impact:

- MSVC extensions may fail.
- Header ordering and missing includes may surface.
- Old standard library behavior may differ.

Mitigation:

- Start with permissive flags only for the PoC.
- Fix portability issues incrementally.
- Add stricter warnings after milestone stability.

## Custom Memory System

The game uses custom heaps, allocator IDs, overloaded `new/delete`, and heap stack routing.

Impact:

- Naive allocation replacement may compile but hide lifecycle bugs.
- Original code may assume heap identity and lifetime.
- Multithreaded allocation behavior may differ.

Mitigation:

- Preserve allocator IDs as logical labels.
- Route to host memory first.
- Add diagnostics before recreating heap constraints.

## Rendering Scope

Rendering is not a small subsystem. Pure3D/PDDI touches assets, materials, cameras, render layers, animation, particles, shadows, and platform extensions.

Impact:

- Full rendering can dominate the port schedule.
- Asset format support is tied to renderer bring-up.
- Debugging gameplay without visual output is difficult after early phases.

Mitigation:

- Bring up rendering in levels: clear screen, debug text, simple geometry, static mesh, then real world data.
- Keep renderer API behind compatibility boundaries.
- Use debug overlays and asset request logs early.

## Audio Scope

The audio system includes music state, dialog, positional sound, event ducking, sound scripts, and FMV audio.

Impact:

- Silent gameplay can run, but many missions and presentation beats expect audio events.
- Dialog timing may affect mission flow.

Mitigation:

- Start no-op but trigger completion callbacks.
- Implement sound event logging before playback.
- Add playback categories incrementally.

## UI And Scrooby

Frontend, HUD, language, boot screens, and menus depend on Scrooby and presentation systems.

Impact:

- Boot and loading contexts may pull in UI dependencies early.
- Gameplay may be hard to test without HUD or menu flow.

Mitigation:

- Stub GUI in the first PoC.
- Add debug keyboard shortcuts for state transitions.
- Render minimal debug UI before full frontend support.

## Lack Of Automated Tests

The original code relies on runtime behavior and debug assertions rather than modern automated tests.

Impact:

- Refactors and shims may break behavior silently.
- Regression detection will be difficult.

Mitigation:

- Add smoke tests around boot and fixed-frame execution.
- Add deterministic frame mode.
- Add event/load/render logs that can be diffed.

## Performance And Timing

Original timing assumptions were tied to old consoles and DirectX-era PC hardware.

Impact:

- Physics and gameplay may depend on frame timing behavior.
- Loading and async service order may matter.

Mitigation:

- Use fixed timestep in early PoCs.
- Preserve service order from `Game::Run()`.
- Add timing diagnostics before optimizing.

## Toolchain And Asset Pipeline

The `tools` tree references old Visual Studio projects, Pure3D tools, Maya 4.0 plugins, and art pipeline scripts.

Impact:

- Rebuilding assets natively may be harder than running the game.
- Old tools may require Windows and obsolete SDKs.

Mitigation:

- Separate runtime porting from asset-tool porting.
- Prefer consuming existing processed game data for early runtime work.
- Only port tools that are required for the chosen asset strategy.

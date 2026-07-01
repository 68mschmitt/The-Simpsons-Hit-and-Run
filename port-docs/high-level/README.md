# High-Level Port Overview

The Linux port should be treated as a staged replacement of old platform and middleware dependencies, not as a single build-system conversion.

The fastest useful path is:

1. Compile a narrow native Linux boot loop with shims.
2. Add a basic SDL2 window, input, and frame pump.
3. Replace no-op subsystems with real Linux-backed services one at a time.
4. Bring up asset loading and rendering before gameplay completeness.
5. Stabilize gameplay, UI, sound, save data, and tooling last.

## Current Phase Status

As of 2026-07-01, step 1 has an initial scaffold: CMake configures, `srr2-linux-poc` builds, and the binary runs a fixed-frame no-render loop through Linux-specific platform and singleton stubs. It is still isolated from the original game loop and middleware-heavy systems.

Next recommended increment: compile a slightly larger vertical slice by replacing the local `LinuxPocGame` loop with selected original `Game`/`GameFlow` code plus PoC context, render, sound, input, and loading shims.

## High-Level Documents

- [Porting Strategy](porting-strategy.md)
- [Workstreams](workstreams.md)
- [Challenges](challenges.md)

## Strategic Constraint

The original codebase was built around proprietary technology and platform SDKs that are not present in this repo. A complete port requires either recovered compatible middleware or replacement implementations.

## Recommended North Star

Use the original gameplay code where practical, but replace the runtime shell:

- SDL2 or equivalent for windowing, input, and timing.
- OpenGL, Vulkan, or a translation layer for rendering.
- OpenAL, SDL audio, miniaudio, or FMOD-like abstraction for audio.
- Native POSIX filesystem support for data access.
- CMake for Linux builds.
- Lightweight compatibility shims for RAD/Pure3D interfaces during transition.

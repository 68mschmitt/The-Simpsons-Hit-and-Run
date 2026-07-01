# High-Level Port Overview

The Linux port should be treated as a staged replacement of old platform and middleware dependencies, not as a single build-system conversion.

The fastest useful path is:

1. Compile a narrow native Linux boot loop with shims.
2. Add a basic SDL2 window, input, and frame pump.
3. Replace no-op subsystems with real Linux-backed services one at a time.
4. Bring up asset loading and rendering before gameplay completeness.
5. Stabilize gameplay, UI, sound, save data, and tooling last.

## Current Phase Status

As of 2026-07-01, step 1 has progressed beyond the initial scaffold: CMake configures, `srr2-linux-poc` builds, and the binary runs a fixed-frame no-render loop through the original `Game` singleton, `GameFlow`, and base `Context` update path. Heavyweight contexts and middleware systems are still represented by Linux PoC shims.

Next recommended increment: replace one shim at a time with a wider original-code or native-backed slice, starting with a basic SDL2 platform shell or a more faithful loading/filesystem shim.

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

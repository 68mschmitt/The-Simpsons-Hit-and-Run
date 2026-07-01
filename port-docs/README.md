# Linux Port Documentation

This directory captures a pragmatic plan for exploring a Linux-native port of this codebase.

The repository is not currently buildable on Linux as a normal application. The original project expects old Windows and console toolchains, proprietary middleware, and runtime data that are not present in this checkout. The fastest useful porting path is therefore to build a small native proof of concept first, using local shims for the missing platform and middleware layers.

## Current State

As of 2026-07-01, the initial Linux-native PoC scaffold exists and builds.

Implemented pieces:

- Root CMake entry point and `cmake/linux-poc.cmake`.
- `srr2-linux-poc` executable target.
- `RAD_LINUX`, `RAD_PC`, `_DEBUG`, and `LINUX_POC` compile definitions.
- `game/code/main/linuxmain.cpp` with fixed-frame startup/update/shutdown logging.
- `game/code/main/linuxplatform.{h,cpp}` no-window platform stub.
- `game/code/main/singletons_linux_poc.{h,cpp}` reduced singleton lifecycle logging.
- Minimal RAD/Pure3D/PDDI shim headers in `game/code/port/stubs/`.
- Linux command-line compatibility fixes in `commandlineoptions.cpp` and `globaltypes.h`.

Validated commands:

```sh
cmake -S . -B build/linux-poc
cmake --build build/linux-poc -j2
./build/linux-poc/srr2-linux-poc --frames 2 skipfe
```

The current PoC is intentionally not yet wired into the original `Game`, `GameFlow`, context, render, sound, input, or loading implementations. It proves that an isolated Linux target can configure, compile, run a deterministic no-render boot loop, and exit cleanly without assets or proprietary middleware.

## Documents

- [Initial PoC Plan](initial-poc-plan.md): shortest path to a Linux-native executable that boots a reduced game loop.
- [High-Level Port Overview](high-level/README.md): overall approach and phase map.
- [Porting Strategy](high-level/porting-strategy.md): recommended architecture and port phases.
- [Workstreams](high-level/workstreams.md): major areas of implementation work.
- [Challenges](high-level/challenges.md): known technical and project risks.

## Guiding Principle

Do not start by porting the whole game. Start by compiling and driving a narrow vertical slice natively on Linux. Replace missing middleware with no-op or logging shims, then progressively make those shims real.

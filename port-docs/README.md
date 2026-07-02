# Linux Port Documentation

This directory captures a pragmatic plan for exploring a Linux-native port of this codebase.

The repository is not currently buildable on Linux as a normal application. The original project expects old Windows and console toolchains, proprietary middleware, and runtime data that are not present in this checkout. The fastest useful porting path is therefore to build a small native proof of concept first, using local shims for the missing platform and middleware layers.

## Current State

As of 2026-07-01, the Linux-native PoC scaffold exists, builds, drives a reduced slice of the original `Game` / `GameFlow` path, and can optionally host that loop in an SDL2 window.

Implemented pieces:

- Root CMake entry point and `cmake/linux-poc.cmake`.
- `srr2-linux-poc` executable target.
- Optional SDL2 discovery/linking via `SRR2_LINUX_POC_WITH_SDL`.
- `RAD_LINUX`, `RAD_PC`, `_DEBUG`, and `LINUX_POC` compile definitions, plus `LINUX_POC_WITH_SDL` when SDL2 is found.
- `game/code/main/linuxmain.cpp` that creates the original `Game` singleton and runs `Game::Initialize()`, `Game::Run()`, and `Game::Terminate()`.
- Linux PoC command-line options: `--frames`, `--until-quit`, `--headless`, `--window`, and `--window-size WxH`.
- `game/code/main/game.cpp` Linux PoC fixed-frame branch using the original service order while pumping host events and presenting host frames.
- `game/code/gameflow/gameflow.cpp` compiled with PoC context wiring under `LINUX_POC`.
- `game/code/contexts/linux_poc_contexts.{h,cpp}` logging contexts for all `ContextEnum` values.
- `game/code/main/linuxplatform.{h,cpp}` Linux platform shell that runs headless or creates an SDL2 window, clears/presents frames, polls window/keyboard/controller events, routes keyboard/controller button state into the input shim, and requests quit on close/Escape.
- `game/code/main/singletons_linux_poc.{h,cpp}` reduced singleton lifecycle that creates input, render, loading, and sound shims.
- Minimal RAD/Pure3D/PDDI, memory, input, render, sound, and loading shim headers/implementations in `game/code/port/`.
- Linux command-line compatibility fixes in `commandlineoptions.cpp` and `globaltypes.h`.

Validated commands:

```sh
cmake -S . -B build/linux-poc
cmake --build build/linux-poc -j2
./build/linux-poc/srr2-linux-poc --frames 2 skipfe
./build/linux-poc/srr2-linux-poc --headless --frames 1
SDL_VIDEODRIVER=dummy ./build/linux-poc/srr2-linux-poc --window --window-size 320x200 --frames 1
```

The current PoC is intentionally still non-playable and does not render game content. It proves that the original `Game` singleton, `GameFlow` singleton, base `Context` update path, reduced context transitions, shimmed render/sound/input/loading services, and an SDL2-backed host shell can configure, compile, run a deterministic boot loop, present clear-screen frames when a host window is available, pump basic host input, and exit cleanly without assets or proprietary middleware.

## Documents

- [Initial PoC Plan](initial-poc-plan.md): shortest path to a Linux-native executable that boots a reduced game loop.
- [High-Level Port Overview](high-level/README.md): overall approach and phase map.
- [Porting Strategy](high-level/porting-strategy.md): recommended architecture and port phases.
- [Workstreams](high-level/workstreams.md): major areas of implementation work.
- [Challenges](high-level/challenges.md): known technical and project risks.

## Guiding Principle

Do not start by porting the whole game. Start by compiling and driving a narrow vertical slice natively on Linux. Replace missing middleware with no-op or logging shims, then progressively make those shims real.

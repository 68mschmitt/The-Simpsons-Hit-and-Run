# Initial Linux-Native PoC Plan

## Goal

Create a native Linux executable that proves the original C++ game code can be compiled, linked, started, and driven through a minimal frame loop without the original Windows, console, RAD, or Pure3D runtime stack.

The first PoC is allowed to be non-rendering and non-playable. It should boot, run a small number of frames, log context/update activity, and exit cleanly.

## Target Result

The PoC should produce a binary similar to:

```text
build/linux-poc/srr2-linux-poc
```

Expected behavior:

```text
LinuxPlatform initialized
CreateSingletonsLinuxPoc
Game initialized
GameFlow context: ENTRY -> BOOTUP
Frame 1
Frame 2
...
Frame 300
Shutdown complete
```

## Implementation Status

Status as of 2026-07-01: initial scaffold, first original-code vertical slice, and an optional SDL2 host shell implemented and validated.

Build/run commands:

```sh
cmake -S . -B build/linux-poc
cmake --build build/linux-poc -j2
./build/linux-poc/srr2-linux-poc --frames 2 skipfe
./build/linux-poc/srr2-linux-poc --headless --frames 1
SDL_VIDEODRIVER=dummy ./build/linux-poc/srr2-linux-poc --window --window-size 320x200 --frames 1
```

Current behavior:

- Configures and builds `build/linux-poc/srr2-linux-poc`.
- Parses PoC options such as `--frames N`, `--until-quit`, `--headless`, `--window`, and `--window-size WxH`.
- Passes other arguments to `CommandLineOptions::HandleOption` after stripping leading dashes.
- Initializes `LinuxPlatform` through `Game::Initialize()` in either headless mode or an SDL2 host-window mode.
- Runs `CreateSingletonsLinuxPoc()` / `DestroySingletonsLinuxPoc()` to create reduced input, render, loading, and sound services.
- Creates the original `Game` singleton from `linuxmain.cpp`.
- Compiles and drives the original `Game::Initialize()`, Linux-PoC `Game::Run()` branch, `Game::Terminate()`, `GameFlow`, and base `Context` update path.
- Uses PoC contexts for all `ContextEnum` values, with fake bootup loading completion.
- Runs a deterministic fixed-frame loop using `LinuxPocConfig::FixedElapsedMilliseconds`, or an explicit `--until-quit` loop.
- When SDL2 is available and not disabled, creates a host window, clears/presents one frame per game-loop tick, polls SDL window/keyboard/controller events, routes keyboard and controller button state into the input shim, and exits on window close or Escape.
- Logs `CONTEXT_ENTRY -> CONTEXT_BOOTUP`, each fixed frame, `CONTEXT_BOOTUP -> CONTEXT_EXIT`, and clean shutdown.

Important limitation: the current target still does not compile or drive the original heavyweight bootup/frontend/gameplay contexts, real `RenderFlow`, real `SoundManager`, real `InputManager`, or real `LoadingManager`. Those are represented by PoC shims so the original top-level game and flow lifecycles can run without assets or proprietary middleware. The SDL2 shell only clears/presents a host window; it does not render game content.

## Non-Goals

- No real rendering.
- No real sound.
- No movie playback.
- No Scrooby GUI.
- No real `.p3d` asset loading.
- No mission gameplay.
- No attempt to build the original Visual Studio solutions on Linux.
- No attempt to preserve binary compatibility with the original executable.

## Why This Is The Quickest Path

The original code depends on missing proprietary systems:

- RAD core, file, memory, time, debug, controller, sound, music, movie, and script libraries.
- Pure3D and PDDI rendering.
- Scrooby UI.
- Poser, Choreo, Sim, and other gameplay/render middleware.
- DirectX-era Win32 runtime libraries.
- Runtime game data and asset folders under `game/cd`.

A direct full build would require reconstructing all of that first. A shim-first PoC lets the port prove core compilation and runtime control before the hard systems are implemented.

## Recommended First Milestone

Build a reduced Linux target that includes only:

- A new Linux entry point.
- A new Linux `Platform` implementation.
- A minimal `Game` loop or lightly modified `Game::Run()` path.
- `GameFlow` and enough contexts to exercise context transition.
- Stub `RenderFlow`.
- Stub `SoundManager`.
- Stub loading completion.
- Minimal RAD/Pure3D type shims needed to compile the selected files.

## Proposed Files

New files:

```text
game/code/main/linuxmain.cpp
game/code/main/linuxplatform.h
game/code/main/linuxplatform.cpp
game/code/main/singletons_linux_poc.cpp
game/code/port/linux_poc_config.h
game/code/port/stubs/README.md
game/code/port/stubs/raddebug.hpp
game/code/port/stubs/radtime.hpp
game/code/port/stubs/radmemory.hpp
game/code/port/stubs/radfile.hpp
game/code/port/stubs/p3d_compat.hpp
CMakeLists.txt
cmake/linux-poc.cmake
```

The exact names can change, but the important idea is to keep PoC-only scaffolding isolated from the original platform files.

## Build System

Use CMake for the Linux PoC. Do not translate the whole Visual Studio project at first.

Initial target shape:

```cmake
add_executable(srr2-linux-poc
    game/code/main/linuxmain.cpp
    game/code/main/linuxplatform.cpp
    game/code/main/game.cpp
    game/code/gameflow/gameflow.cpp
    game/code/contexts/context.cpp
    game/code/contexts/entrycontext.cpp
    game/code/contexts/bootupcontext.cpp
    game/code/main/singletons_linux_poc.cpp
)
```

That file list will need adjustment as compile errors reveal dependencies. Keep the list small. Every file added should be justified by the current milestone.

Suggested compile definitions:

```text
RAD_LINUX
RAD_PC
_DEBUG
LINUX_POC
```

Avoid defining `RAD_WIN32`, `RAD_PS2`, `RAD_XBOX`, or `RAD_GAMECUBE` for the native target.

## Step 1: Linux Entry Point

Add `linuxmain.cpp` that mirrors the high-level structure of `win32main.cpp`, but omits Win32-specific initialization.

Responsibilities:

- Initialize command line defaults.
- Parse simple command line options using `CommandLineOptions::HandleOption`.
- Initialize minimal memory/time/debug shims.
- Create reduced singletons.
- Create `LinuxPlatform`.
- Create `Game`.
- Run for a fixed frame count or until `Game::Stop()`.
- Destroy objects in reverse order.

The first version should avoid loading `command.txt` and avoid platform UI behavior.

## Step 2: Linux Platform Stub

Implement `LinuxPlatform : public Platform`.

Initial behavior:

- `InitializePlatform()` logs and returns.
- `ShutdownPlatform()` logs and returns.
- `LaunchDashboard()` sets a quit flag or logs only.
- `ResetMachine()` logs only.
- `DisplaySplashScreen()` logs only.
- `OnControllerError()` logs only.
- `OnDriveError()` returns false.
- `InitializeFoundationDrive()`, `InitializePure3D()`, and related platform methods are no-ops.

This platform should not create a window yet. A window belongs in the second PoC milestone.

## Step 3: Middleware Shims

Create small compatibility headers for missing middleware. Start with the absolute minimum needed to compile the selected files.

Stub categories:

| Area | First PoC Behavior |
| --- | --- |
| `raddebug.hpp` | Map asserts and debug prints to standard C/C++ output. |
| `radtime.hpp` | Return monotonic milliseconds and microseconds. |
| `radmemory.hpp` | Route allocations to normal `new`, `delete`, `malloc`, or `free`. |
| `radfile.hpp` | Provide empty service functions and placeholder interfaces. |
| `p3d/*` | Provide null global pointers or tiny structs only where referenced. |
| `pddi/*` | Provide placeholder display and color types. |
| `radsound*` | No-op service and manager interfaces. |

Keep shims intentionally small. Do not reimplement full middleware semantics during the first PoC.

## Step 4: Reduced Singletons

The original `singletons.cpp` creates almost every gameplay system. That is too much for the first PoC.

Create a PoC-specific singleton file that only initializes what the boot loop truly needs.

Likely first version:

```text
CreateSingletonsLinuxPoc()
DestroySingletonsLinuxPoc()
```

These can initially create only stub versions of:

- `RenderFlow`
- `SoundManager`
- `EventManager`, if needed by selected contexts
- `LoadingManager`, if selected context code touches loading

If `BootupContext` pulls in too many dependencies, introduce a simpler PoC context or compile-time gate under `LINUX_POC`.

## Step 5: Fake Loading

The first PoC should not read assets.

For PoC builds, make loading calls behave like this:

- Log the requested filename.
- Do not open the file.
- Immediately call the completion callback.
- Keep `IsLoading()` false after callback completion.

This proves flow control without requiring `game/cd`, cement libraries, Pure3D chunks, or Scrooby resources.

## Step 6: Stub Render And Sound

The first `RenderFlow` can be a no-op implementation:

```text
RenderFlow::CreateInstance()
RenderFlow::GetInstance()
RenderFlow::DestroyInstance()
RenderFlow::DoAllRegistration()
RenderFlow::OnTimerDone()
```

The first `SoundManager` can be a no-op implementation:

```text
SoundManager::CreateInstance(...)
SoundManager::GetInstance()
SoundManager::DestroyInstance()
SoundManager::Update()
SoundManager::UpdateOncePerFrame(...)
```

If the original concrete files are too dependency-heavy, use PoC-only implementations with the same public interfaces.

## Step 7: Fixed Frame Loop

Use a deterministic loop for the first milestone.

Example behavior:

```text
for 300 frames:
    elapsedMs = 16
    timerList->Service if available
    GameFlow::OnTimerDone(elapsedMs, nullptr)
    RenderFlow::OnTimerDone(elapsedMs, nullptr)
    SoundManager::Update()
```

Avoid real-time scheduling until the binary is stable.

## Step 8: Clean Exit

The first PoC must prove shutdown order. This is important because the original code relies heavily on singleton lifetime and custom allocation.

Exit criteria:

- No crash on startup.
- No crash after fixed frame count.
- `DestroySingletonsLinuxPoc()` completes.
- `LinuxPlatform::ShutdownPlatform()` completes.
- Process returns `0`.

## Expected Compile Problems

Expect these immediately:

- Case-sensitive include path mismatches on Linux.
- Backslash file paths in string literals and build scripts.
- Missing `stdafx` or forced precompiled header assumptions.
- MSVC-specific syntax and pragmas.
- Old C++ standard library assumptions.
- Platform-specific `#ifdef RAD_WIN32` branches hiding required PC behavior.
- Missing declarations from middleware headers.
- Custom allocator overload interactions.

Fix these narrowly. Prefer shim headers and PoC guards over broad rewrites.

## First Success Criteria

The initial PoC is successful when all of these are true:

- `cmake -S . -B build/linux-poc` configures.
- `cmake --build build/linux-poc` produces `srr2-linux-poc`.
- Running the binary logs startup and frame updates.
- The binary exits cleanly after a fixed frame count.
- No real assets or proprietary middleware are required.

## Next Milestone After This PoC

The SDL2-backed platform shell now exists at a minimal level:

- Creates a window when SDL2 is available and a display is available or `--window` is passed.
- Polls window, keyboard, and controller events.
- Clears/presents the host window once per frame.
- Keeps the game update loop on the deterministic PoC timestep.

Next recommended increment: make the loading/filesystem shim more faithful by normalizing paths, recording all asset requests, probing an optional data root, and reporting missing files with case-sensitivity diagnostics. Do not load real assets until the SDL shell and no-op runtime are stable.

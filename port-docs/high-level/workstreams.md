# Port Workstreams

## Build System

Scope:

- Add CMake or another Linux-native build system.
- Keep original Visual Studio and console build files intact.
- Start with small PoC targets and grow source coverage intentionally.
- Add build presets for debug, sanitizer, and release configurations.

Key work:

- Normalize include paths.
- Handle case-sensitive file names.
- Remove or gate MSVC-only pragmas.
- Decide C++ standard level.
- Add compiler warning policy after the code compiles.

## Platform Layer

Scope:

- Replace Win32, Xbox, PS2, and GameCube startup code with `RAD_LINUX` startup.
- Implement the `Platform` interface for Linux.
- Provide timing, window, input, and shutdown behavior.

Key work:

- `linuxmain.cpp`.
- `LinuxPlatform`.
- SDL event pump.
- Graceful quit path.
- Runtime config loading.

## Middleware Compatibility

Scope:

- Provide headers and implementation shims for missing RAD and Pure3D APIs.
- Start with no-op functions, then replace with real behavior.

Key work:

- RAD debug/assert/logging.
- RAD time.
- RAD memory and allocator behavior.
- RAD file interfaces.
- RAD controller interfaces.
- RAD sound/music/movie placeholders.
- Pure3D/PDDI object and render abstractions.
- Scrooby GUI placeholders.

## Memory System

Scope:

- Preserve enough allocator semantics for original code expectations.
- Avoid overfitting to original console heap layout during early PoC.

Key work:

- Map `GameMemoryAllocator` to host allocations.
- Preserve placement `new` overloads.
- Track heap labels for diagnostics.
- Add optional leak logging.
- Revisit fixed heaps only if gameplay requires them.

## Filesystem And Assets

Scope:

- Replace console/Windows file access with Linux path handling.
- Support original data layout or define a new normalized layout.

Key work:

- Convert backslashes to slashes.
- Handle case sensitivity.
- Log missing file requests.
- Decide whether to keep original cement libraries or unpack assets.
- Implement async loading compatibility.

## Rendering

Scope:

- Replace Pure3D/PDDI runtime behavior with a Linux renderer or compatibility layer.

Key work:

- Window and graphics context.
- Shader/material translation.
- Texture upload.
- Mesh loading.
- Animation support.
- Scene graph or render layer mapping.
- Camera and split-screen support.
- Debug overlays.

Potential backends:

- OpenGL for fastest bring-up.
- Vulkan for long-term explicit control.
- bgfx or similar abstraction if project goals allow another dependency.

## Audio

Scope:

- Replace RADSound, RADMusic, and Bink audio integration.

Key work:

- Sound effect playback.
- Music streaming.
- Dialog playback and coordination.
- Positional audio.
- Volume groups and ducking.
- Surround mode degradation or replacement.

Potential backends:

- SDL audio.
- miniaudio.
- OpenAL Soft.
- FMOD-like abstraction if licensing permits.

## Input

Scope:

- Map Linux keyboard, mouse, and gamepad input to the existing input manager model.

Key work:

- Keyboard mapping.
- Controller mapping.
- Mouse frontend support.
- Hotplug events.
- Rumble support or no-op fallback.
- Configurable bindings.

## UI And Frontend

Scope:

- Replace or emulate Scrooby and frontend behavior.

Key work:

- Determine Scrooby file format support.
- Stub GUI for early gameplay bring-up.
- Render menus and HUD.
- Text/font handling.
- Localization asset handling.

## Gameplay And Mission Systems

Scope:

- Preserve original gameplay logic once platform, loading, and rendering are stable enough.

Key work:

- Mission script loading.
- Mission state transitions.
- Dynamic zone loading.
- Traffic and pedestrian managers.
- Vehicle and character controllers.
- Trigger volumes and events.
- Save/load state.

## Tooling And Asset Pipeline

Scope:

- Decide whether to port old asset tools, replace them, or avoid them for runtime-only goals.

Key work:

- Pure3D conversion tools.
- Maya 4.0 plugin replacement strategy.
- World builder and track editor relevance.
- Build scripts for assets.
- Asset validation tools.

## Testing And Diagnostics

Scope:

- Add modern diagnostics where the original code has little automated testing.

Key work:

- Startup smoke test.
- Fixed-frame deterministic test mode.
- Missing asset report.
- Sanitizer builds.
- Crash logging.
- Frame timing logs.
- Event trace logs.

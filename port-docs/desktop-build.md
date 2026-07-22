# Desktop Build (Linux / macOS)

`main` builds the full game (`SRR2`) via the adopted port tree: original game
code under `code/`, restored middleware under `libs/`, and one root
`CMakeLists.txt`. The desktop SDL + OpenGL path is the default CMake
configuration (`RAD_WIN32 + RAD_CONSOLE + RAD_SDL`, `SRR2_P3D_PDDI=OpenGL`).
Keep `RAD_PC` undefined.

## Linux

Use the [README Linux quickstart](../README.md#linux-quickstart) for the
supported user install (`cmake --install ... --prefix "$HOME/.local"`) and
current Ubuntu/Debian, Arch, and Fedora package commands. Use
`tools/linux-runtime/dev-run.sh --data-dir /path/to/assets -- skipfe` for the
source-tree configure/build/run loop. The complete asset, launcher, and Xvfb
troubleshooting reference is [linux-native-port.md](linux-native-port.md).

Validated 2026-07-18 with retail PC assets on Linux under Xvfb/llvmpipe.

## macOS

Dependencies (Homebrew): `cmake sdl2 libpng openal-soft ffmpeg`

```sh
cmake -S . -B build/desktop -DCMAKE_BUILD_TYPE=Release -DSRR2_BUILD_TESTS=OFF -DSRR2_USE_PCH=OFF
cmake --build build/desktop -j8
```

Notes:

- SDL2 is the default; SDL3 is opt-in via `-DSRR2_USE_SDL3=ON` because the
  SDL3 code path has API drift in
  `libs/pure3d/pddi/gl/display_win32/gldisplay.cpp` (gamma ramp, renamed
  constants). Fixing that would allow SDL3.
- On macOS, CMake automatically prefers Homebrew openal-soft over the deprecated
  system OpenAL framework (which lacks `efx.h`).
- The OpenGL PDDI backend requests a legacy compatibility context; macOS
  provides GL 2.1, which is deprecated but functional. If it proves problematic,
  `-DSRR2_P3D_PDDI=GLES2` via ANGLE is the fallback.

## Running

On Linux, configure a lawful retail PC install once and use the launcher; it
validates the case-sensitive asset tree and changes into it before starting the
game:

```sh
srr2 --set-data-dir "/path/to/The Simpsons - Hit & Run"
srr2
srr2 skipfe
```

The full installation includes retail RCF archives such as `dialog.rcf`,
`music00.rcf`-`music03.rcf`, `ambience.rcf`, `carsound.rcf`, `nis.rcf`,
`scripts.rcf`, and `soundfx.rcf`. For macOS, run the built executable with its
working directory set to the retail PC asset root. Keep Linux asset lookup
case-sensitive and deterministic; do not add case-folding as a first response
to missing files.

## Provenance

Upstream base: ZenoArrows/The-Simpsons-Hit-and-Run @ `07951a8d` (Switch/Vita
port, full game playable). See
`port-docs/high-level/pure3d-runtime-options.md` for the evaluation and
`port-docs/upstream-readme.md` for the upstream README.

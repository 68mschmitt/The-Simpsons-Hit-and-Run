# Desktop Build (Linux / macOS)

`main` builds the full game (`SRR2`) via the adopted port tree: original game
code under `code/`, restored middleware under `libs/`, and one root
`CMakeLists.txt`. The desktop SDL + OpenGL path is the default CMake
configuration (`RAD_WIN32 + RAD_CONSOLE + RAD_SDL`, `SRR2_P3D_PDDI=OpenGL`).
Keep `RAD_PC` undefined.

## Linux

Dependencies (apt): `cmake libsdl2-dev libpng-dev libopenal-dev` plus FFmpeg dev
packages (`libavformat-dev libavcodec-dev libavutil-dev libswresample-dev
libswscale-dev`) and `zlib1g-dev`.

```sh
cmake -S . -B build/native -DCMAKE_BUILD_TYPE=Release -DSRR2_BUILD_TESTS=OFF -DSRR2_USE_PCH=OFF
cmake --build build/native -j$(nproc)
```

Validated 2026-07-18 with retail PC assets on Linux under Xvfb/llvmpipe. See
`port-docs/linux-native-port.md` and `port-docs/HANDOFF.md` for exact smoke-test
commands and artifacts.

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

The game needs retail PC assets. Run the executable with the working directory
set to an asset root containing at least:

```text
art/
movies/
scripts/
sound/
simpsons.ini
```

Retail RCF archives such as `dialog.rcf`, `music00.rcf`-`music03.rcf`,
`ambience.rcf`, `carsound.rcf`, `nis.rcf`, `scripts.rcf`, and `soundfx.rcf`
should be present at the asset root when using a full PC install.

Example:

```sh
cd "/tmp/shar-assets/The Simpsons - Hit & Run"
/path/to/repo/build/native/code/SRR2
/path/to/repo/build/native/code/SRR2 skipfe
```

Keep Linux asset lookup case-sensitive and deterministic. If a future asset dump
has path problems, instrument RAD file path composition before changing lookup
behavior; do not add case-folding as a first response.

## Provenance

Upstream base: ZenoArrows/The-Simpsons-Hit-and-Run @ `07951a8d` (Switch/Vita
port, full game playable). See
`port-docs/high-level/pure3d-runtime-options.md` for the evaluation and
`port-docs/upstream-readme.md` for the upstream README.

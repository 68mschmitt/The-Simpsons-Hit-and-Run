# Desktop Build (Linux / macOS)

The `zenoarrows-base` branch builds the full game (`SRR2`) via the adopted
port tree: original game code under `code/`, restored middleware under
`libs/`, one root `CMakeLists.txt`. The desktop SDL + OpenGL path is the
default CMake configuration (`RAD_WIN32 + RAD_CONSOLE + RAD_SDL`,
`SRR2_P3D_PDDI=OpenGL`).

## macOS

Dependencies (Homebrew): `cmake sdl2 libpng openal-soft ffmpeg`

```sh
OPENALDIR=/opt/homebrew/opt/openal-soft cmake -S . -B build/desktop \
    -DSRR2_BUILD_TESTS=OFF \
    -DCMAKE_DISABLE_FIND_PACKAGE_SDL3=ON \
    -DCMAKE_FIND_FRAMEWORK=LAST
cmake --build build/desktop -j8
```

Notes:

- SDL3 is disabled because the upstream SDL3 code path has API drift in
  `libs/pure3d/pddi/gl/display_win32/gldisplay.cpp` (gamma ramp, renamed
  constants). Fixing that would allow SDL3.
- `OPENALDIR` + `CMAKE_FIND_FRAMEWORK=LAST` steer CMake away from the
  deprecated system OpenAL framework, which lacks `efx.h`.
- The OpenGL PDDI backend requests a legacy compatibility context; macOS
  provides GL 2.1, which is deprecated but functional. If it proves
  problematic, `-DSRR2_P3D_PDDI=GLES2` via ANGLE is the fallback.

## Linux

Dependencies (apt): `cmake libsdl2-dev libpng-dev libopenal-dev`
plus FFmpeg dev packages (`libavformat-dev libavcodec-dev libavutil-dev
libswresample-dev libswscale-dev`).

```sh
cmake -S . -B build/desktop -DSRR2_BUILD_TESTS=OFF
cmake --build build/desktop -j$(nproc)
```

Verified 2026-07-07 on Ubuntu 24.04 (GCC, x86-64, Docker) with the package
list above.

## Running

The game needs the retail PC assets (not the leaked-source assets): copy the
retail installation's data folders (`art`, `sound`, `movies`, `scripts`,
`music`, ...) next to the binary or run with the working directory set to
the asset root. Movies must be the original `.rmv` files.

Asset dumps coming from Windows can have case mismatches on case-sensitive
filesystems. The `main` branch's `srr2-linux-poc` tool records and probes
asset requests (`--data-root`, `--asset-manifest`) and is the quickest way
to validate a dump.

## Provenance

Upstream base: ZenoArrows/The-Simpsons-Hit-and-Run @ `07951a8d` (Switch/
Vita port, full game playable). See
`port-docs/high-level/pure3d-runtime-options.md` for the evaluation and
`port-docs/upstream-readme.md` for the upstream README.

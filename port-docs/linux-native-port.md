# Linux Native Port

This checkout includes a playable SDL2/OpenGL Linux target (`SRR2`) using the
restored RAD/Pure3D middleware under `libs/` and the ported game code under
`code/`.

## Dependencies

Install the native build prerequisites with one of these commands:

```sh
# Ubuntu/Debian
sudo apt install build-essential cmake pkg-config libsdl2-dev libpng-dev libopenal-dev \
    libavformat-dev libavcodec-dev libavutil-dev libswresample-dev libswscale-dev zlib1g-dev p7zip-full

# Arch Linux
sudo pacman -S --needed cmake gcc pkgconf sdl2-compat libpng openal ffmpeg zlib p7zip

# Fedora
sudo dnf install gcc-c++ cmake pkgconf-pkg-config SDL2-devel libpng-devel \
    openal-soft-devel ffmpeg-devel zlib-devel p7zip
```

`7z`/p7zip is only needed by `srr2 --setup-assets`; an existing retail install
does not need to be copied or extracted. For headless smoke tests, also install Xvfb and xwd:

```sh
# Arch
sudo pacman -S --needed xorg-server-xvfb xorg-xwd imagemagick

# Debian/Ubuntu
sudo apt install xvfb x11-apps imagemagick
```

SDL3 is not used by default; keep it off for validation unless you are working
specifically on the SDL3 backend.

## Required game data and launcher setup

A lawful retail PC data root must contain at least:

```text
art/
movies/
scripts/
sound/
simpsons.ini
art/frontend/dynaload/images/license/licensePC.p3d
art/frontend/scrooby/frontend.p3d
```

A full installation also has retail RCFs such as `dialog.rcf`, `music00.rcf`
through `music03.rcf`, and `scripts.rcf`. The launcher validates the minimum
boot/frontend entries above with exact, case-sensitive diagnostics; it does not
case-fold file names.

After installing the project, record an existing PC installation once:

```sh
srr2 --set-data-dir "/path/to/The Simpsons - Hit & Run"
srr2
srr2 skipfe
```

The setting is stored as a literal `DATA_DIR=...` line in
`$XDG_CONFIG_HOME/srr2/config`, or `$HOME/.config/srr2/config` when XDG config
is unset. A one-off `--data-dir` overrides that setting; `SRR2_DATA_DIR` is the
environment-variable alternative. Run `srr2 --print-config` to see the selected
root and executable.

For an archive, use the managed user data location rather than copying retail
files into this checkout:

```sh
srr2 --setup-assets "/path/to/The Simpsons - Hit & Run.rar"
```

The command needs `7z`, extracts into `$XDG_DATA_HOME/srr2/assets` (or
`$HOME/.local/share/srr2/assets`), validates the result before moving it into
place, and leaves an existing incomplete managed directory untouched. It may
also be given an existing retail directory, which is recorded directly without
copying it. `tools/linux-runtime/validate-assets.sh "$ASSET_ROOT"` runs the
same read-only validation used by the launcher and smoke scripts.

For a manual temporary extraction outside the repository:

```sh
7z x -y -o/tmp/shar-assets "The Simpsons - Hit & Run.rar"
ASSET_ROOT="/tmp/shar-assets/The Simpsons - Hit & Run"
tools/linux-runtime/validate-assets.sh "$ASSET_ROOT"
```

## Build, install, and developer run

```sh
cmake -S . -B build/native -DCMAKE_BUILD_TYPE=Release -DSRR2_BUILD_TESTS=OFF -DSRR2_USE_PCH=OFF
cmake --build build/native -j$(nproc)
cmake --install build/native --prefix "$HOME/.local"
```

The install places `SRR2` and `srr2` in `$HOME/.local/bin`, the desktop entry
in `$HOME/.local/share/applications`, the 128px icon in the hicolor icon tree,
and this document plus the SDL control reference in `$HOME/.local/share/doc/SRR2`.
Ensure `$HOME/.local/bin` is on `PATH` before invoking `srr2`.

For source-tree work, use the build/run wrapper instead of changing into a
build directory or asset root manually:

```sh
tools/linux-runtime/dev-run.sh --data-dir "/path/to/The Simpsons - Hit & Run" -- skipfe
```

It checks prerequisites (or `--install-deps` installs them through apt, pacman,
or dnf), configures the known-good Release SDL2 defaults, builds incrementally,
and launches through `srr2-launch`. The desktop build intentionally uses
`RAD_WIN32 + RAD_CONSOLE + RAD_SDL` and keeps `RAD_PC` undefined. SDL2 remains
the default; SDL3 is opt-in while its backend is incomplete.

If audio hardware is unavailable for a direct smoke run, use OpenAL Soft's null
backend through the launcher:

```sh
ALSOFT_DRIVERS=null srr2 --data-dir "$ASSET_ROOT" skipfe
```

## Headless smoke tests

### Local Xvfb harness

Use this when Docker is unavailable:

```sh
ASSET_ROOT="/tmp/shar-assets/The Simpsons - Hit & Run" \
OUT_DIR=/tmp/srr2-smoke/frontend \
RUN_SECONDS=60 \
SKIP_BUILD=1 \
tools/linux-runtime/run-local-game.sh

ASSET_ROOT="/tmp/shar-assets/The Simpsons - Hit & Run" \
OUT_DIR=/tmp/srr2-smoke/skipfe \
RUN_SECONDS=120 \
GAME_ARGS=skipfe \
SKIP_BUILD=1 \
tools/linux-runtime/run-local-game.sh
```

The harness forces `SDL_VIDEODRIVER=x11` and unsets `WAYLAND_DISPLAY` so SDL
uses the Xvfb display even when the host desktop session is Wayland. Logs land in
`$OUT_DIR/game.log`; screenshots land in `$OUT_DIR/screenshot.xwd` and, when
ImageMagick is installed, `$OUT_DIR/screenshot.png`.

### Docker harness

Once the Docker daemon is available:

```sh
docker build -t srr2-linux-runtime tools/linux-runtime
mkdir -p /tmp/srr2-docker-out
docker run --rm \
  -v "$PWD:/src:ro" \
  -v "/tmp/shar-assets/The Simpsons - Hit & Run:/assets:ro" \
  -v /tmp/srr2-docker-out:/out \
  -e RUN_SECONDS=120 \
  -e GAME_ARGS=skipfe \
  srr2-linux-runtime
```

## Expected validation output

Normal frontend smoke should include:

```text
GameFlow context: ENTRY -> BOOTUP
GameFlow context: BOOTUP -> FRONTEND
```

The screenshot should show the title/frontend flow, and there should be no
`FileNotFound` for `licensePC.p3d` or `frontend.p3d`.

Gameplay shortcut smoke should include:

```text
GameFlow context: ENTRY -> BOOTUP
GameFlow context: BOOTUP -> LOADING_GAMEPLAY
GameFlow context: LOADING_GAMEPLAY -> GAMEPLAY
```

The screenshot should show Level 1 gameplay. Timeout status `124` is expected
for these smoke tests.

Validation completed on 2026-07-18 with retail assets:

- frontend log/screenshot: `/tmp/srr2-smoke/frontend-final/`
- `skipfe` log/screenshot: `/tmp/srr2-smoke/skipfe-final/`

## Desktop controls

A keyboard/mouse-backed virtual controller is exposed as `Port0\\Slot0`, so the
game is playable without a physical gamepad. The maintained reference is
[SDL keyboard/mouse controls](../docs/sdl-keyboard-mouse-controls.md): on foot,
use `WASD`, `Space`, `E`, `F`/`Ctrl`/left mouse, and `P`; while driving use
`WASD`, `Space` for handbrake, `H`/`Shift` for horn, and `R` to reset. Mouse
movement or `IJKL` controls camera/look. Native SDL gamepads retain the console
input mappings.

## Current known gaps

- Full subjective playability, real-audio quality, and end-to-end save/load UI
  behavior still need a real desktop pass.
- `SAVE:` resolves to `$XDG_DATA_HOME/srr2`, then `$HOME/.local/share/srr2`,
  then local `save/` only when HOME/XDG are unavailable. The selected directory
  is created automatically; creation/access failures are logged as `SRR2 save:`
  diagnostics and do not silently fall back to the retail asset tree. End-to-end
  UI save/load still needs validation; see [Runtime Validation Records](runtime-validation.md).
- Do not reintroduce case-folding to solve path issues. If file loading fails,
  first instrument `radSdlDrive::BuildFileSpec`/open results and fix path
  composition.

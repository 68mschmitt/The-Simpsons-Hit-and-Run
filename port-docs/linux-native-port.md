# Linux Native Port

This checkout includes a playable SDL2/OpenGL Linux target (`SRR2`) using the
restored RAD/Pure3D middleware under `libs/` and the ported game code under
`code/`.

## Dependencies

Arch Linux:

```sh
sudo pacman -S --needed cmake gcc pkgconf sdl2-compat libpng openal ffmpeg zlib
```

Debian/Ubuntu equivalents:

```sh
sudo apt install build-essential cmake pkg-config libsdl2-dev libpng-dev libopenal-dev \
    libavformat-dev libavcodec-dev libavutil-dev libswresample-dev libswscale-dev zlib1g-dev
```

For headless smoke tests, also install Xvfb and xwd:

```sh
# Arch
sudo pacman -S --needed xorg-server-xvfb xorg-xwd imagemagick

# Debian/Ubuntu
sudo apt install xvfb x11-apps imagemagick
```

SDL3 is not used by default; keep it off for validation unless you are working
specifically on the SDL3 backend.

## Required game data

Run the native executable from a PC installation data directory containing at
least:

```text
art/
movies/
scripts/
sound/
simpsons.ini
```

For local testing with the retail archive in this checkout, extract outside the
source tree:

```sh
7z x -y -o/tmp/shar-assets "The Simpsons - Hit & Run.rar"
ASSET_ROOT="/tmp/shar-assets/The Simpsons - Hit & Run"
```

Useful checks:

```sh
test -d "$ASSET_ROOT/art"
test -d "$ASSET_ROOT/movies"
test -d "$ASSET_ROOT/scripts"
test -d "$ASSET_ROOT/sound"
test -f "$ASSET_ROOT/simpsons.ini"
test -f "$ASSET_ROOT/art/frontend/dynaload/images/license/licensePC.p3d"
test -f "$ASSET_ROOT/art/frontend/scrooby/frontend.p3d"
```

Do not run the executable from the source tree unless these data folders have
been copied there.

## Build

```sh
cmake -S . -B build/native -DCMAKE_BUILD_TYPE=Release -DSRR2_BUILD_TESTS=OFF -DSRR2_USE_PCH=OFF
cmake --build build/native -j$(nproc)
```

The desktop build intentionally uses `RAD_WIN32 + RAD_CONSOLE + RAD_SDL` and
keeps `RAD_PC` undefined.

## Run on a desktop session

From the extracted/installed game data directory:

```sh
cd "/tmp/shar-assets/The Simpsons - Hit & Run"
/path/to/repo/build/native/code/SRR2
```

Use `skipfe` as a development shortcut to load directly into gameplay:

```sh
/path/to/repo/build/native/code/SRR2 skipfe
```

If audio hardware is unavailable during smoke tests, use OpenAL Soft's null
backend:

```sh
ALSOFT_DRIVERS=null /path/to/repo/build/native/code/SRR2 skipfe
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

A keyboard/mouse backed virtual controller is exposed as `Port0\\Slot0`, so the
game is playable without a physical gamepad:

- Movement/steer/menu navigation: `WASD` or arrow keys
- Select/jump/gas: `Space` or `Enter` (gas also uses `W`/Up via right trigger)
- Back/dash: `Esc` or `Shift`
- Action: `E`
- Attack/handbrake: `Ctrl`, `F`, or left mouse
- Brake/reverse: `S`/Down
- Pause: `P`
- Reset car: `R`
- Horn: `H` or right mouse
- Camera/look: mouse movement, or `IJKL`

## Current known gaps

- Full manual controls, real-audio quality, and save/load behavior still need a
  real desktop pass.
- If save/load is broken, the next implementation task should define a Linux
  save directory policy (for example `$XDG_DATA_HOME/srr2`, with a local
  `save/` fallback) through the existing RAD drive model.
- Do not reintroduce case-folding to solve path issues. If file loading fails,
  first instrument `radSdlDrive::BuildFileSpec`/open results and fix path
  composition.

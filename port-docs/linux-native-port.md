# Linux Native Port

This checkout includes a playable SDL/OpenGL Linux target (`SRR2`) using the restored RAD/Pure3D middleware under `libs/` and the ported game code under `code/`.

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

SDL3 is not used by default; the current OpenGL PDDI backend is tested through the SDL2 interface.

## Required game data

Run the native executable from a PC installation data directory containing at least:

```text
art/
movies/
scripts/
sound/
simpsons.ini
```

For local testing, extract a retail PC archive outside the source tree, for example:

```sh
7z x -o/tmp/shar-assets "The Simpsons - Hit & Run.rar"
```

Do not run the executable from the source tree unless these data folders have been copied there.

## Build

```sh
cmake -S . -B build/native -DCMAKE_BUILD_TYPE=Release -DSRR2_BUILD_TESTS=OFF -DSRR2_USE_PCH=OFF
cmake --build build/native -j$(nproc)
```

## Run

From the extracted/installed game data directory:

```sh
cd "/tmp/shar-assets/The Simpsons - Hit & Run"
/path/to/repo/build/native/code/SRR2
```

Use `skipfe` as a development shortcut to load directly into gameplay:

```sh
/path/to/repo/build/native/code/SRR2 skipfe
```

If audio hardware is unavailable during smoke tests, set `SDL_AUDIODRIVER=dummy`.

## Desktop controls

A keyboard/mouse backed virtual controller is exposed as `Port0\\Slot0`, so the game is playable without a physical gamepad:

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

## Validation notes

Useful smoke-test sequence:

```sh
cmake -S . -B build/native -DCMAKE_BUILD_TYPE=Release -DSRR2_BUILD_TESTS=OFF -DSRR2_USE_PCH=OFF
cmake --build build/native -j$(nproc)
cd "/tmp/shar-assets/The Simpsons - Hit & Run"
SDL_AUDIODRIVER=dummy timeout -s KILL 20s /path/to/repo/build/native/code/SRR2
SDL_AUDIODRIVER=dummy timeout -s KILL 30s /path/to/repo/build/native/code/SRR2 skipfe
```

Expected behavior: normal startup reaches the frontend without requiring intro FMV playback, and `skipfe` reaches gameplay without the SDL pause-settings camera layout crash.

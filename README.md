<h2 align=center>Simpsons Hit & Run 2003 Source Code</h2>

## Linux quickstart

The native SDL2/OpenGL build needs a lawful retail PC data installation. The
launcher keeps that data outside this checkout, validates it before launch, and
runs the game from the required asset working directory.

Install build prerequisites for your distribution:

```sh
# Ubuntu/Debian
sudo apt install build-essential cmake pkg-config libsdl2-dev libpng-dev libopenal-dev libavformat-dev libavcodec-dev libavutil-dev libswresample-dev libswscale-dev zlib1g-dev p7zip-full

# Arch
sudo pacman -S --needed cmake gcc pkgconf sdl2-compat libpng openal ffmpeg zlib p7zip

# Fedora
sudo dnf install gcc-c++ cmake pkgconf-pkg-config SDL2-devel libpng-devel openal-soft-devel ffmpeg-devel zlib-devel p7zip
```

Build and install into your user prefix:

```sh
cmake -S . -B build/native -DCMAKE_BUILD_TYPE=Release -DSRR2_BUILD_TESTS=OFF -DSRR2_USE_PCH=OFF
cmake --build build/native -j$(nproc)
cmake --install build/native --prefix "$HOME/.local"
```

Ensure `$HOME/.local/bin` is on `PATH`, then point the launcher at the existing
retail installation and run it:

```sh
srr2 --set-data-dir "/path/to/The Simpsons - Hit & Run"
srr2
# Development shortcut: srr2 skipfe
```

To extract a supported `.rar`, `.zip`, or `.7z` retail archive into
`$XDG_DATA_HOME/srr2/assets` (or `$HOME/.local/share/srr2/assets`), use
`srr2 --setup-assets /path/to/archive`. It requires `7z` and never copies an
existing retail directory into the repository.

For source-tree development, one command configures, incrementally builds, and
uses the same launcher validation:

```sh
tools/linux-runtime/dev-run.sh --data-dir "/path/to/The Simpsons - Hit & Run" -- skipfe
```

See [port-docs/linux-native-port.md](port-docs/linux-native-port.md) for data
requirements, headless smoke tests, troubleshooting, and runtime details. See
[docs/sdl-keyboard-mouse-controls.md](docs/sdl-keyboard-mouse-controls.md) for
the current SDL keyboard/mouse controls.

<br>

<p align=center><b>Decided to do something dumb today and upload stolen and slightly cleaned up source code from 2003 of <a href='https://en.wikipedia.org/wiki/The_Simpsons:_Hit_%26_Run'>The Simpsons: Hit & Run</a> video game for the PS2 Xbox and PC, in which I found on an unknown-ish forum that I'm not going to mention here.</b></p>

<br>

<h5 align=center>Want to talk more about this leak?<br><b><a href='https://news.ycombinator.com/item?id=36703711'>Click Here</a></b> to read on over 150 comments.</h5>

<br>

<h3 align=center><sup>A download for a compiled development/debug version of the game is in the releases tab, the rest of this repo is on <a href='https://mega.nz/folder/kBhiACKI#QUjjgq2yqYBhiqpxkbZjBw'>MEGA</a> since there are way too many files to upload at once to GitHub. The MEGA folder will include a ton of additional assets and scripts for compiling/building the game yourself. (documents will still not be included)</sup></h3>

<h4 align=center><b>Enjoy :)</b></h4>

<br>

<p align=center><sup><b>NOTE:</b> I didn't include the 'documents/' folder in this repo because it contains private information about Radical's (2003) development team and their project notes, vacation schedules etc.</sup></p>

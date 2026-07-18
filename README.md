<h2 align=center>Simpsons Hit & Run 2003 Source Code</h2>

## Linux native build

This checkout includes a playable SDL/OpenGL Linux target (`SRR2`) using the restored RAD/Pure3D middleware under `libs/` and the ported game code under `code/`.

See [port-docs/linux-native-port.md](port-docs/linux-native-port.md) for dependencies, required PC game data, controls, and validation notes.

```sh
cmake -S . -B build/native -DCMAKE_BUILD_TYPE=Release -DSRR2_BUILD_TESTS=OFF -DSRR2_USE_PCH=OFF
cmake --build build/native -j$(nproc)
```

Run from a PC game-data directory containing `art/`, `movies/`, `scripts/`, `sound/`, and `simpsons.ini`:

```sh
cd "/path/to/The Simpsons - Hit & Run"
/path/to/repo/build/native/code/SRR2
```

<br>

<p align=center><b>Decided to do something dumb today and upload stolen and slightly cleaned up source code from 2003 of <a href='https://en.wikipedia.org/wiki/The_Simpsons:_Hit_%26_Run'>The Simpsons: Hit & Run</a> video game for the PS2 Xbox and PC, in which I found on an unknown-ish forum that I'm not going to mention here.</b></p>

<br>

<h5 align=center>Want to talk more about this leak?<br><b><a href='https://news.ycombinator.com/item?id=36703711'>Click Here</a></b> to read on over 150 comments.</h5>

<br>

<h3 align=center><sup>A download for a compiled development/debug version of the game is in the releases tab, the rest of this repo is on <a href='https://mega.nz/folder/kBhiACKI#QUjjgq2yqYBhiqpxkbZjBw'>MEGA</a> since there are way too many files to upload at once to GitHub. The MEGA folder will include a ton of additional assets and scripts for compiling/building the game yourself. (documents will still not be included)</sup></h3>

<h4 align=center><b>Enjoy :)</b></h4>

<br>

<p align=center><sup><b>NOTE:</b> I didn't include the 'documents/' folder in this repo because it contains private information about Radical's (2003) development team and their project notes, vacation schedules etc.</sup></p>

# Runtime Validation Records

## 2026-07-19 — Linux OpenAL real-device verification

- Repo state: `main` at `891c3b8`.
- Artifact directory: `/tmp/srr2-openal-validation-20260719T013647Z`.
- Assets: `/tmp/shar-assets/The Simpsons - Hit & Run`.
- Build: Release, tests off, PCH off.
- Result: PASS for automated OpenAL Soft null-backend smoke and real-device
  smoke. Real audio backend access was available in this environment through
  PipeWire/Pulse; OpenAL Soft created the non-null `USB2.0 Device Analog Stereo`
  playback device and the game reached gameplay before the expected timeout.
  No code changes were required.

### Commands

```sh
cmake -S . -B build/native -DCMAKE_BUILD_TYPE=Release -DSRR2_BUILD_TESTS=OFF -DSRR2_USE_PCH=OFF
cmake --build build/native -j$(nproc)
```

Host/OpenAL probes recorded `ldd build/native/code/SRR2`, default
`openal-info`, null `openal-info` with `ALSOFT_DRIVERS=null`, real-preferred
`openal-info` with `ALSOFT_DRIVERS=pipewire,pulse,alsa`, `pactl info`, and
`wpctl status` under `01-host-probe/`. `ldd` resolves `libopenal.so.1` from
`/usr/lib/libopenal.so.1`; `openal-info` reports OpenAL Soft 1.25.2 with
required `AL_SOFTX_map_buffer`, plus `AL_EXT_debug` and `ALC_EXT_EFX`.

Null smoke used the standard harness:

```sh
ASSET_ROOT="/tmp/shar-assets/The Simpsons - Hit & Run" \
OUT_DIR="/tmp/srr2-openal-validation-20260719T013647Z/02-null-smoke/run" \
BUILD_DIR="/home/mschmitt/projects/The-Simpsons-Hit-and-Run/build/native" \
RUN_SECONDS=45 GAME_ARGS="skipfe notutorial" SKIP_BUILD=1 \
ALSOFT_DRIVERS=null ALSOFT_LOGLEVEL=3 \
ALSOFT_LOGFILE="/tmp/srr2-openal-validation-20260719T013647Z/02-null-smoke/run/alsoft.log" \
tools/linux-runtime/run-local-game.sh
```

Real preferred-backend smoke used the standard harness with an explicit non-null
backend list:

```sh
ASSET_ROOT="/tmp/shar-assets/The Simpsons - Hit & Run" \
OUT_DIR="/tmp/srr2-openal-validation-20260719T013647Z/03-real-smoke/harness-real-preferred" \
BUILD_DIR="/home/mschmitt/projects/The-Simpsons-Hit-and-Run/build/native" \
RUN_SECONDS=45 GAME_ARGS="skipfe notutorial" SKIP_BUILD=1 \
ALSOFT_DRIVERS=pipewire,pulse,alsa ALSOFT_LOGLEVEL=3 \
ALSOFT_LOGFILE="/tmp/srr2-openal-validation-20260719T013647Z/03-real-smoke/harness-real-preferred/alsoft.log" \
tools/linux-runtime/run-local-game.sh
```

Default-backend smoke bypassed `tools/linux-runtime/run-local-game.sh` because
that harness intentionally sets `ALSOFT_DRIVERS=null` when the variable is unset.
The successful artifact is `03-real-smoke/direct-default-unset-run2/`; it used
`env -u ALSOFT_DRIVERS` with a custom Xvfb wrapper.

Equivalent user-run command for validating default real audio on a desktop/audio
session:

```sh
cd "/tmp/shar-assets/The Simpsons - Hit & Run"
env -u ALSOFT_DRIVERS \
  ALSOFT_LOGLEVEL=3 \
  ALSOFT_LOGFILE="/tmp/srr2-openal-validation-20260719T013647Z/direct-default-user.alsoft.log" \
  /home/mschmitt/projects/The-Simpsons-Hit-and-Run/build/native/code/SRR2 skipfe notutorial
```

Expected real-audio evidence is an OpenAL Soft log containing
`Initialized backend "pipewire"` or another non-null backend and
`Created device ..., "<real device>"`.

### Coverage

| Area | Result | Evidence |
| --- | --- | --- |
| Configure/build | PASS | `00-build/configure.log` status 0 and `00-build/build.log` status 0. |
| Host audio/OpenAL probe | PASS | `01-host-probe/summary.md` shows PipeWire 1.6.8 / PulseAudio-on-PipeWire, default sink `USB2.0 Device Analog Stereo`, and `libopenal.so.1` linkage. |
| Required OpenAL extension | PASS | `01-host-probe/extension-error-audit.md` shows `AL_SOFTX_map_buffer` present for default, null, and real-preferred OpenAL devices. |
| Null OpenAL smoke | PASS | `02-null-smoke/run/status.txt` is 124; `alsoft.log` shows backend `null` and device `No Output`; `game.log` reaches `LOADING_GAMEPLAY -> GAMEPLAY`. |
| Real preferred-backend smoke | PASS | `03-real-smoke/harness-real-preferred/status.txt` is 124; `alsoft.log` shows backend `pipewire` and device `USB2.0 Device Analog Stereo`; `game.log` reaches `LOADING_GAMEPLAY -> GAMEPLAY`. |
| Default-backend real smoke | PASS | `03-real-smoke/direct-default-unset-run2/status.txt` is 124; `alsoft.log` shows backend `pipewire` with `ALSOFT_DRIVERS` unset and device `USB2.0 Device Analog Stereo`; `game.log` reaches `LOADING_GAMEPLAY -> GAMEPLAY`. |
| Real audio accessibility | PASS | Host session exposed `/run/user/1000/pipewire-0` and `/run/user/1000/pulse/native`; no no-device fallback was needed. |

### Limitations

- Automated validation proves real OpenAL backend/device initialization and
  gameplay smoke stability, not subjective speaker audibility, mix correctness,
  music/dialog sequencing, or latency.
- Video still ran under Xvfb/llvmpipe for deterministic automation; audio used
  the host PipeWire/Pulse session.
- OpenAL Soft logged `1 Effect not deleted` at shutdown in null and real smoke
  runs. This did not block startup/gameplay but is cleanup evidence if EFX
  lifecycle cleanup becomes a follow-up target.
- `tools/linux-runtime/run-local-game.sh` defaults to null audio. Future real
  OpenAL runs must set `ALSOFT_DRIVERS=pipewire,pulse,alsa` or use a wrapper that
  leaves `ALSOFT_DRIVERS` unset.

## 2026-07-19 — Linux SDL keyboard and targeted mouse input

- Repo state: `main` at `9a5ba4e`.
- Artifact directory: `/tmp/srr2-input-validation-20260719T002744Z`.
- Assets: `/tmp/shar-assets/The Simpsons - Hit & Run`.
- Build: Release, tests off, PCH off.
- Result: PASS for automated keyboard smoke coverage, plus targeted PASS for
  observable left-mouse attack and relative mouse camera/look response. Mouse
  right-button vehicle horn and mouse handbrake are not validated by these
  artifacts; no code changes required.

### Commands

```sh
cmake -S . -B build/native -DCMAKE_BUILD_TYPE=Release -DSRR2_BUILD_TESTS=OFF -DSRR2_USE_PCH=OFF
cmake --build build/native -j$(nproc)
```

Runtime smoke used `tools/linux-runtime/run-local-game.sh` with `SKIP_BUILD=1`,
`ASSET_ROOT=/tmp/shar-assets/The Simpsons - Hit & Run`, and Xvfb/xdotool held-key
input. Gameplay shortcut cases used `GAME_ARGS="skipfe notutorial"` to avoid the
tutorial popup while validating gameplay controls.

Mouse follow-up artifacts were generated under `05-mouse-followup/` with the
same harness/build/assets using:

```sh
/tmp/srr2-input-validation-20260719T002744Z/05-mouse-followup/run-mouse-followup.sh
/tmp/srr2-input-validation-20260719T002744Z/05-mouse-followup/run-mouse-motion-sweep.sh
```

### Coverage

| Area | Result | Evidence |
| --- | --- | --- |
| Frontend boot | PASS | `01-xvfb-frontend/baseline/` shows title/frontend and expected `BOOTUP -> FRONTEND`. |
| Frontend navigation/select/back | PASS | `01-xvfb-frontend/options-select-back-held/` shows Start to menu, Right navigation to Options, Return selecting Options, Escape returning to main menu. |
| On-foot keyboard movement/jump | PASS | `02-xvfb-skipfe-pause/onfoot-movement-probe/` shows arrow/WASD movement and Space jump. |
| Vehicle keyboard entry/gas/steer/brake | PASS | `02-xvfb-skipfe-pause/vehicle-entry-probe/` shows E entering the pink car, gas/steer movement, and brake/reverse response. |
| Vehicle keyboard horn/handbrake/reset | PASS with audio limitation | `02-xvfb-skipfe-pause/vehicle-controls-supplement/` issues H, F while driving, and R. Reset visibly relocates the car; horn audio was not audible because smoke runs use null/dummy audio. |
| Mouse left button / on-foot attack | PASS | `05-mouse-followup/xvfb-mouse-buttons-motion/` records a Button1 hold in `xdotool.log`; `00-before-mouse-input.png` -> `01-during-lmb-down.png` shows Homer enter the attack/kick animation. |
| Relative mouse motion / camera-look path | PASS, limited | `05-mouse-followup/xvfb-mouse-motion-sweep/` records right/left relative mouse sweeps in `xdotool.log`; `00-before-motion-sweep.png`, `01-after-right-sweep.png`, and `02-after-left-sweep.png` show camera yaw changes. `image-diff.txt` records non-zero frame diffs. This validates visible response only, not subjective mouse-look feel. |
| Mouse right button / vehicle horn | NOT VALIDATED | `05-mouse-followup/xvfb-mouse-buttons-motion/` injects RMB only on foot. No artifact demonstrates RMB in a vehicle or an audible/visible horn effect. |
| Pause/unpause | PASS | `02-xvfb-skipfe-pause/pause-unpause-start-held/` logs `GAMEPLAY -> PAUSE -> GAMEPLAY` and screenshots pause menu/resumed gameplay. |
| Real display automated smoke | PASS, limited | `03-manual/real-display-automated-smoke/` ran directly on detected `DISPLAY=:1` and confirmed the desktop window receives keyboard input. |

### Limitations

- Xvfb/llvmpipe smoke validates deterministic control delivery and visible state
  changes, not subjective gameplay or mouse-look feel.
- Null/dummy audio means horn input was not audibly verified. Keyboard H horn was
  smoke-tested for acceptance/no crash only; mouse RMB horn is not validated.
- Mouse follow-up validates LMB on-foot attack and relative mouse camera yaw only.
  It does not separately validate mouse handbrake in a vehicle, RMB as vehicle
  horn, frontend mouse UI selection, or physical mouse feel on a human desktop.
- Physical gamepad priority/conflict with keyboard on `Port0\\Slot0` was not
  changed or validated.
- A human desktop pass is still recommended for analog/deadzone feel, mouse-look
  feel, real audio, mouse handbrake/RMB horn, and physical-controller conflict
  checks. Exact manual commands and expected artifacts are in
  `/tmp/srr2-input-validation-20260719T002744Z/notes.md`.

# Runtime Validation Records

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

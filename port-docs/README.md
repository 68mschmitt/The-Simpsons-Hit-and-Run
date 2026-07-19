# Linux Port Documentation

This directory documents the current desktop port work for `SRR2`, the restored
full-game target.

## Current state

As of 2026-07-18, `main` builds and runs the full game on Linux with SDL2,
OpenGL, OpenAL, FFmpeg, and the restored middleware under `libs/`. The old
stub-first Linux PoC is historical; use `SRR2` for current validation and fixes.

Validated Linux smoke tests with retail PC assets now cover:

- normal boot to the frontend/title screen; and
- `skipfe` boot to Level 1 gameplay.

See [HANDOFF.md](HANDOFF.md) for exact commands, output paths, and latest known
blockers. See [linux-native-port.md](linux-native-port.md) for everyday build,
run, controls, and runtime-harness instructions.

## Key documents

- [HANDOFF.md](HANDOFF.md): current status, completed runtime validation,
  reproduction commands, and next triage targets.
- [Linux Native Port](linux-native-port.md): dependencies, asset setup, build,
  run, Xvfb/Docker smoke commands, and controls.
- [Runtime Validation Records](runtime-validation.md): dated validation results,
  artifact paths, coverage, and limitations.
- [Desktop Build](desktop-build.md): desktop CMake notes.
- [Initial PoC Plan](initial-poc-plan.md): historical record for the earlier
  non-playable Linux proof of concept.
- [High-Level Port Overview](high-level/README.md): original strategy and phase
  map.

## Current guiding principle

Stabilize the full `SRR2` desktop runtime with real retail assets before adding
new platform features. Keep Linux file lookup case-sensitive and deterministic;
do not paper over asset-path bugs with case-folding.

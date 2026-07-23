# Native save manager

The native Linux build can install a Go interactive save manager at
[`tools/savegame/README.md`](../tools/savegame/README.md). It is built with
Charm's Bubble Tea, Bubbles, and Lip Gloss v2 libraries. CMake builds and
installs `srr2-save` when Go 1.26.5 or newer is available; set
`-DSRR2_BUILD_SAVE_MANAGER=OFF` to omit it.

```sh
# After `cmake --install ...`, run the installed manager.
srr2-save

# Or run it from the source tree.
cd tools/savegame
go run ./cmd/srr2-save

# Use a non-default save location when needed.
go run ./cmd/srr2-save --save-dir /path/to/srr2-saves
```

The manager supports only the current 64-bit native Linux layout: a 7,194-byte
little-endian `Save1`–`Save4` file. It refuses legacy PC and console saves
rather than decode their incompatible raw C++ structures.

## Capabilities

- Validates and lists all slots, including timestamp, level/mission, progress,
  SHA-256, and invalid/empty status.
- Inspects player progress, tokens, cars, persistent-world summary, cards,
  gags, wasps, FMVs, and named mission records.
- Stages gag, card, mission, token, wasp, FMV, and gag-count-repair edits for
  review before any disk write.
- Imports a native export (with explicit confirmation before replacing an
  occupied slot), copies to an empty slot, exports a selected slot, and moves a
  selected slot to recoverable `.savegame-trash`.
- Creates an exclusive sibling `SaveN.bak[.N]` before every in-place commit.

The dashboard shows shortcuts directly. Open a valid slot with `Enter`; the
detail view supports `1`–`4` tabs, `Space` to toggle a selected gag/card/named
mission, `[`/`]` to change level, and `s` to review/save staged edits.

## Save location and safety

The game and manager choose `$XDG_DATA_HOME/srr2` when `XDG_DATA_HOME` is an
absolute path, then `$HOME/.local/share/srr2`; when neither is available the
game falls back to `save/` beneath its working directory.

**Exit SRR2 before making changes.** The manager refuses writes and
replacements in the default save directory while it detects an `SRR2` process,
and refuses trashing from any directory while it detects one. It rejects
symlinks and non-regular files, verifies a file has not changed since loading,
fsyncs writes, and uses an atomic exchange that restores an unexpected
concurrent update rather than silently overwriting it.

Run the Go tests with `cd tools/savegame && go test ./...`.

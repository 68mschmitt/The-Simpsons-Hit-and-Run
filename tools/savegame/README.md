# SRR2 Save Manager

A polished, interactive save manager for the native 64-bit Linux build. It is
implemented in Go with Charm's [Bubble Tea](https://charm.land/bubbletea),
[Bubbles](https://charm.land/bubbles), and
[Lip Gloss](https://charm.land/lipgloss) v2 libraries.

It deliberately supports only the repository's 7,194-byte native Linux save
layout (`Save1`–`Save4`). Original PC and console saves use different raw C++
layouts and are rejected rather than risk corruption.

## Run

```sh
cd tools/savegame
go run ./cmd/srr2-save

# Or build a reusable binary.
go build -o srr2-save ./cmd/srr2-save
./srr2-save --save-dir "$HOME/.local/share/srr2"

# The Linux CMake build also builds and installs srr2-save with Go 1.26.5+.
cmake -S ../.. -B ../../build/native -DSRR2_BUILD_SAVE_MANAGER=ON
cmake --build ../../build/native
cmake --install ../../build/native --prefix "$HOME/.local"
```

The default save directory follows the game: `$XDG_DATA_HOME/srr2` when
`XDG_DATA_HOME` is absolute, otherwise `$HOME/.local/share/srr2`.

## Interface

The dashboard validates all four slots and shows their timestamp, progress,
completion totals, and SHA-256. It can import a valid backup, copy to an empty
slot, export a selected slot, and move a slot to recoverable trash. Importing
into an occupied slot always requires a second explicit confirmation.

Open a valid slot with `Enter`:

| Key | Action |
| --- | --- |
| `1`–`4` | Overview, gags, cards, and missions views |
| `[` / `]` | Change the current level in collection views |
| `Space` | Toggle the selected gag, card, or named mission |
| `t` / `w` / `f` | Stage token, wasp, or FMV changes |
| `r` | Repair gag-count fields from their bit masks |
| `d` | Discard all staged edits |
| `s` | Review and save staged edits |
| `Esc` | Return to the dashboard |

Dashboard controls: `i` import a valid save into the selected slot, `c` copy a
valid selected slot to an empty slot, `b` export, `x` move to trash, `r` reload,
and `q` quit. While an operation refreshes the slot list, dashboard actions are
temporarily disabled so a follow-up copy/export cannot use a stale snapshot.

## Safety

Edits are staged in memory; no save file changes until the review-and-save
confirmation. A commit validates the native bytes, rejects symlinks and stale
snapshots, makes an exclusive `SaveN.bak[.N]`, fsyncs data, and atomically
exchanges the temporary file with the slot. The displaced inode is verified
before deletion, so a concurrent newer file is restored rather than silently
overwritten. The manager also refuses writes and replacements in the default
save directory while an `SRR2` process is detected, and refuses trashing from
any directory while one is detected.

Always exit the game before manipulating a save.

## Test

```sh
cd tools/savegame
go test ./...
```

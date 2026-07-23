// Package storage owns Linux filesystem safety for the native save manager.
// It keeps the raw-byte native model separate from symlink, race, backup, and
// atomic-replacement policy.
package storage

import (
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"strings"
	"time"

	"github.com/mschmitt/the-simpsons-hit-and-run/tools/savegame/native"
	"golang.org/x/sys/unix"
)

type Fingerprint struct {
	Dev, Ino uint64
	Size     int64
	MTimeNS  int64
	Mode     uint32
}

type Snapshot struct {
	Path        string
	Raw         []byte
	Save        *native.Save // nil only for a corrupt file deliberately loaded for restore
	Fingerprint Fingerprint
}

type Slot struct {
	Number   int
	Path     string
	Status   string // empty, valid, invalid
	Snapshot *Snapshot
	Err      error
}

type CommitResult struct{ Backup string }

func DefaultDir() string {
	if xdg := os.Getenv("XDG_DATA_HOME"); filepath.IsAbs(xdg) && xdg != "" {
		return filepath.Join(xdg, "srr2")
	}
	if home := os.Getenv("HOME"); filepath.IsAbs(home) && home != "" {
		return filepath.Join(home, ".local", "share", "srr2")
	}
	cwd, err := os.Getwd()
	if err != nil {
		return "save"
	}
	return filepath.Join(cwd, "save")
}

func SlotPath(dir string, number int) (string, error) {
	if number < 1 || number > native.NumSlots {
		return "", fmt.Errorf("slot must be 1 through %d", native.NumSlots)
	}
	return filepath.Join(dir, fmt.Sprintf("Save%d", number)), nil
}

func fingerprint(path string) (Fingerprint, error) {
	var st unix.Stat_t
	if err := unix.Lstat(path, &st); err != nil {
		return Fingerprint{}, err
	}
	if st.Mode&unix.S_IFMT != unix.S_IFREG {
		return Fingerprint{}, fmt.Errorf("refusing non-regular file %s", path)
	}
	return Fingerprint{Dev: uint64(st.Dev), Ino: st.Ino, Size: st.Size, MTimeNS: st.Mtim.Sec*1_000_000_000 + st.Mtim.Nsec, Mode: st.Mode}, nil
}

func unchanged(path string, fp Fingerprint) error {
	current, err := fingerprint(path)
	if err != nil {
		return err
	}
	if current != fp {
		return fmt.Errorf("%s changed after it was read", path)
	}
	return nil
}

func load(path string, allowInvalid bool) (*Snapshot, error) {
	before, err := fingerprint(path)
	if err != nil {
		return nil, err
	}
	raw, err := os.ReadFile(path)
	if err != nil {
		return nil, err
	}
	after, err := fingerprint(path)
	if err != nil {
		return nil, err
	}
	if before != after {
		return nil, fmt.Errorf("%s changed while it was read", path)
	}
	save, parseErr := native.Parse(raw)
	if parseErr != nil && !allowInvalid {
		return nil, parseErr
	}
	return &Snapshot{Path: path, Raw: raw, Save: save, Fingerprint: after}, nil
}

func Load(path string) (*Snapshot, error)    { return load(path, false) }
func LoadRaw(path string) (*Snapshot, error) { return load(path, true) }

func Slots(dir string) []Slot {
	out := make([]Slot, 0, native.NumSlots)
	for number := 1; number <= native.NumSlots; number++ {
		path, _ := SlotPath(dir, number)
		_, err := os.Lstat(path)
		if errors.Is(err, os.ErrNotExist) {
			out = append(out, Slot{Number: number, Path: path, Status: "empty"})
			continue
		}
		if err != nil {
			out = append(out, Slot{Number: number, Path: path, Status: "invalid", Err: err})
			continue
		}
		snapshot, err := Load(path)
		if err != nil {
			out = append(out, Slot{Number: number, Path: path, Status: "invalid", Err: err})
			continue
		}
		out = append(out, Slot{Number: number, Path: path, Status: "valid", Snapshot: snapshot})
	}
	return out
}

func syncDir(dir string) error {
	file, err := os.Open(dir)
	if err != nil {
		return err
	}
	defer file.Close()
	return file.Sync()
}

func backup(snapshot *Snapshot) (string, error) {
	for suffix := 0; ; suffix++ {
		name := snapshot.Path + ".bak"
		if suffix > 0 {
			name = fmt.Sprintf("%s.bak.%d", snapshot.Path, suffix)
		}
		file, err := os.OpenFile(name, os.O_WRONLY|os.O_CREATE|os.O_EXCL, os.FileMode(snapshot.Fingerprint.Mode&0o777))
		if errors.Is(err, os.ErrExist) {
			continue
		}
		if err != nil {
			return "", err
		}
		if _, err = file.Write(snapshot.Raw); err == nil {
			err = file.Sync()
		}
		closeErr := file.Close()
		if err == nil {
			err = closeErr
		}
		if err != nil {
			_ = os.Remove(name)
			return "", err
		}
		if err = syncDir(filepath.Dir(snapshot.Path)); err != nil {
			return "", err
		}
		return name, nil
	}
}

// processRunning is replaceable in tests. It is intentionally best-effort: the
// snapshot/race checks remain the correctness boundary if /proc is unavailable.
var processRunning = running

func running() bool {
	entries, err := os.ReadDir("/proc")
	if err != nil {
		return false
	}
	for _, entry := range entries {
		if _, err := fmt.Sscanf(entry.Name(), "%d", new(int)); err != nil {
			continue
		}
		data, err := os.ReadFile(filepath.Join("/proc", entry.Name(), "comm"))
		if err != nil {
			continue
		}
		if strings.EqualFold(strings.TrimSpace(string(data)), "SRR2") {
			return true
		}
	}
	return false
}

func guardGame(path string) error {
	defaultDir, err := filepath.EvalSymlinks(DefaultDir())
	if err != nil {
		defaultDir = filepath.Clean(DefaultDir())
	}
	parent, err := filepath.EvalSymlinks(filepath.Dir(path))
	if err != nil {
		parent = filepath.Clean(filepath.Dir(path))
	}
	if parent == defaultDir && processRunning() {
		return errors.New("SRR2 appears to be running with the default save directory; exit the game before writing")
	}
	return nil
}

func writeTemp(dir, prefix string, raw []byte, mode uint32) (string, error) {
	file, err := os.CreateTemp(dir, prefix)
	if err != nil {
		return "", err
	}
	name := file.Name()
	if err = file.Chmod(os.FileMode(mode & 0o777)); err == nil {
		_, err = file.Write(raw)
	}
	if err == nil {
		err = file.Sync()
	}
	closeErr := file.Close()
	if err == nil {
		err = closeErr
	}
	if err != nil {
		_ = os.Remove(name)
		return "", err
	}
	return name, nil
}

// Commit replaces an unchanged snapshot. RENAME_EXCHANGE allows the manager to
// verify the displaced inode before unlinking it, rather than losing a narrow
// concurrent game save between check and replacement.
func Commit(snapshot *Snapshot, edited *native.Save, makeBackup bool) (CommitResult, error) {
	if snapshot == nil || edited == nil {
		return CommitResult{}, errors.New("missing save snapshot")
	}
	if err := guardGame(snapshot.Path); err != nil {
		return CommitResult{}, err
	}
	if err := unchanged(snapshot.Path, snapshot.Fingerprint); err != nil {
		return CommitResult{}, err
	}
	result := CommitResult{}
	var err error
	if makeBackup {
		result.Backup, err = backup(snapshot)
		if err != nil {
			return CommitResult{}, err
		}
	}
	if err = unchanged(snapshot.Path, snapshot.Fingerprint); err != nil {
		return CommitResult{}, err
	}
	temp, err := writeTemp(filepath.Dir(snapshot.Path), filepath.Base(snapshot.Path)+".", edited.Bytes(), snapshot.Fingerprint.Mode)
	if err != nil {
		return CommitResult{}, err
	}
	cleanup := true
	defer func() {
		if cleanup {
			_ = os.Remove(temp)
		}
	}()
	if err = unchanged(snapshot.Path, snapshot.Fingerprint); err != nil {
		return CommitResult{}, err
	}
	if err = unix.Renameat2(unix.AT_FDCWD, temp, unix.AT_FDCWD, snapshot.Path, unix.RENAME_EXCHANGE); err != nil {
		return CommitResult{}, err
	}
	// temp now points at what was previously at snapshot.Path.
	if err = unchanged(temp, snapshot.Fingerprint); err != nil {
		// Restore the unexpected current file atomically; leave the new temp for deferred cleanup.
		if rollbackErr := unix.Renameat2(unix.AT_FDCWD, temp, unix.AT_FDCWD, snapshot.Path, unix.RENAME_EXCHANGE); rollbackErr != nil {
			return CommitResult{}, fmt.Errorf("save changed during commit (%v); rollback failed: %w", err, rollbackErr)
		}
		return CommitResult{}, fmt.Errorf("save changed during commit; newer contents were restored")
	}
	if err = os.Remove(temp); err != nil {
		return CommitResult{}, err
	}
	cleanup = false
	if err = syncDir(filepath.Dir(snapshot.Path)); err != nil {
		return CommitResult{}, err
	}
	return result, nil
}

// Export writes a valid save to a new path. It uses link(2) for a no-clobber
// destination, so a concurrently created file is never overwritten.
func Export(snapshot *Snapshot, destination string, force bool) error {
	if snapshot == nil || snapshot.Save == nil {
		return errors.New("source is not a valid native save")
	}
	// Export is also used for imports and slot copies. Guard the destination,
	// not only the source, so no default save-directory write can race SRR2.
	if err := guardGame(destination); err != nil {
		return err
	}
	if err := os.MkdirAll(filepath.Dir(destination), 0o700); err != nil {
		return err
	}
	if _, err := os.Lstat(destination); err == nil && !force {
		return fmt.Errorf("destination exists: %s", destination)
	} else if err != nil && !errors.Is(err, os.ErrNotExist) {
		return err
	}
	temp, err := writeTemp(filepath.Dir(destination), filepath.Base(destination)+".", snapshot.Raw, snapshot.Fingerprint.Mode)
	if err != nil {
		return err
	}
	defer os.Remove(temp)
	if force {
		if err = unix.Rename(temp, destination); err != nil {
			return err
		}
	} else {
		if err = os.Link(temp, destination); err != nil {
			return err
		}
		if err = os.Remove(temp); err != nil {
			return err
		}
	}
	return syncDir(filepath.Dir(destination))
}

func Trash(snapshot *Snapshot) (string, error) {
	if snapshot == nil || snapshot.Save == nil {
		return "", errors.New("only valid saves can be moved to trash")
	}
	if err := guardGame(snapshot.Path); err != nil {
		return "", err
	}
	// Unlike Commit, deleting a pathname cannot be made conditionally atomic
	// against an independently running game. Refuse trashing from *any* save
	// directory while SRR2 is visible, including an explicit --save-dir.
	if processRunning() {
		return "", errors.New("SRR2 appears to be running; exit the game before moving a save to trash")
	}
	if err := unchanged(snapshot.Path, snapshot.Fingerprint); err != nil {
		return "", err
	}
	trash := filepath.Join(filepath.Dir(snapshot.Path), ".savegame-trash")
	if err := os.MkdirAll(trash, 0o700); err != nil {
		return "", err
	}
	info, err := os.Lstat(trash)
	if err != nil {
		return "", err
	}
	if !info.IsDir() || info.Mode()&os.ModeSymlink != 0 {
		return "", errors.New("unsafe trash directory")
	}
	stamp := time.Now().UTC().Format("20060102T150405Z")
	for suffix := 0; ; suffix++ {
		name := filepath.Join(trash, filepath.Base(snapshot.Path)+"."+stamp)
		if suffix > 0 {
			name = fmt.Sprintf("%s.%d", name, suffix)
		}
		// This is an atomic no-clobber move. Do not link then unlink: a game
		// could replace snapshot.Path after a check and before the unlink, which
		// would delete its newer save.
		err = unix.Renameat2(unix.AT_FDCWD, snapshot.Path, unix.AT_FDCWD, name, unix.RENAME_NOREPLACE)
		if errors.Is(err, os.ErrExist) {
			continue
		}
		if err != nil {
			return "", err
		}
		if err = unchanged(name, snapshot.Fingerprint); err != nil {
			// A replacement won the narrow race before the atomic rename. It is
			// intact in trash rather than silently deleted; do not overwrite a
			// possible newly-created source while trying to roll it back.
			return "", fmt.Errorf("%s changed during trash; moved file remains at %s for manual recovery", snapshot.Path, name)
		}
		if err = syncDir(filepath.Dir(snapshot.Path)); err != nil {
			return "", err
		}
		if err = syncDir(trash); err != nil {
			return "", err
		}
		return name, nil
	}
}

func ListTrash(dir string) ([]Slot, error) {
	trash := filepath.Join(dir, ".savegame-trash")
	entries, err := os.ReadDir(trash)
	if errors.Is(err, os.ErrNotExist) {
		return nil, nil
	}
	if err != nil {
		return nil, err
	}
	out := []Slot{}
	for _, entry := range entries {
		path := filepath.Join(trash, entry.Name())
		snapshot, err := Load(path)
		if err != nil {
			out = append(out, Slot{Path: path, Status: "invalid", Err: err})
		} else {
			out = append(out, Slot{Path: path, Status: "valid", Snapshot: snapshot})
		}
	}
	return out, nil
}

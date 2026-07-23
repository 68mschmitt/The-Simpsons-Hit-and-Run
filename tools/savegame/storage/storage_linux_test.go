package storage

import (
	"encoding/binary"
	"os"
	"path/filepath"
	"testing"

	"github.com/mschmitt/the-simpsons-hit-and-run/tools/savegame/native"
)

func fixture() []byte {
	raw := make([]byte, native.SaveSize)
	binary.LittleEndian.PutUint16(raw[0:], native.MagicNumber)
	binary.LittleEndian.PutUint16(raw[2:], 2026)
	raw[4], raw[5], raw[6], raw[7], raw[8] = 7, 22, 13, 40, 11
	raw[10], raw[11] = 4, 4
	binary.LittleEndian.PutUint32(raw[12:], native.SaveSize)
	for i := 5841; i < 7153; i++ {
		raw[i] = 0xff
	}
	return raw
}

func TestCommitBackupExportAndTrash(t *testing.T) {
	previous := processRunning
	processRunning = func() bool { return false }
	t.Cleanup(func() { processRunning = previous })

	dir := t.TempDir()
	path := filepath.Join(dir, "Save1")
	original := fixture()
	if err := os.WriteFile(path, original, 0o600); err != nil {
		t.Fatal(err)
	}
	snapshot, err := Load(path)
	if err != nil {
		t.Fatal(err)
	}
	edited, err := snapshot.Save.SetTokens(500)
	if err != nil {
		t.Fatal(err)
	}
	result, err := Commit(snapshot, edited, true)
	if err != nil {
		t.Fatal(err)
	}
	if result.Backup == "" {
		t.Fatal("missing backup")
	}
	backup, err := os.ReadFile(result.Backup)
	if err != nil {
		t.Fatal(err)
	}
	if string(backup) != string(original) {
		t.Fatal("backup differs from original")
	}
	loaded, err := Load(path)
	if err != nil {
		t.Fatal(err)
	}
	if loaded.Save.Tokens() != 500 {
		t.Fatalf("tokens = %d", loaded.Save.Tokens())
	}
	export := filepath.Join(dir, "export.srr2")
	if err := Export(loaded, export, false); err != nil {
		t.Fatal(err)
	}
	if _, err := Load(export); err != nil {
		t.Fatal(err)
	}
	trash, err := Trash(loaded)
	if err != nil {
		t.Fatal(err)
	}
	if _, err := os.Stat(path); !os.IsNotExist(err) {
		t.Fatalf("source still exists: %v", err)
	}
	if _, err := Load(trash); err != nil {
		t.Fatal(err)
	}
}

func TestDefaultSaveDirectoryWritesAreGuardedWhileGameRuns(t *testing.T) {
	xdg := t.TempDir()
	t.Setenv("XDG_DATA_HOME", xdg)
	defaultDir := DefaultDir()
	if err := os.MkdirAll(defaultDir, 0o700); err != nil {
		t.Fatal(err)
	}

	previous := processRunning
	processRunning = func() bool { return true }
	t.Cleanup(func() { processRunning = previous })

	path := filepath.Join(defaultDir, "Save1")
	if err := os.WriteFile(path, fixture(), 0o600); err != nil {
		t.Fatal(err)
	}
	snapshot, err := Load(path)
	if err != nil {
		t.Fatal(err)
	}
	edited, err := snapshot.Save.SetTokens(500)
	if err != nil {
		t.Fatal(err)
	}
	if _, err = Commit(snapshot, edited, true); err == nil {
		t.Fatal("Commit succeeded while SRR2 was running")
	}
	if _, err = Trash(snapshot); err == nil {
		t.Fatal("Trash succeeded while SRR2 was running")
	}

	sourceDir := t.TempDir()
	sourcePath := filepath.Join(sourceDir, "Save1")
	if err = os.WriteFile(sourcePath, fixture(), 0o600); err != nil {
		t.Fatal(err)
	}
	source, err := Load(sourcePath)
	if err != nil {
		t.Fatal(err)
	}
	if err = Export(source, filepath.Join(defaultDir, "Save2"), false); err == nil {
		t.Fatal("Export into the default directory succeeded while SRR2 was running")
	}
}

func TestTrashRefusesRunningGameForCustomDirectory(t *testing.T) {
	dir := t.TempDir()
	path := filepath.Join(dir, "Save1")
	if err := os.WriteFile(path, fixture(), 0o600); err != nil {
		t.Fatal(err)
	}
	snapshot, err := Load(path)
	if err != nil {
		t.Fatal(err)
	}
	previous := processRunning
	processRunning = func() bool { return true }
	t.Cleanup(func() { processRunning = previous })
	if _, err = Trash(snapshot); err == nil {
		t.Fatal("Trash succeeded in a custom directory while SRR2 was running")
	}
	if _, err = Load(path); err != nil {
		t.Fatalf("trash changed the source despite refusing it: %v", err)
	}
}

func TestRejectsSymlink(t *testing.T) {
	dir := t.TempDir()
	path := filepath.Join(dir, "Save1")
	if err := os.Symlink("missing", path); err != nil {
		t.Fatal(err)
	}
	if _, err := Load(path); err == nil {
		t.Fatal("expected symlink rejection")
	}
}

package tui

import (
	"encoding/binary"
	"os"
	"path/filepath"
	"testing"

	tea "charm.land/bubbletea/v2"
	"github.com/mschmitt/the-simpsons-hit-and-run/tools/savegame/native"
	"github.com/mschmitt/the-simpsons-hit-and-run/tools/savegame/storage"
)

func validSave(tokens int32) []byte {
	raw := make([]byte, native.SaveSize)
	binary.LittleEndian.PutUint16(raw, native.MagicNumber)
	binary.LittleEndian.PutUint16(raw[2:], 2026)
	raw[4], raw[5], raw[6], raw[7], raw[8] = 7, 22, 13, 40, 11
	raw[10], raw[11] = 1, 1
	binary.LittleEndian.PutUint32(raw[12:], native.SaveSize)
	for i := 5841; i < 7153; i++ {
		raw[i] = 0xff
	}
	save, err := native.Parse(raw)
	if err != nil {
		panic(err)
	}
	save, err = save.SetTokens(tokens)
	if err != nil {
		panic(err)
	}
	return save.Bytes()
}

func writeSave(t *testing.T, path string, tokens int32) {
	t.Helper()
	if err := os.WriteFile(path, validSave(tokens), 0o600); err != nil {
		t.Fatal(err)
	}
}

func settle(t *testing.T, m *Model, cmd tea.Cmd) {
	t.Helper()
	if cmd == nil {
		t.Fatal("expected slot reload command")
	}
	updated, next := m.Update(cmd())
	if next != nil {
		t.Fatal("slot reload scheduled an unexpected command")
	}
	model, ok := updated.(Model)
	if !ok {
		t.Fatalf("Update returned %T", updated)
	}
	*m = model
}

func loadedModel(t *testing.T, dir string) Model {
	t.Helper()
	m := New(dir)
	settle(t, &m, m.Init())
	if m.refreshing {
		t.Fatal("initial slot scan did not settle")
	}
	return m
}

func TestRebuildTableSafelySwitchesColumnCounts(t *testing.T) {
	dir := t.TempDir()
	writeSave(t, filepath.Join(dir, "Save1"), 100)
	m := loadedModel(t, dir)
	m.openSelected()

	for _, current := range []tab{gagsTab, cardsTab, missionsTab, overviewTab, gagsTab} {
		m.tab = current
		m.rebuildTable()
		if len(m.table.Columns()) == 0 || len(m.table.Rows()) == 0 {
			t.Fatalf("tab %d did not populate its table", current)
		}
	}
}

func TestCommitReloadsSlotsBeforeDashboardActions(t *testing.T) {
	dir := t.TempDir()
	writeSave(t, filepath.Join(dir, "Save1"), 100)
	m := loadedModel(t, dir)

	m.openSelected()
	if m.page != detail {
		t.Fatal("valid slot did not open")
	}
	next, err := m.working.SetTokens(500)
	if err != nil {
		t.Fatal(err)
	}
	m.stage(next, "token bank → 500", nil)
	cmd := m.commit()
	if !m.refreshing || m.page != dashboard {
		t.Fatal("successful commit did not lock dashboard pending a rescan")
	}
	settle(t, &m, cmd)
	if m.selected.Snapshot == nil || m.selected.Snapshot.Save.Tokens() != 500 {
		t.Fatalf("dashboard kept stale save after commit: %+v", m.selected.Snapshot)
	}

	cmd = m.copyTo("2")
	if !m.refreshing {
		t.Fatal("successful copy did not schedule a rescan")
	}
	settle(t, &m, cmd)
	copy, err := storage.Load(filepath.Join(dir, "Save2"))
	if err != nil {
		t.Fatal(err)
	}
	if copy.Save.Tokens() != 500 {
		t.Fatalf("copy used stale snapshot: tokens = %d", copy.Save.Tokens())
	}
}

func TestOccupiedImportRequiresConfirmation(t *testing.T) {
	dir := t.TempDir()
	destination := filepath.Join(dir, "Save1")
	writeSave(t, destination, 100)
	source := filepath.Join(t.TempDir(), "imported-save")
	writeSave(t, source, 900)
	m := loadedModel(t, dir)

	if cmd := m.prepareImport(source); cmd != nil {
		t.Fatal("occupied import wrote before confirmation")
	}
	if m.page != importConfirmModal {
		t.Fatal("occupied import did not request confirmation")
	}
	before, err := storage.Load(destination)
	if err != nil {
		t.Fatal(err)
	}
	if before.Save.Tokens() != 100 {
		t.Fatalf("occupied import overwrote before confirmation: %d", before.Save.Tokens())
	}

	cmd := m.importConfirmed()
	if !m.refreshing || m.page != dashboard {
		t.Fatal("confirmed import did not lock dashboard pending a rescan")
	}
	settle(t, &m, cmd)
	after, err := storage.Load(destination)
	if err != nil {
		t.Fatal(err)
	}
	if after.Save.Tokens() != 900 {
		t.Fatalf("imported tokens = %d", after.Save.Tokens())
	}
	backups, err := filepath.Glob(destination + ".bak*")
	if err != nil {
		t.Fatal(err)
	}
	if len(backups) != 1 {
		t.Fatalf("backup count = %d, want 1", len(backups))
	}
}

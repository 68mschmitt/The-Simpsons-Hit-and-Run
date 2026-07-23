package native

import (
	"encoding/binary"
	"testing"
)

func fixture() []byte {
	raw := make([]byte, SaveSize)
	binary.LittleEndian.PutUint16(raw[0:], MagicNumber)
	binary.LittleEndian.PutUint16(raw[2:], 2026)
	raw[4], raw[5], raw[6], raw[7], raw[8] = 7, 22, 13, 40, 11
	raw[10], raw[11] = 4, 4
	binary.LittleEndian.PutUint32(raw[12:], SaveSize)
	copy(raw[characterOffset:], "Player1")
	for i := persistentOffset; i < persistentOffset+persistentSize; i++ {
		raw[i] = 0xff
	}
	return raw
}

func TestLevelFourGagAndCardNativeOffsets(t *testing.T) {
	save, err := Parse(fixture())
	if err != nil {
		t.Fatal(err)
	}
	save, err = save.SetGag(4, 4, true)
	if err != nil {
		t.Fatal(err)
	}
	save, err = save.SetGag(4, 14, true)
	if err != nil {
		t.Fatal(err)
	}
	raw := save.Bytes()
	const viewed = 17 + 16 + 3*620 + 568
	const mask = 17 + 16 + 3*620 + 572
	if got := int32(binary.LittleEndian.Uint32(raw[viewed:])); got != 2 {
		t.Fatalf("gag count = %d", got)
	}
	if got := binary.LittleEndian.Uint32(raw[mask:]); got != 0x4010 {
		t.Fatalf("gag mask = %#x", got)
	}
	save, err = save.SetCard(4, 3, true)
	if err != nil {
		t.Fatal(err)
	}
	raw = save.Bytes()
	if raw[galleryOffset+3] != 0x04 {
		t.Fatalf("gallery byte = %#x", raw[galleryOffset+3])
	}
	mirror := 17 + 16 + 3*620 + 2*17
	if got := string(raw[mirror : mirror+6]); got != "Cardx\x00" {
		t.Fatalf("mirror name = %q", got)
	}
	if raw[mirror+16] != 1 {
		t.Fatal("card mirror is not complete")
	}
}

func TestTrailingHandlerOffsetsMatchGameRuntimeOrder(t *testing.T) {
	save, err := Parse(fixture())
	if err != nil {
		t.Fatal(err)
	}
	save, err = save.SetOption(Radar, true)
	if err != nil {
		t.Fatal(err)
	}
	music := float32(0.75)
	save, err = save.SetSound(SoundPatch{Music: &music})
	if err != nil {
		t.Fatal(err)
	}
	jump := true
	save, err = save.SetCamera(CameraPatch{JumpCameras: &jump})
	if err != nil {
		t.Fatal(err)
	}
	save, err = save.SetOption(Tutorials, true)
	if err != nil {
		t.Fatal(err)
	}
	raw := save.Bytes()
	// The current game's runtime construction order writes SoundSettings, four
	// SuperCamCentral handlers, TutorialManager, GUI, then CardGallery after
	// CharacterSheet. (This differs from the order of nearby source files.)
	if got := binary.LittleEndian.Uint32(raw[7157:]); got != 0x3f400000 {
		t.Fatalf("sound music bits = %#x", got)
	}
	if got := raw[7177]; got != 1 {
		t.Fatalf("first supercam byte = %#x", got)
	}
	if got := raw[7181]; got != 1 {
		t.Fatalf("tutorial byte = %#x", got)
	}
	if got := raw[7186]; got != 1 {
		t.Fatalf("GUI/radar byte = %#x", got)
	}
	if got := raw[7187:7194]; len(got) != 7 {
		t.Fatalf("card gallery length = %d", len(got))
	}
}

func TestSafeMutationsKeepInvariants(t *testing.T) {
	save, _ := Parse(fixture())
	if _, err := save.SetWasps(1, 21); err == nil {
		t.Fatal("expected wasp bound error")
	}
	ticket, err := save.SetOption(ItchyScratchyTicket, true)
	if err != nil {
		t.Fatal(err)
	}
	fmv, _ := ticket.FMVUnlocked(3)
	if !fmv {
		t.Fatal("ticket should unlock level 3 FMV")
	}
	ticket, err = ticket.SetOption(ItchyScratchyTicket, false)
	if err != nil {
		t.Fatal(err)
	}
	fmv, _ = ticket.FMVUnlocked(3)
	if fmv {
		t.Fatal("removing ticket should reconcile level 3 FMV")
	}
	value := float32(0.75)
	ticket, err = ticket.SetSound(SoundPatch{Music: &value})
	if err != nil {
		t.Fatal(err)
	}
	if got := binary.LittleEndian.Uint32(ticket.Bytes()[soundOffset:]); got != 0x3f400000 {
		t.Fatalf("music bits = %#x", got)
	}
}

func TestDiffReportsGagChanges(t *testing.T) {
	left, _ := Parse(fixture())
	right, err := left.SetGag(4, 4, true)
	if err != nil {
		t.Fatal(err)
	}
	diff, err := left.Diff(right)
	if err != nil {
		t.Fatal(err)
	}
	if diff.ChangedBytes == 0 || len(diff.GagChanges) != 1 {
		t.Fatalf("unexpected diff: %+v", diff)
	}
}

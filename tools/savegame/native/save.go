// Package native decodes the one native Linux save layout produced by this
// repository's CMake build. It intentionally uses explicit byte offsets and
// little-endian primitives rather than Go structs or unsafe casts: the game
// serializes C++ structs, including padding, directly to disk.
package native

import (
	"crypto/sha256"
	"encoding/binary"
	"errors"
	"fmt"
	"math"
	"strings"
)

const (
	MagicNumber uint16 = 1978
	SaveSize           = 7194
	NumSlots           = 4
	MaxLevels          = 7

	headerSize       = 16
	inputOffset      = 16
	characterOffset  = 17
	characterSize    = 7140
	levelListOffset  = 16
	levelRecordSize  = 620
	cardRecordSize   = 17
	missionSize      = 32
	missionList      = 120
	streetRaceList   = 376
	bonusMission     = 472
	fmvUnlocked      = 536
	waspsDestroyed   = 548
	gagsViewed       = 568
	gagMask          = 572
	currentMission   = characterOffset + 4356
	highestMission   = characterOffset + 4364
	navSystem        = characterOffset + 4372
	tokens           = characterOffset + 4376
	carInventory     = characterOffset + 4380
	carRecordSize    = 24
	maxCars          = 60
	carCount         = carInventory + maxCars*carRecordSize
	persistentOffset = characterOffset + 5824
	persistentSize   = 1312
	specialStates    = persistentOffset + persistentSize
	// These final handlers are registered in the game's *runtime construction*
	// order, not source-file order: SoundSettings, one SuperCamCentral per
	// player, TutorialManager, GUI, then CardGallery. Keep these explicit
	// offsets in lock-step with a game-produced save; these bytes are not part
	// of CharacterSheet.
	soundOffset    = characterOffset + characterSize
	superCamOffset = soundOffset + 20
	tutorialOffset = superCamOffset + 4
	guiOffset      = tutorialOffset + 5
	galleryOffset  = guiOffset + 1
)

// Header is the 16-byte SaveGameInfoData prefix.
type Header struct {
	Magic        uint16
	Year         uint16
	Month        uint8
	Day          uint8
	Hour         uint8
	Minute       uint8
	Second       uint8
	Level        uint8
	Mission      uint8
	DeclaredSize uint32
}

func (h Header) Timestamp() string {
	return fmt.Sprintf("%04d-%02d-%02d %02d:%02d:%02d", h.Year, h.Month, h.Day, h.Hour, h.Minute, h.Second)
}

// Save owns a validated raw save byte slice. All mutations return a new Save.
type Save struct {
	raw    []byte
	header Header
}

// Parse validates the native 64-bit Linux format and takes a private copy.
func Parse(raw []byte) (*Save, error) {
	if len(raw) != SaveSize {
		return nil, fmt.Errorf("unsupported save size %d (expected %d)", len(raw), SaveSize)
	}
	h := Header{
		Magic: binary.LittleEndian.Uint16(raw[0:2]),
		Year:  binary.LittleEndian.Uint16(raw[2:4]),
		Month: raw[4], Day: raw[5], Hour: raw[6], Minute: raw[7], Second: raw[8],
		Level: raw[10], Mission: raw[11], DeclaredSize: binary.LittleEndian.Uint32(raw[12:16]),
	}
	if h.Magic != MagicNumber {
		return nil, fmt.Errorf("unsupported magic %d (expected %d)", h.Magic, MagicNumber)
	}
	if h.DeclaredSize != SaveSize {
		return nil, fmt.Errorf("header declares %d bytes (expected %d)", h.DeclaredSize, SaveSize)
	}
	if h.Month < 1 || h.Month > 12 || h.Day < 1 || h.Day > 31 {
		return nil, errors.New("header has an invalid month or day")
	}
	copyRaw := append([]byte(nil), raw...)
	return &Save{raw: copyRaw, header: h}, nil
}

func (s *Save) Header() Header { return s.header }
func (s *Save) Bytes() []byte  { return append([]byte(nil), s.raw...) }
func (s *Save) SHA256() string { return fmt.Sprintf("%x", sha256.Sum256(s.raw)) }

func levelBase(level int) (int, error) {
	if level < 1 || level > MaxLevels {
		return 0, fmt.Errorf("level must be from 1 to %d", MaxLevels)
	}
	return characterOffset + levelListOffset + (level-1)*levelRecordSize, nil
}

func (s *Save) clone(change func([]byte) error) (*Save, error) {
	raw := s.Bytes()
	if err := change(raw); err != nil {
		return nil, err
	}
	return Parse(raw)
}

func stringAt(raw []byte, offset, length int) string {
	end := offset
	for end < offset+length && raw[end] != 0 {
		end++
	}
	return string(raw[offset:end])
}

func writeString(raw []byte, offset, length int, value string) error {
	if len(value) >= length {
		return fmt.Errorf("%q does not fit in %d bytes", value, length-1)
	}
	for _, r := range value {
		if r > 127 {
			return fmt.Errorf("%q is not ASCII", value)
		}
	}
	copy(raw[offset:offset+length], value)
	for i := offset + len(value); i < offset+length; i++ {
		raw[i] = 0
	}
	return nil
}

func int32At(raw []byte, offset int) int32 { return int32(binary.LittleEndian.Uint32(raw[offset:])) }
func putInt32(raw []byte, offset int, value int32) {
	binary.LittleEndian.PutUint32(raw[offset:], uint32(value))
}

// GagDefinition is the stock script's ordered persistent gag list.
type GagDefinition struct {
	ID      string
	Label   string
	Aliases []string
}

var gagLists = [][]GagDefinition{
	{{"barbecue", "Barbecue", []string{"bbq"}}, {"poison-gas", "Poison gas", []string{"gas"}}, {"bomb-shelter", "Flanders's bomb shelter", []string{"shel", "shelter"}}, {"homer-swing", "Homer's-yard swing set", []string{"swng", "swing"}}, {"park-swing", "Park swing set", []string{"swg2"}}, {"squishee", "Squishee machine (Kwik-E-Mart)", []string{"sqsh"}}, {"jasper", "Jasper in the freezer", []string{"jasp"}}, {"atm", "ATM machine", nil}, {"tv", "Simpsons House TV", []string{"television"}}, {"aztec-fire", "Aztec fire", []string{"azte"}}, {"power-plant-meltdown", "Power Plant meltdown", []string{"melt"}}, {"fire-extinguisher", "Dead fire extinguisher", []string{"fire"}}, {"fire-alarm", "Springfield Elementary fire alarm", []string{"sknr"}}, {"looter-video", "Larry the Looter video", []string{"tele", "larry"}}, {"silent-alarm", "Kwik-E-Mart silent alarm", []string{"alm2"}}},
	{{"pickle-jar", "Pickle jar", []string{"jar"}}, {"flaming-moe", "Flaming Moe", []string{"flm"}}, {"love-tester", "Love tester", []string{"love"}}, {"slot-machine", "Slot machine", []string{"slot"}}, {"catapult", "Catapult", []string{"cata"}}, {"fat-tony-construction", "Fat Tony's construction", []string{"tony"}}, {"hermans-bomb", "Herman's bomb", []string{"bomb"}}, {"rat-milker", "Rat milker machine", []string{"milk"}}, {"exploding-car", "Exploding car outside Try-N-Save", []string{"car"}}, {"moleman-dmv", "Moleman DMV photo", []string{"mol2"}}, {"dumpster", "Level 2 dumpster", []string{"l2-dump"}}},
	{{"radioactive-man", "Radioactive Man", []string{"racm"}}, {"clank-clank", "Clank Clank robot", []string{"clnk"}}, {"kamp-krusty-flag", "Kamp Krusty flag", []string{"flag"}}, {"kamp-krusty-pig", "Kamp Krusty pig on a stick", []string{"pig"}}, {"observatory-alarm", "Observatory silent alarm", []string{"alrm"}}, {"matter-transporter", "Matter transporter", []string{"matt"}}, {"perpetual-motion", "Perpetual motion machine", []string{"perp"}}, {"hot-pants-crane", "Hot Pants-spilling crane", []string{"pant"}}, {"wally-weasel-kids", "Wally Weasel ball-room kids", []string{"kids"}}, {"observatory-video", "Observatory telescope video", []string{"tele"}}, {"dumpster", "Level 3 dumpster", []string{"l3-dump"}}},
	{{"barbecue", "Barbecue", []string{"bbq"}}, {"poison-gas", "Poison gas", []string{"gas"}}, {"bomb-shelter", "Flanders's bomb shelter", []string{"shel", "shelter"}}, {"homer-swing", "Homer's-yard swing set", []string{"swng", "swing"}}, {"squishee", "Squishee machine (Kwik-E-Mart)", []string{"sqsh"}}, {"jasper", "Jasper in the freezer", []string{"jasp"}}, {"atm", "ATM machine", nil}, {"tv", "Simpsons House TV", []string{"television"}}, {"aztec-fire", "Aztec fire", []string{"azte"}}, {"power-plant-meltdown", "Power Plant meltdown", []string{"melt"}}, {"fire-extinguisher", "Dead fire extinguisher", []string{"fire"}}, {"fire-alarm", "Springfield Elementary fire alarm", []string{"sknr"}}, {"exploding-lamp", "Exploding lamp in Bart's room", []string{"lamp"}}, {"looter-video", "Larry the Looter video", []string{"tele", "larry"}}, {"silent-alarm", "Kwik-E-Mart silent alarm", []string{"alm2"}}},
	{{"pickle-jar", "Pickle jar", []string{"jar"}}, {"flaming-moe", "Flaming Moe", []string{"flm"}}, {"love-tester", "Love tester", []string{"love"}}, {"slot-machine", "Slot machine", []string{"slot"}}, {"moleman-dmv", "Moleman DMV photo", []string{"mol2"}}, {"dumpster", "Level 5 dumpster", []string{"l5-dump"}}},
	{{"radioactive-man", "Radioactive Man", []string{"racm"}}, {"clank-clank", "Clank Clank robot", []string{"clnk"}}, {"kamp-krusty-flag", "Kamp Krusty flag", []string{"flag"}}, {"kamp-krusty-pig", "Kamp Krusty pig on a stick", []string{"pig"}}, {"observatory-alarm", "Observatory silent alarm", []string{"alrm"}}, {"matter-transporter", "Matter transporter", []string{"matt"}}, {"perpetual-motion", "Perpetual motion machine", []string{"perp"}}, {"hot-pants-crane", "Hot Pants-spilling crane", []string{"pant"}}, {"wally-weasel-kids", "Wally Weasel ball-room kids", []string{"kids"}}, {"observatory-video", "Observatory telescope video", []string{"tele"}}, {"dumpster", "Level 6 dumpster", []string{"l6-dump"}}},
	{{"poison-gas", "Poison gas", []string{"gas"}}, {"homer-swing", "Homer's-yard swing set", []string{"swng", "swing"}}, {"park-swing", "Park swing set", []string{"swg2"}}, {"squishee", "Squishee machine (Kwik-E-Mart)", []string{"sqsh"}}, {"jasper", "Jasper in the freezer", []string{"jasp"}}, {"atm", "ATM machine", nil}, {"tv", "Simpsons House TV", []string{"television"}}, {"aztec-fire", "Aztec fire", []string{"azte"}}, {"fire-extinguisher", "Dead fire extinguisher", []string{"fire"}}, {"fire-alarm", "Springfield Elementary fire alarm", []string{"sknr"}}, {"looter-video", "Larry the Looter video", []string{"tele", "larry"}}, {"silent-alarm", "Kwik-E-Mart silent alarm", []string{"alm2"}}, {"exploding-lamp", "Exploding lamp in Bart's room", []string{"lamp"}}, {"clown-bed", "Scary clown bed in Bart's room", []string{"bed"}}, {"bomb-shelter", "Flanders's bomb shelter", []string{"shel", "shelter"}}},
}

func Gags(level int) ([]GagDefinition, error) {
	if level < 1 || level > MaxLevels {
		return nil, fmt.Errorf("level must be from 1 to %d", MaxLevels)
	}
	return append([]GagDefinition(nil), gagLists[level-1]...), nil
}

func normalize(value string) string {
	var b strings.Builder
	for _, r := range strings.ToLower(value) {
		if (r >= 'a' && r <= 'z') || (r >= '0' && r <= '9') {
			b.WriteRune(r)
		}
	}
	return b.String()
}

// ResolveGag accepts a one-based number, index:N, stock ID, label, or alias.
func ResolveGag(level int, selector string) (int, error) {
	gags, err := Gags(level)
	if err != nil {
		return 0, err
	}
	if strings.HasPrefix(selector, "index:") {
		var index int
		if _, err := fmt.Sscanf(strings.TrimPrefix(selector, "index:"), "%d", &index); err != nil || index < 0 || index >= len(gags) {
			return 0, fmt.Errorf("invalid gag index %q", selector)
		}
		return index, nil
	}
	var ordinal int
	if _, err := fmt.Sscanf(selector, "%d", &ordinal); err == nil && fmt.Sprintf("%d", ordinal) == selector {
		if ordinal < 1 || ordinal > len(gags) {
			return 0, fmt.Errorf("level %d has gag numbers 1 through %d", level, len(gags))
		}
		return ordinal - 1, nil
	}
	needle := normalize(selector)
	for index, gag := range gags {
		if needle == normalize(gag.ID) || needle == normalize(gag.Label) {
			return index, nil
		}
		for _, alias := range gag.Aliases {
			if needle == normalize(alias) {
				return index, nil
			}
		}
	}
	return 0, fmt.Errorf("no level %d gag matches %q", level, selector)
}

type GagState struct {
	Viewed int32
	Mask   uint32
}
type GagEntry struct {
	Index      int
	Definition GagDefinition
	Completed  bool
}

func (s *Save) GagState(level int) (GagState, error) {
	base, err := levelBase(level)
	if err != nil {
		return GagState{}, err
	}
	return GagState{Viewed: int32At(s.raw, base+gagsViewed), Mask: binary.LittleEndian.Uint32(s.raw[base+gagMask:])}, nil
}
func (s *Save) GagEntries(level int) ([]GagEntry, error) {
	state, err := s.GagState(level)
	if err != nil {
		return nil, err
	}
	gags, _ := Gags(level)
	entries := make([]GagEntry, len(gags))
	for i, gag := range gags {
		entries[i] = GagEntry{Index: i, Definition: gag, Completed: state.Mask&(1<<i) != 0}
	}
	return entries, nil
}

func (s *Save) SetGag(level, index int, completed bool) (*Save, error) {
	gags, err := Gags(level)
	if err != nil {
		return nil, err
	}
	if index < 0 || index >= len(gags) {
		return nil, fmt.Errorf("invalid level %d gag index %d", level, index)
	}
	base, _ := levelBase(level)
	return s.clone(func(raw []byte) error {
		mask := binary.LittleEndian.Uint32(raw[base+gagMask:])
		bit := uint32(1 << index)
		if completed {
			mask |= bit
		} else {
			mask &^= bit
		}
		binary.LittleEndian.PutUint32(raw[base+gagMask:], mask)
		putInt32(raw, base+gagsViewed, int32(bitsSet(mask)))
		return nil
	})
}
func (s *Save) SetAllGags(level int, completed bool) (*Save, error) {
	gags, err := Gags(level)
	if err != nil {
		return nil, err
	}
	base, _ := levelBase(level)
	known := uint32((1 << len(gags)) - 1)
	return s.clone(func(raw []byte) error {
		mask := binary.LittleEndian.Uint32(raw[base+gagMask:])
		if completed {
			mask |= known
		} else {
			mask &^= known
		}
		binary.LittleEndian.PutUint32(raw[base+gagMask:], mask)
		putInt32(raw, base+gagsViewed, int32(bitsSet(mask)))
		return nil
	})
}
func (s *Save) RepairGagCounts() (*Save, []int, error) {
	changed := []int{}
	next, err := s.clone(func(raw []byte) error {
		for level := 1; level <= MaxLevels; level++ {
			base, _ := levelBase(level)
			count := int32At(raw, base+gagsViewed)
			mask := binary.LittleEndian.Uint32(raw[base+gagMask:])
			wanted := int32(bitsSet(mask))
			if count != wanted {
				putInt32(raw, base+gagsViewed, wanted)
				changed = append(changed, level)
			}
		}
		return nil
	})
	return next, changed, err
}
func bitsSet(value uint32) int {
	count := 0
	for value != 0 {
		value &= value - 1
		count++
	}
	return count
}

type Card struct {
	Slot            int
	Gallery         bool
	MirrorName      string
	MirrorCompleted bool
}

func (s *Save) Cards(level int) ([]Card, error) {
	base, err := levelBase(level)
	if err != nil {
		return nil, err
	}
	cards := make([]Card, 7)
	gallery := s.raw[galleryOffset+level-1]
	for i := range cards {
		offset := base + i*cardRecordSize
		cards[i] = Card{i + 1, gallery&(1<<i) != 0, stringAt(s.raw, offset, 16), s.raw[offset+16] != 0}
	}
	return cards, nil
}
func (s *Save) SetCard(level, slot int, completed bool) (*Save, error) {
	if slot < 1 || slot > 7 {
		return nil, errors.New("card slot must be from 1 to 7")
	}
	base, err := levelBase(level)
	if err != nil {
		return nil, err
	}
	return s.clone(func(raw []byte) error {
		bit := byte(1 << (slot - 1))
		goff := galleryOffset + level - 1
		if completed {
			raw[goff] |= bit
		} else {
			raw[goff] &^= bit
		}
		offset := base + (slot-1)*cardRecordSize
		if err := writeString(raw, offset, 16, map[bool]string{true: "Cardx", false: "NULL"}[completed]); err != nil {
			return err
		}
		if completed {
			raw[offset+16] = 1
		} else {
			raw[offset+16] = 0
		}
		return nil
	})
}
func (s *Save) SetAllCards(level int, completed bool) (*Save, error) {
	next := s
	var err error
	for slot := 1; slot <= 7; slot++ {
		next, err = next.SetCard(level, slot, completed)
		if err != nil {
			return nil, err
		}
	}
	return next, nil
}

type MissionRecord struct {
	Slot           int
	Name           string
	Completed      bool
	BonusObjective bool
	Attempts       uint32
	Skipped        bool
	BestTime       int32
}
type RecordKind string

const (
	Mission RecordKind = "mission"
	Race    RecordKind = "race"
	Bonus   RecordKind = "bonus"
)

func recordOffset(level int, kind RecordKind, slot int) (int, error) {
	base, err := levelBase(level)
	if err != nil {
		return 0, err
	}
	var start, count int
	switch kind {
	case Mission:
		start, count = missionList, 8
	case Race:
		start, count = streetRaceList, 3
	case Bonus:
		start, count = bonusMission, 1
	default:
		return 0, errors.New("unknown record kind")
	}
	if slot < 1 || slot > count {
		return 0, fmt.Errorf("%s slot must be from 1 to %d", kind, count)
	}
	return base + start + (slot-1)*missionSize, nil
}
func readRecord(raw []byte, offset, slot int) MissionRecord {
	return MissionRecord{slot, stringAt(raw, offset, 16), raw[offset+16] != 0, raw[offset+17] != 0, binary.LittleEndian.Uint32(raw[offset+20:]), raw[offset+24] != 0, int32At(raw, offset+28)}
}
func (s *Save) Records(level int, kind RecordKind) ([]MissionRecord, error) {
	var count int
	switch kind {
	case Mission:
		count = 8
	case Race:
		count = 3
	case Bonus:
		count = 1
	default:
		return nil, errors.New("unknown record kind")
	}
	records := make([]MissionRecord, count)
	for i := range records {
		offset, err := recordOffset(level, kind, i+1)
		if err != nil {
			return nil, err
		}
		records[i] = readRecord(s.raw, offset, i+1)
	}
	return records, nil
}
func (s *Save) SetRecordCompleted(level int, kind RecordKind, slot int, completed bool) (*Save, string, error) {
	offset, err := recordOffset(level, kind, slot)
	if err != nil {
		return nil, "", err
	}
	name := stringAt(s.raw, offset, 16)
	if name == "" || name == "NULL" {
		return nil, "", fmt.Errorf("level %d %s slot %d has no named record", level, kind, slot)
	}
	next, err := s.clone(func(raw []byte) error {
		if completed {
			raw[offset+16] = 1
		} else {
			raw[offset+16] = 0
		}
		return nil
	})
	return next, name, err
}

func (s *Save) Tokens() int32 { return int32At(s.raw, tokens) }
func (s *Save) SetTokens(value int32) (*Save, error) {
	if value < 0 {
		return nil, errors.New("tokens cannot be negative")
	}
	return s.clone(func(raw []byte) error { putInt32(raw, tokens, value); return nil })
}
func (s *Save) Wasps(level int) (int32, error) {
	base, err := levelBase(level)
	if err != nil {
		return 0, err
	}
	return int32At(s.raw, base+waspsDestroyed), nil
}
func (s *Save) SetWasps(level int, value int32) (*Save, error) {
	if value < 0 || value > 20 {
		return nil, errors.New("retail wasp count must be from 0 to 20")
	}
	base, err := levelBase(level)
	if err != nil {
		return nil, err
	}
	return s.clone(func(raw []byte) error { putInt32(raw, base+waspsDestroyed, value); return nil })
}
func (s *Save) FMVUnlocked(level int) (bool, error) {
	base, err := levelBase(level)
	if err != nil {
		return false, err
	}
	return s.raw[base+fmvUnlocked] != 0, nil
}
func (s *Save) SetFMV(level int, unlocked bool) (*Save, error) {
	base, err := levelBase(level)
	if err != nil {
		return nil, err
	}
	return s.clone(func(raw []byte) error {
		if unlocked {
			raw[base+fmvUnlocked] = 1
		} else {
			raw[base+fmvUnlocked] = 0
		}
		return nil
	})
}

type Option string

const (
	Rumble                    Option = "rumble"
	Radar                     Option = "radar"
	Navigation                Option = "navigation"
	Tutorials                 Option = "tutorials"
	Surround                  Option = "surround"
	ItchyScratchyTicket       Option = "itchy-scratchy-ticket"
	ItchyScratchyInstructions Option = "itchy-scratchy-instructions"
)

func (s *Save) SetOption(option Option, enabled bool) (*Save, error) {
	return s.clone(func(raw []byte) error {
		var offset int
		switch option {
		case Rumble:
			offset = inputOffset
		case Radar:
			offset = guiOffset
		case Navigation:
			offset = navSystem
		case Tutorials:
			offset = tutorialOffset
		case Surround:
			offset = soundOffset + 16
		case ItchyScratchyTicket, ItchyScratchyInstructions:
			bit := byte(1)
			if option == ItchyScratchyTicket {
				bit = 2
			}
			if enabled {
				raw[specialStates] |= bit
			} else {
				raw[specialStates] &^= bit
			}
			if option == ItchyScratchyTicket {
				base, _ := levelBase(3)
				if enabled {
					raw[base+fmvUnlocked] = 1
				} else {
					raw[base+fmvUnlocked] = 0
				}
			}
			return nil
		default:
			return fmt.Errorf("unknown option %q", option)
		}
		if enabled {
			raw[offset] = 1
		} else {
			raw[offset] = 0
		}
		return nil
	})
}

type CameraPatch struct {
	JumpCameras *bool
	Inverted    *bool
	Preferred   *string
}

func (s *Save) SetCamera(patch CameraPatch) (*Save, error) {
	return s.clone(func(raw []byte) error {
		v := raw[superCamOffset]
		if patch.JumpCameras != nil {
			if *patch.JumpCameras {
				v |= 1
			} else {
				v &^= 1
			}
		}
		if patch.Inverted != nil {
			if *patch.Inverted {
				v |= 2
			} else {
				v &^= 2
			}
		}
		if patch.Preferred != nil {
			types := map[string]byte{"near": 2, "far": 3, "bumper": 4, "comedy": 9}
			kind, ok := types[*patch.Preferred]
			if !ok {
				return errors.New("camera must be near, far, bumper, or comedy")
			}
			v = (v & 3) | (kind << 2)
		}
		raw[superCamOffset] = v
		return nil
	})
}

type SoundPatch struct {
	Music, SFX, Car, Dialogue *float32
	Surround                  *bool
}

func (s *Save) SetSound(p SoundPatch) (*Save, error) {
	return s.clone(func(raw []byte) error {
		vals := []*float32{p.Music, p.SFX, p.Car, p.Dialogue}
		for i, v := range vals {
			if v == nil {
				continue
			}
			if *v < 0 || *v > 1 {
				return errors.New("volume must be between 0.0 and 1.0")
			}
			binary.LittleEndian.PutUint32(raw[soundOffset+i*4:], math.Float32bits(*v))
		}
		if p.Surround != nil {
			if *p.Surround {
				raw[soundOffset+16] = 1
			} else {
				raw[soundOffset+16] = 0
			}
		}
		return nil
	})
}
func (s *Save) RepairCars() (*Save, int, error) {
	count := int32At(s.raw, carCount)
	if count < 0 || count > maxCars {
		return nil, 0, fmt.Errorf("invalid car inventory count %d", count)
	}
	fixed := 0
	next, err := s.clone(func(raw []byte) error {
		for i := 0; i < int(count); i++ {
			offset := carInventory + i*carRecordSize + 16
			current := math.Float32frombits(binary.LittleEndian.Uint32(raw[offset:]))
			if current != 1 {
				binary.LittleEndian.PutUint32(raw[offset:], math.Float32bits(1))
				fixed++
			}
		}
		return nil
	})
	return next, fixed, err
}

// Summary is a compact detail view suitable for the TUI dashboard.
type Summary struct {
	Player                       string
	Tokens                       int32
	CurrentLevel, CurrentMission int
	HighestLevel, HighestMission int
	Radar, Navigation            bool
	Cars                         int32
	PersistentBroken             int
	LevelStats                   []LevelSummary
}
type LevelSummary struct {
	Level, Cards, Gags, GagTotal int
	Wasps                        int32
	FMV                          bool
	Missions                     []MissionRecord
	Races                        []MissionRecord
}

func (s *Save) Summary() (Summary, error) {
	out := Summary{Player: stringAt(s.raw, characterOffset, 16), Tokens: s.Tokens(), CurrentLevel: int(int32At(s.raw, currentMission)) + 1, CurrentMission: int(int32At(s.raw, currentMission+4)) + 1, HighestLevel: int(int32At(s.raw, highestMission)) + 1, HighestMission: int(int32At(s.raw, highestMission+4)) + 1, Radar: s.raw[guiOffset] != 0, Navigation: s.raw[navSystem] != 0, Cars: int32At(s.raw, carCount)}
	for _, b := range s.raw[persistentOffset : persistentOffset+persistentSize] {
		out.PersistentBroken += 8 - bitsSet(uint32(b))
	}
	for level := 1; level <= MaxLevels; level++ {
		cards, _ := s.Cards(level)
		count := 0
		for _, card := range cards {
			if card.Gallery {
				count++
			}
		}
		state, _ := s.GagState(level)
		fmv, _ := s.FMVUnlocked(level)
		wasps, _ := s.Wasps(level)
		missions, _ := s.Records(level, Mission)
		races, _ := s.Records(level, Race)
		out.LevelStats = append(out.LevelStats, LevelSummary{level, count, bitsSet(state.Mask), len(gagLists[level-1]), wasps, fmv, missions, races})
	}
	return out, nil
}

type ByteRange struct{ Start, End int }
type Diff struct {
	ChangedBytes int
	Ranges       []ByteRange
	GagChanges   []string
}

func (s *Save) Diff(other *Save) (Diff, error) {
	d := Diff{}
	start := -1
	for i := range s.raw {
		if s.raw[i] != other.raw[i] {
			d.ChangedBytes++
			if start < 0 {
				start = i
			}
		} else if start >= 0 {
			d.Ranges = append(d.Ranges, ByteRange{start, i - 1})
			start = -1
		}
	}
	if start >= 0 {
		d.Ranges = append(d.Ranges, ByteRange{start, len(s.raw) - 1})
	}
	for level := 1; level <= MaxLevels; level++ {
		left, _ := s.GagState(level)
		right, _ := other.GagState(level)
		for i, g := range gagLists[level-1] {
			if (left.Mask&(1<<i) != 0) != (right.Mask&(1<<i) != 0) {
				d.GagChanges = append(d.GagChanges, fmt.Sprintf("L%d %s: %t → %t", level, g.ID, left.Mask&(1<<i) != 0, right.Mask&(1<<i) != 0))
			}
		}
	}
	return d, nil
}

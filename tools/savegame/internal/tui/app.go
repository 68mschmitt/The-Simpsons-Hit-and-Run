// Package tui provides the interactive Charm-powered save manager.
package tui

import (
	"fmt"
	"os"
	"strconv"
	"strings"
	"time"

	"charm.land/bubbles/v2/list"
	"charm.land/bubbles/v2/table"
	"charm.land/bubbles/v2/textinput"
	tea "charm.land/bubbletea/v2"
	"charm.land/lipgloss/v2"
	"github.com/mschmitt/the-simpsons-hit-and-run/tools/savegame/native"
	"github.com/mschmitt/the-simpsons-hit-and-run/tools/savegame/storage"
)

type slotItem struct{ slot storage.Slot }

func (i slotItem) FilterValue() string {
	return fmt.Sprintf("slot %d %s", i.slot.Number, i.slot.Status)
}
func (i slotItem) Title() string {
	return fmt.Sprintf("Slot %d  ·  %s", i.slot.Number, strings.ToUpper(i.slot.Status))
}
func (i slotItem) Description() string {
	if i.slot.Status == "valid" {
		h := i.slot.Snapshot.Save.Header()
		return fmt.Sprintf("%s  ·  L%d M%d", h.Timestamp(), h.Level, h.Mission)
	}
	if i.slot.Err != nil {
		return i.slot.Err.Error()
	}
	return "Empty slot — ready for an import or copy"
}

type page int

const (
	dashboard page = iota
	detail
	inputModal
	confirmModal
	importConfirmModal
	deleteModal
)

type tab int

const (
	overviewTab tab = iota
	gagsTab
	cardsTab
	missionsTab
)

type slotsMsg struct {
	slots  []storage.Slot
	status string
}
type errMsg struct{ err error }

type Model struct {
	dir           string
	width, height int
	page          page
	tab           tab
	level         int
	slots         []storage.Slot
	list          list.Model
	table         table.Model
	selected      storage.Slot
	original      *storage.Snapshot
	working       *native.Save
	dirty         []string
	input         textinput.Model
	inputKind     string
	importSource  *storage.Snapshot
	importPath    string
	refreshing    bool
	status        string
	err           error
}

var (
	ink         = lipgloss.Color("#EDEFF5")
	muted       = lipgloss.Color("#8E9AAF")
	purple      = lipgloss.Color("#BFA3FF")
	cyan        = lipgloss.Color("#64D7E8")
	green       = lipgloss.Color("#83E69A")
	red         = lipgloss.Color("#FF7B8D")
	panelStyle  = lipgloss.NewStyle().Border(lipgloss.RoundedBorder()).BorderForeground(lipgloss.Color("#3B4261")).Padding(0, 1)
	titleStyle  = lipgloss.NewStyle().Bold(true).Foreground(purple)
	mutedStyle  = lipgloss.NewStyle().Foreground(muted)
	okStyle     = lipgloss.NewStyle().Foreground(green)
	badStyle    = lipgloss.NewStyle().Foreground(red)
	footerStyle = lipgloss.NewStyle().Foreground(muted).Padding(0, 1)
)

func New(dir string) Model {
	delegate := list.NewDefaultDelegate()
	delegate.SetHeight(3)
	l := list.New(nil, delegate, 42, 18)
	l.Title = "Native save slots"
	l.SetShowStatusBar(false)
	l.SetFilteringEnabled(false)
	t := table.New(table.WithColumns([]table.Column{{Title: "Field", Width: 22}, {Title: "Value", Width: 48}}), table.WithRows(nil), table.WithHeight(16), table.WithWidth(76))
	ti := textinput.New()
	ti.Prompt = ""
	ti.SetWidth(54)
	return Model{dir: dir, page: dashboard, level: 1, list: l, table: t, input: ti, refreshing: true, status: "Loading slots…"}
}

func (m Model) Init() tea.Cmd { return loadSlots(m.dir, "") }
func loadSlots(dir, status string) tea.Cmd {
	return func() tea.Msg { return slotsMsg{slots: storage.Slots(dir), status: status} }
}

func (m *Model) setSlots(slots []storage.Slot, status string) {
	m.slots = slots
	items := make([]list.Item, 0, len(slots))
	for _, slot := range slots {
		items = append(items, slotItem{slot})
	}
	_ = m.list.SetItems(items)
	if len(slots) > 0 {
		m.selected = slots[m.list.Index()]
	}
	m.refreshing = false
	if status != "" {
		m.status = status
	} else {
		m.status = fmt.Sprintf("%s · %d slots scanned", m.dir, len(slots))
	}
}
func (m *Model) refreshSelection() {
	if item, ok := m.list.SelectedItem().(slotItem); ok {
		m.selected = item.slot
	}
}
func (m *Model) openSelected() {
	m.refreshSelection()
	if m.selected.Status != "valid" {
		m.status = "Choose a valid slot to inspect, or use i to import into an empty slot"
		return
	}
	m.original = m.selected.Snapshot
	m.working = m.original.Save
	m.dirty = nil
	m.tab = overviewTab
	h := m.working.Header()
	if h.Level >= 1 && h.Level <= native.MaxLevels {
		m.level = int(h.Level)
	} else {
		m.level = 1
	}
	m.page = detail
	m.rebuildTable()
}
func (m *Model) rebuildTable() {
	if m.working == nil {
		return
	}
	rows := []table.Row{}
	columns := []table.Column{}
	switch m.tab {
	case overviewTab:
		s, _ := m.working.Summary()
		columns = []table.Column{{Title: "Field", Width: 24}, {Title: "Value", Width: 56}}
		rows = append(rows, table.Row{"Player", fallback(s.Player, "(unnamed)")}, table.Row{"Save timestamp", m.working.Header().Timestamp()}, table.Row{"Progress", fmt.Sprintf("Current L%d M%d · highest L%d M%d", s.CurrentLevel, s.CurrentMission, s.HighestLevel, s.HighestMission)}, table.Row{"Token bank", fmt.Sprintf("%d", s.Tokens)}, table.Row{"Navigation / radar", fmt.Sprintf("%s / %s", onOff(s.Navigation), onOff(s.Radar))}, table.Row{"Owned cars", fmt.Sprintf("%d", s.Cars)}, table.Row{"Persistent objects", fmt.Sprintf("%d broken or removed", s.PersistentBroken)})
		for _, l := range s.LevelStats {
			rows = append(rows, table.Row{fmt.Sprintf("Level %d", l.Level), fmt.Sprintf("cards %d/7 · gags %d/%d · wasps %d/20 · FMV %s", l.Cards, l.Gags, l.GagTotal, l.Wasps, onOff(l.FMV))})
		}
	case gagsTab:
		columns = []table.Column{{Title: "#", Width: 4}, {Title: "State", Width: 10}, {Title: "Gag", Width: 28}, {Title: "Description", Width: 38}}
		entries, _ := m.working.GagEntries(m.level)
		for _, entry := range entries {
			state := "missing"
			if entry.Completed {
				state = "done"
			}
			rows = append(rows, table.Row{strconv.Itoa(entry.Index + 1), state, entry.Definition.ID, entry.Definition.Label})
		}
	case cardsTab:
		columns = []table.Column{{Title: "Card", Width: 8}, {Title: "Gallery", Width: 12}, {Title: "Mirror", Width: 12}, {Title: "Record", Width: 38}}
		cards, _ := m.working.Cards(m.level)
		for _, card := range cards {
			rows = append(rows, table.Row{strconv.Itoa(card.Slot), onOff(card.Gallery), onOff(card.MirrorCompleted), fallback(card.MirrorName, "NULL")})
		}
	case missionsTab:
		columns = []table.Column{{Title: "#", Width: 4}, {Title: "State", Width: 12}, {Title: "Record", Width: 30}, {Title: "Attempts", Width: 10}, {Title: "Best", Width: 10}}
		records, _ := m.working.Records(m.level, native.Mission)
		for _, record := range records {
			state := "missing"
			if record.Completed {
				state = "done"
			}
			name := record.Name
			if name == "" {
				name = "NULL"
			}
			best := "—"
			if record.BestTime >= 0 {
				best = fmt.Sprintf("%ds", record.BestTime)
			}
			rows = append(rows, table.Row{strconv.Itoa(record.Slot), state, name, fmt.Sprintf("%d", record.Attempts), best})
		}
	}
	// bubbles/table renders immediately in SetColumns. Clear rows first so a
	// tab with more columns never tries to render the previous tab's narrower
	// rows (for example, overview's two fields as a four-column gag row).
	m.table.SetRows(nil)
	m.table.SetColumns(columns)
	m.table.SetRows(rows)
	m.table.SetCursor(0)
}
func onOff(v bool) string {
	if v {
		return "on"
	}
	return "off"
}
func fallback(value, other string) string {
	if value == "" {
		return other
	}
	return value
}
func (m *Model) stage(next *native.Save, note string, err error) {
	if err != nil {
		m.err = err
		m.status = "Edit rejected: " + err.Error()
		return
	}
	m.working = next
	m.dirty = append(m.dirty, note)
	m.status = "Staged: " + note
	m.rebuildTable()
}
func (m *Model) toggleRow() {
	if m.working == nil {
		return
	}
	row := m.table.Cursor()
	switch m.tab {
	case gagsTab:
		entries, _ := m.working.GagEntries(m.level)
		if row >= len(entries) {
			return
		}
		entry := entries[row]
		next, err := m.working.SetGag(m.level, entry.Index, !entry.Completed)
		m.stage(next, fmt.Sprintf("L%d gag %s → %s", m.level, entry.Definition.ID, map[bool]string{true: "done", false: "missing"}[!entry.Completed]), err)
	case cardsTab:
		cards, _ := m.working.Cards(m.level)
		if row >= len(cards) {
			return
		}
		card := cards[row]
		next, err := m.working.SetCard(m.level, card.Slot, !card.Gallery)
		m.stage(next, fmt.Sprintf("L%d card %d → %s", m.level, card.Slot, map[bool]string{true: "done", false: "missing"}[!card.Gallery]), err)
	case missionsTab:
		records, _ := m.working.Records(m.level, native.Mission)
		if row >= len(records) || records[row].Name == "" || records[row].Name == "NULL" {
			m.status = "This mission slot has no assigned record"
			return
		}
		record := records[row]
		next, name, err := m.working.SetRecordCompleted(m.level, native.Mission, record.Slot, !record.Completed)
		m.stage(next, fmt.Sprintf("L%d mission %s → %s", m.level, name, map[bool]string{true: "done", false: "missing"}[!record.Completed]), err)
	}
}
func (m *Model) openInput(kind, prompt, value string) {
	m.page = inputModal
	m.inputKind = kind
	m.input.SetValue(value)
	m.input.Prompt = prompt
	m.input.Focus()
}
func (m *Model) applyInput() tea.Cmd {
	value := strings.TrimSpace(m.input.Value())
	var err error
	switch m.inputKind {
	case "tokens":
		v, parseErr := strconv.ParseInt(value, 10, 32)
		if parseErr != nil {
			err = parseErr
		} else {
			next, e := m.working.SetTokens(int32(v))
			m.stage(next, fmt.Sprintf("token bank → %d", v), e)
		}
	case "wasps":
		v, parseErr := strconv.ParseInt(value, 10, 32)
		if parseErr != nil {
			err = parseErr
		} else {
			next, e := m.working.SetWasps(m.level, int32(v))
			m.stage(next, fmt.Sprintf("L%d wasps → %d", m.level, v), e)
		}
	case "import":
		return m.prepareImport(value)
	case "copy":
		return m.copyTo(value)
	}
	if err != nil {
		m.status = "Invalid value: " + err.Error()
		return nil
	}
	m.page = detail
	return nil
}
func (m *Model) prepareImport(path string) tea.Cmd {
	source, err := storage.Load(path)
	if err != nil {
		m.status = "Import failed: " + err.Error()
		m.page = dashboard
		return nil
	}
	if m.selected.Status != "empty" {
		m.importSource = source
		m.importPath = path
		m.page = importConfirmModal
		return nil
	}
	if err := storage.Export(source, m.selected.Path, false); err != nil {
		m.status = "Import failed: " + err.Error()
		m.page = dashboard
		return nil
	}
	return m.finishDashboard("Imported " + path + " into slot " + strconv.Itoa(m.selected.Number))
}

func (m *Model) importConfirmed() tea.Cmd {
	if m.importSource == nil {
		m.status = "Import failed: source was lost before confirmation"
		m.page = dashboard
		return nil
	}
	destination, err := storage.LoadRaw(m.selected.Path)
	if err == nil {
		_, err = storage.Commit(destination, m.importSource.Save, true)
	}
	if err != nil {
		m.status = "Import failed: " + err.Error()
		m.page = dashboard
		m.importSource = nil
		m.importPath = ""
		return nil
	}
	path := m.importPath
	m.importSource = nil
	m.importPath = ""
	return m.finishDashboard("Imported " + path + " into slot " + strconv.Itoa(m.selected.Number))
}
func (m *Model) copyTo(value string) tea.Cmd {
	slot, err := strconv.Atoi(value)
	if err != nil || slot < 1 || slot > native.NumSlots {
		m.status = "Choose a slot from 1 to 4"
		return nil
	}
	dest, _ := storage.SlotPath(m.dir, slot)
	source := m.selected.Snapshot
	if source == nil {
		m.status = "Choose a valid source slot"
		m.page = dashboard
		return nil
	}
	if _, err = os.Lstat(dest); err == nil {
		m.status = "Copy only targets an empty slot; use import to deliberately replace a slot"
		m.page = dashboard
		return nil
	} else if !os.IsNotExist(err) {
		m.status = "Copy failed: " + err.Error()
		m.page = dashboard
		return nil
	}
	if err := storage.Export(source, dest, false); err != nil {
		m.status = "Copy failed: " + err.Error()
		m.page = dashboard
		return nil
	}
	return m.finishDashboard(fmt.Sprintf("Copied slot %d to slot %d", m.selected.Number, slot))
}

func (m *Model) export() tea.Cmd {
	if m.selected.Snapshot == nil {
		return nil
	}
	name := fmt.Sprintf("%s.export-%s", m.selected.Path, time.Now().UTC().Format("20060102T150405Z"))
	if err := storage.Export(m.selected.Snapshot, name, false); err != nil {
		m.status = "Export failed: " + err.Error()
	} else {
		m.status = "Exported " + name
	}
	return nil
}
func (m *Model) finishDashboard(status string) tea.Cmd {
	m.page = dashboard
	m.refreshing = true
	m.status = status + " · refreshing slots…"
	m.dirty = nil
	m.original = nil
	m.working = nil
	return loadSlots(m.dir, status)
}
func (m *Model) commit() tea.Cmd {
	if m.original == nil || m.working == nil {
		return nil
	}
	result, err := storage.Commit(m.original, m.working, true)
	if err != nil {
		m.status = "Save failed: " + err.Error()
		m.page = detail
		return nil
	}
	status := "Saved staged edits"
	if result.Backup != "" {
		status += " · backup " + result.Backup
	}
	return m.finishDashboard(status)
}
func (m *Model) delete() tea.Cmd {
	if m.selected.Snapshot == nil {
		return nil
	}
	path, err := storage.Trash(m.selected.Snapshot)
	if err != nil {
		m.status = "Delete failed: " + err.Error()
		m.page = dashboard
		return nil
	}
	return m.finishDashboard("Moved to trash: " + path)
}

func (m Model) Update(msg tea.Msg) (tea.Model, tea.Cmd) {
	switch msg := msg.(type) {
	case slotsMsg:
		m.setSlots(msg.slots, msg.status)
		return m, nil
	case tea.WindowSizeMsg:
		m.width, m.height = msg.Width, msg.Height
		m.list.SetWidth(max(34, msg.Width/3))
		m.list.SetHeight(max(12, msg.Height-8))
		m.table.SetWidth(max(48, msg.Width-max(38, msg.Width/3)-6))
		m.table.SetHeight(max(8, msg.Height-10))
		return m, nil
	case tea.KeyPressMsg:
		key := msg.String()
		if key == "ctrl+c" || key == "q" && m.page == dashboard {
			return m, tea.Quit
		}
		switch m.page {
		case dashboard:
			if m.refreshing {
				return m, nil
			}
			switch key {
			case "enter":
				m.openSelected()
				return m, nil
			case "r":
				m.refreshing = true
				m.status = "Refreshing slots…"
				return m, loadSlots(m.dir, "")
			case "b":
				m.refreshSelection()
				return m, m.export()
			case "c":
				m.refreshSelection()
				if m.selected.Status == "valid" {
					m.openInput("copy", "Copy to empty slot (1–4): ", "")
					return m, nil
				}
			case "i":
				m.refreshSelection()
				m.openInput("import", "Import native save path: ", "")
				return m, nil
			case "x":
				m.refreshSelection()
				if m.selected.Status == "valid" {
					m.page = deleteModal
					return m, nil
				}
			}
		case detail:
			switch key {
			case "esc", "backspace":
				m.page = dashboard
				return m, nil
			case "1":
				m.tab = overviewTab
				m.rebuildTable()
				return m, nil
			case "2":
				m.tab = gagsTab
				m.rebuildTable()
				return m, nil
			case "3":
				m.tab = cardsTab
				m.rebuildTable()
				return m, nil
			case "4":
				m.tab = missionsTab
				m.rebuildTable()
				return m, nil
			case "[":
				if m.level > 1 {
					m.level--
					m.rebuildTable()
				}
				return m, nil
			case "]":
				if m.level < native.MaxLevels {
					m.level++
					m.rebuildTable()
				}
				return m, nil
			case " ":
				m.toggleRow()
				return m, nil
			case "t":
				m.openInput("tokens", "Token bank: ", fmt.Sprintf("%d", m.working.Tokens()))
				return m, nil
			case "w":
				current, _ := m.working.Wasps(m.level)
				m.openInput("wasps", fmt.Sprintf("Level %d wasps (0–20): ", m.level), fmt.Sprintf("%d", current))
				return m, nil
			case "f":
				current, _ := m.working.FMVUnlocked(m.level)
				next, err := m.working.SetFMV(m.level, !current)
				m.stage(next, fmt.Sprintf("L%d FMV → %s", m.level, onOff(!current)), err)
				return m, nil
			case "r":
				next, changed, err := m.working.RepairGagCounts()
				m.stage(next, fmt.Sprintf("repaired gag counts for levels %v", changed), err)
				return m, nil
			case "d":
				m.working = m.original.Save
				m.dirty = nil
				m.status = "Discarded staged edits"
				m.rebuildTable()
				return m, nil
			case "s":
				if len(m.dirty) > 0 {
					m.page = confirmModal
					return m, nil
				}
			}
		case inputModal:
			if key == "esc" {
				m.page = dashboard
				if m.inputKind == "tokens" || m.inputKind == "wasps" {
					m.page = detail
				}
				m.input.Blur()
				return m, nil
			}
			if key == "enter" {
				m.input.Blur()
				return m, m.applyInput()
			}
		case confirmModal:
			if key == "y" || key == "enter" {
				return m, m.commit()
			}
			if key == "n" || key == "esc" {
				m.page = detail
				return m, nil
			}
		case importConfirmModal:
			if key == "y" || key == "enter" {
				return m, m.importConfirmed()
			}
			if key == "n" || key == "esc" {
				m.importSource = nil
				m.importPath = ""
				m.page = dashboard
				return m, nil
			}
		case deleteModal:
			if key == "y" || key == "enter" {
				return m, m.delete()
			}
			if key == "n" || key == "esc" {
				m.page = dashboard
				return m, nil
			}
		}
	}
	var cmd tea.Cmd
	switch m.page {
	case dashboard:
		m.list, cmd = m.list.Update(msg)
		m.refreshSelection()
	case detail:
		m.table, cmd = m.table.Update(msg)
	case inputModal:
		m.input, cmd = m.input.Update(msg)
	}
	return m, cmd
}

func (m Model) View() tea.View {
	var body string
	switch m.page {
	case dashboard:
		body = m.dashboardView()
	case detail:
		body = m.detailView()
	case inputModal:
		body = m.modalView(m.input.Prompt+m.input.View(), "Enter to continue · esc to cancel")
	case confirmModal:
		body = m.modalView("Write staged changes to disk?\n\n"+strings.Join(prefix(m.dirty, "• "), "\n"), "y/enter to save (a .bak is created) · n/esc to cancel")
	case importConfirmModal:
		body = m.modalView(fmt.Sprintf("Replace Slot %d with this imported save?\n\n%s", m.selected.Number, m.importPath), "y/enter to replace (a .bak is created) · n/esc to cancel")
	case deleteModal:
		body = m.modalView(fmt.Sprintf("Move Slot %d to .savegame-trash?", m.selected.Number), "y/enter to move · n/esc to cancel")
	}
	view := tea.NewView(body)
	view.AltScreen = true
	return view
}
func (m Model) dashboardView() string {
	right := m.slotPreview()
	left := panelStyle.Width(max(32, m.width/3)).Render(m.list.View())
	rightPanel := panelStyle.Width(max(46, m.width-max(38, m.width/3)-5)).Render(right)
	help := "enter inspect · i import · c copy · b export · x trash · r reload · q quit"
	if m.refreshing {
		help = "Refreshing slots; actions are disabled · q quit"
	}
	status := footerStyle.Render(m.status + "\n" + help)
	return lipgloss.JoinHorizontal(lipgloss.Top, left, " ", rightPanel) + "\n" + status
}
func (m Model) slotPreview() string {
	m.refreshSelection()
	if m.selected.Status != "valid" {
		return titleStyle.Render("Save manager") + "\n\n" + mutedStyle.Render("Select a valid slot to inspect it.\n\nEmpty slots accept imports and copies.\nInvalid slots can be recovered by importing a known-good export.")
	}
	s, _ := m.selected.Snapshot.Save.Summary()
	h := m.selected.Snapshot.Save.Header()
	lines := []string{titleStyle.Render(fmt.Sprintf("Slot %d · %s", m.selected.Number, h.Timestamp())), fmt.Sprintf("Player: %s", fallback(s.Player, "(unnamed)")), fmt.Sprintf("Progress: L%d M%d · highest L%d M%d", s.CurrentLevel, s.CurrentMission, s.HighestLevel, s.HighestMission), fmt.Sprintf("Tokens: %d · Cars: %d", s.Tokens, s.Cars), "", titleStyle.Render("Level completion")}
	for _, l := range s.LevelStats {
		lines = append(lines, fmt.Sprintf("L%d   cards %d/7   gags %d/%d   wasps %d/20", l.Level, l.Cards, l.Gags, l.GagTotal, l.Wasps))
	}
	lines = append(lines, "", mutedStyle.Render("SHA-256  "+m.selected.Snapshot.Save.SHA256()[:16]+"…"))
	return strings.Join(lines, "\n")
}
func (m Model) detailView() string {
	tabNames := []string{"1 Overview", "2 Gags", "3 Cards", "4 Missions"}
	for i := range tabNames {
		if tab(i) == m.tab {
			tabNames[i] = titleStyle.Render(tabNames[i])
		} else {
			tabNames[i] = mutedStyle.Render(tabNames[i])
		}
	}
	head := fmt.Sprintf("Slot %d  ·  Level %d  ·  %s", m.selected.Number, m.level, strings.Join(tabNames, "  "))
	dirty := "No staged changes"
	if len(m.dirty) > 0 {
		dirty = okStyle.Render(fmt.Sprintf("%d staged change(s) — press s to review/save", len(m.dirty)))
	}
	footer := footerStyle.Render("space toggle selected · [ ] level · t tokens · w wasps · f FMV · r repair gags · d discard · esc back\n" + dirty + "\n" + m.status)
	return panelStyle.Width(max(64, m.width-4)).Render(titleStyle.Render(head)+"\n\n"+m.table.View()+"\n"+m.table.HelpView()) + "\n" + footer
}
func (m Model) modalView(content, hint string) string {
	box := panelStyle.BorderForeground(cyan).Padding(2, 3).Render(titleStyle.Render("SRR2 Save Manager") + "\n\n" + content + "\n\n" + mutedStyle.Render(hint))
	return lipgloss.Place(max(1, m.width), max(1, m.height), lipgloss.Center, lipgloss.Center, box)
}
func prefix(values []string, p string) []string {
	out := make([]string, len(values))
	for i, v := range values {
		out[i] = p + " " + v
	}
	return out
}
func max(a, b int) int {
	if a > b {
		return a
	}
	return b
}

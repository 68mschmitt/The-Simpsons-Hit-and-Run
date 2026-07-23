// srr2-save is the interactive native Linux save manager for SRR2.
package main

import (
	"flag"
	"fmt"
	"os"

	tea "charm.land/bubbletea/v2"
	"github.com/mschmitt/the-simpsons-hit-and-run/tools/savegame/internal/tui"
	"github.com/mschmitt/the-simpsons-hit-and-run/tools/savegame/storage"
)

func main() {
	saveDir := flag.String("save-dir", storage.DefaultDir(), "directory containing Save1 through Save4")
	flag.Parse()
	program := tea.NewProgram(tui.New(*saveDir))
	if _, err := program.Run(); err != nil {
		fmt.Fprintln(os.Stderr, "srr2-save:", err)
		os.Exit(1)
	}
}

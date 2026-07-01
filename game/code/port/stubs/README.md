# Linux PoC middleware stubs

This directory contains deliberately small compatibility headers used only by the
`LINUX_POC` CMake target. They stand in for missing RAD/Pure3D/PDDI interfaces so
that the first Linux-native executable can be built without proprietary SDKs.

Keep these headers narrow: add only the API surface required by the current PoC
milestone, then replace no-op behavior with real Linux backends incrementally.

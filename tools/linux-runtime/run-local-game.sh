#!/bin/bash
# Local Linux/Xvfb runtime harness for SRR2. This is the non-Docker fallback for
# machines where the Docker daemon is unavailable.
#
# Required:
#   ASSET_ROOT=/path/to/extracted/retail/assets
# Optional:
#   OUT_DIR       directory for logs/screenshots (default /tmp/srr2-linux-runtime-*)
#   BUILD_DIR     CMake build directory (default build/native)
#   RUN_SECONDS   timeout in seconds (default 60)
#   GAME_ARGS     command-line arguments for the game (e.g. "skipfe")
#   SKIP_BUILD=1  reuse an existing BUILD_DIR/code/SRR2
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
ASSET_ROOT="${ASSET_ROOT:-${1:-}}"
if [ -z "$ASSET_ROOT" ]; then
    echo "usage: ASSET_ROOT=/path/to/assets $0" >&2
    exit 2
fi
if [ ! -d "$ASSET_ROOT" ]; then
    echo "asset root does not exist: $ASSET_ROOT" >&2
    exit 2
fi
"$REPO_ROOT/tools/linux-runtime/validate-assets.sh" -- "$ASSET_ROOT"

OUT_DIR="${OUT_DIR:-/tmp/srr2-linux-runtime-$(date +%Y%m%d-%H%M%S)}"
BUILD_DIR="${BUILD_DIR:-$REPO_ROOT/build/native}"
RUN_SECONDS="${RUN_SECONDS:-60}"
GAME_ARGS="${GAME_ARGS:-}"
SKIP_BUILD="${SKIP_BUILD:-0}"
mkdir -p "$OUT_DIR"

if ! command -v Xvfb >/dev/null 2>&1; then
    echo "Xvfb is required. Install xorg-server-xvfb (Arch) or xvfb (Debian/Ubuntu), or start Docker and use tools/linux-runtime/." >&2
    exit 2
fi
if ! command -v xwd >/dev/null 2>&1; then
    echo "xwd is required for screenshots. Install xorg-xwd (Arch) or x11-apps (Debian/Ubuntu)." >&2
    exit 2
fi

if [ "$SKIP_BUILD" != "1" ]; then
    cmake -S "$REPO_ROOT" -B "$BUILD_DIR" \
        -DCMAKE_BUILD_TYPE=Release \
        -DSRR2_BUILD_TESTS=OFF \
        -DSRR2_USE_PCH=OFF > "$OUT_DIR/cfg.log" 2>&1
    cmake --build "$BUILD_DIR" -j"$(nproc)" > "$OUT_DIR/build.log" 2>&1
fi

if [ ! -x "$BUILD_DIR/code/SRR2" ]; then
    echo "SRR2 binary not found: $BUILD_DIR/code/SRR2" >&2
    exit 2
fi

DISPLAY_NUM="${DISPLAY_NUM:-99}"
while [ -e "/tmp/.X${DISPLAY_NUM}-lock" ]; do
    DISPLAY_NUM=$((DISPLAY_NUM + 1))
done

Xvfb ":$DISPLAY_NUM" -screen 0 1024x768x24 > "$OUT_DIR/xvfb.log" 2>&1 &
XVFB_PID=$!
cleanup() {
    kill "$XVFB_PID" 2>/dev/null || true
    wait "$XVFB_PID" 2>/dev/null || true
}
trap cleanup EXIT
sleep 1

export DISPLAY=":$DISPLAY_NUM"
export LIBGL_ALWAYS_SOFTWARE="${LIBGL_ALWAYS_SOFTWARE:-1}"
export ALSOFT_DRIVERS="${ALSOFT_DRIVERS:-null}"
export SDL_AUDIODRIVER="${SDL_AUDIODRIVER:-dummy}"
# On Wayland desktops SDL can otherwise ignore DISPLAY and create the game
# window on the user's compositor instead of the Xvfb server, producing black
# root-window screenshots. Force the X11 backend for this harness.
export SDL_VIDEODRIVER=x11
unset WAYLAND_DISPLAY

(
    sleep $(( RUN_SECONDS * 3 / 4 ))
    xwd -root -display "$DISPLAY" -silent > "$OUT_DIR/screenshot.xwd" 2> "$OUT_DIR/xwd.err" || true
    if command -v magick >/dev/null 2>&1; then
        magick "$OUT_DIR/screenshot.xwd" "$OUT_DIR/screenshot.png" >> "$OUT_DIR/convert.log" 2>&1 || true
    elif command -v convert >/dev/null 2>&1; then
        convert "$OUT_DIR/screenshot.xwd" "$OUT_DIR/screenshot.png" >> "$OUT_DIR/convert.log" 2>&1 || true
    fi
) &
SHOT_PID=$!

cd "$ASSET_ROOT"
set +e
# shellcheck disable=SC2086
timeout -k 5s "$RUN_SECONDS" "$BUILD_DIR/code/SRR2" ${GAME_ARGS} 2>&1 | tee "$OUT_DIR/game.log"
STATUS=${PIPESTATUS[0]}
set -e
wait "$SHOT_PID" 2>/dev/null || true

echo "$STATUS" > "$OUT_DIR/status.txt"
echo "game exit status: $STATUS (124 = ran until timeout)"
echo "output: $OUT_DIR"

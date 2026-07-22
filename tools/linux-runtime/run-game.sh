#!/bin/bash
# Build SRR2 from /src, then run it under Xvfb with Mesa software GL from the
# asset tree at /assets. Environment overrides:
#   RUN_SECONDS   how long to let the game run before stopping (default 30)
#   GAME_ARGS     command-line arguments for the game (e.g. "skipfe")
#   SKIP_BUILD=1  reuse /out/build/code/SRR2 instead of rebuilding
#
# A screenshot of the virtual display is written to /out/screenshot.xwd
# shortly before the timeout (view with xwud, GIMP, or `magick x:...`).
set -e

/usr/local/lib/srr2/validate-assets.sh -- /assets

if [ "${SKIP_BUILD}" != "1" ]; then
    cmake -S /src -B /out/build \
        -DCMAKE_BUILD_TYPE=Release \
        -DSRR2_BUILD_TESTS=OFF \
        -DSRR2_USE_PCH=OFF > /out/cfg.log 2>&1
    cmake --build /out/build -j"$(nproc)" > /out/build.log 2>&1
fi

export LIBGL_ALWAYS_SOFTWARE=1
export ALSOFT_DRIVERS=null
export SDL_VIDEODRIVER=x11
unset WAYLAND_DISPLAY

RUN_SECONDS="${RUN_SECONDS:-30}"

cd /assets
Xvfb :99 -screen 0 1024x768x24 &
XVFB_PID=$!
export DISPLAY=:99
sleep 1

(
    sleep $(( RUN_SECONDS * 3 / 4 ))
    xwd -root -display :99 -silent > /out/screenshot.xwd 2>/dev/null || true
) &

# shellcheck disable=SC2086
timeout "$RUN_SECONDS" /out/build/code/SRR2 ${GAME_ARGS} 2>&1 | tee /out/game.log
STATUS=${PIPESTATUS[0]}

kill $XVFB_PID 2>/dev/null || true
echo "game exit status: $STATUS (124 = ran until timeout)"

#!/bin/bash
# Build SRR2 from /src, then run it under Xvfb with Mesa software GL from the
# asset tree at /assets. Environment overrides:
#   RUN_SECONDS   how long to let the game run before stopping (default 30)
#   SKIP_BUILD=1  reuse /out/build/code/SRR2 instead of rebuilding
set -e

if [ "${SKIP_BUILD}" != "1" ]; then
    cmake -S /src -B /out/build -DSRR2_BUILD_TESTS=OFF > /out/cfg.log 2>&1
    cmake --build /out/build -j"$(nproc)" > /out/build.log 2>&1
fi

export LIBGL_ALWAYS_SOFTWARE=1
export SDL_AUDIODRIVER=dummy

cd /assets
Xvfb :99 -screen 0 1024x768x24 &
XVFB_PID=$!
export DISPLAY=:99
sleep 1

timeout "${RUN_SECONDS:-30}" /out/build/code/SRR2 2>&1 | tee /out/game.log
STATUS=${PIPESTATUS[0]}

kill $XVFB_PID 2>/dev/null || true
echo "game exit status: $STATUS (124 = ran until timeout)"

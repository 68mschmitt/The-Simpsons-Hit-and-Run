# Headless Linux runtime environment for SRR2: Xvfb + Mesa llvmpipe gives the
# game a software OpenGL 2.1 compatibility context with no GPU or display.
#
# Build (from repo root):
#   docker build -t srr2-linux-runtime tools/linux-runtime
# Run the game against an asset tree mounted at /assets (build output and
# logs land in the /out mount):
#   docker run --rm -v "$PWD:/src:ro" -v /path/to/assets:/assets \
#       -v /path/for/output:/out srr2-linux-runtime
FROM ubuntu:24.04

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
    cmake g++ make pkg-config \
    libsdl2-dev libpng-dev libopenal-dev \
    libavformat-dev libavcodec-dev libavutil-dev libswresample-dev libswscale-dev \
    xvfb mesa-utils libgl1-mesa-dri x11-apps \
    && rm -rf /var/lib/apt/lists/*

COPY run-game.sh /usr/local/bin/run-game.sh
RUN chmod +x /usr/local/bin/run-game.sh

CMD ["/usr/local/bin/run-game.sh"]

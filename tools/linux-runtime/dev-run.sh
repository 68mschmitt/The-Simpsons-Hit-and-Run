#!/usr/bin/env bash
# Configure, incrementally build, and run the native Linux target through the
# same launcher and asset validation path used by an installed copy.
set -euo pipefail

usage() {
    cat <<'EOF'
Usage: tools/linux-runtime/dev-run.sh --data-dir PATH [--install-deps] [--] [game arguments]

Builds build/native with the validated SDL2/OpenGL defaults, then launches it
from PATH. Pass `skipfe` after -- (or directly) to enter gameplay immediately.

Options:
  --data-dir PATH  Retail PC game-data root for this run (required).
  --install-deps   Install known distro prerequisites with the detected package
                   manager. Without it, missing prerequisites print the exact
                   package command instead of invoking sudo.
  -h, --help       Show this help text.
EOF
}

die() {
    local status=$1
    shift
    printf 'dev-run: %s\n' "$*" >&2
    exit "$status"
}

REPO_ROOT="$(cd -P -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd)"
RUNTIME_DIR="$REPO_ROOT/tools/linux-runtime"
BUILD_DIR="${SRR2_BUILD_DIR:-$REPO_ROOT/build/native}"
if [[ $BUILD_DIR != /* ]]; then
    BUILD_DIR="$REPO_ROOT/$BUILD_DIR"
fi

PACKAGE_MANAGER=""
if command -v apt-get >/dev/null 2>&1; then
    PACKAGE_MANAGER=apt
elif command -v pacman >/dev/null 2>&1; then
    PACKAGE_MANAGER=pacman
elif command -v dnf >/dev/null 2>&1; then
    PACKAGE_MANAGER=dnf
fi

print_dependency_command() {
    case $PACKAGE_MANAGER in
        apt)
            cat <<'EOF' >&2
Debian/Ubuntu prerequisites:
  sudo apt update
  sudo apt install build-essential cmake pkg-config libsdl2-dev libpng-dev libopenal-dev libavformat-dev libavcodec-dev libavutil-dev libswresample-dev libswscale-dev zlib1g-dev p7zip-full
EOF
            ;;
        pacman)
            cat <<'EOF' >&2
Arch prerequisites:
  sudo pacman -S --needed cmake gcc pkgconf sdl2-compat libpng openal ffmpeg zlib p7zip
EOF
            ;;
        dnf)
            cat <<'EOF' >&2
Fedora prerequisites:
  sudo dnf install gcc-c++ cmake pkgconf-pkg-config SDL2-devel libpng-devel openal-soft-devel ffmpeg-devel zlib-devel p7zip
EOF
            ;;
        *)
            cat <<'EOF' >&2
Install these prerequisites for your distribution:
  C++ compiler, cmake, pkg-config, SDL2 development files, libpng development
  files, OpenAL development files, FFmpeg development files (avformat,
  avcodec, avutil, swresample, swscale), zlib development files, and p7zip.
EOF
            ;;
    esac
}

install_dependencies() {
    command -v sudo >/dev/null 2>&1 || die 2 "--install-deps requires sudo; run the command below manually"
    case $PACKAGE_MANAGER in
        apt)
            sudo apt-get update
            sudo apt-get install -y build-essential cmake pkg-config libsdl2-dev libpng-dev libopenal-dev \
                libavformat-dev libavcodec-dev libavutil-dev libswresample-dev libswscale-dev zlib1g-dev p7zip-full
            ;;
        pacman)
            sudo pacman -S --needed cmake gcc pkgconf sdl2-compat libpng openal ffmpeg zlib p7zip
            ;;
        dnf)
            sudo dnf install -y gcc-c++ cmake pkgconf-pkg-config SDL2-devel libpng-devel openal-soft-devel ffmpeg-devel zlib-devel p7zip
            ;;
        *)
            print_dependency_command
            die 2 "could not detect apt, pacman, or dnf"
            ;;
    esac
}

check_prerequisites() {
    local missing=0 module
    for command in cmake c++ pkg-config; do
        if ! command -v "$command" >/dev/null 2>&1; then
            printf 'dev-run: missing command: %s\n' "$command" >&2
            missing=1
        fi
    done

    if command -v pkg-config >/dev/null 2>&1; then
        for module in sdl2 libpng openal libavformat libavcodec libavutil libswresample libswscale zlib; do
            if ! pkg-config --exists "$module"; then
                printf 'dev-run: pkg-config module not found: %s\n' "$module" >&2
                missing=1
            fi
        done
    fi

    return "$missing"
}

DATA_DIR=""
INSTALL_DEPS=0
declare -a GAME_ARGS=()
while [[ $# -gt 0 ]]; do
    case $1 in
        --data-dir)
            [[ $# -ge 2 && -n ${2:-} ]] || die 2 "--data-dir requires a path"
            DATA_DIR=$2
            shift 2
            ;;
        --data-dir=*)
            DATA_DIR=${1#--data-dir=}
            [[ -n $DATA_DIR ]] || die 2 "--data-dir requires a path"
            shift
            ;;
        --install-deps)
            INSTALL_DEPS=1
            shift
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        --)
            shift
            GAME_ARGS+=("$@")
            break
            ;;
        *)
            GAME_ARGS+=("$1")
            shift
            ;;
    esac
done

[[ -n $DATA_DIR ]] || die 2 "--data-dir is required"

if ! check_prerequisites; then
    if [[ $INSTALL_DEPS -eq 1 ]]; then
        install_dependencies
        check_prerequisites || die 2 "prerequisites are still missing after installation"
    else
        print_dependency_command
        die 2 "install the prerequisites above, or rerun with --install-deps"
    fi
fi

JOBS="${SRR2_JOBS:-}"
if [[ -z $JOBS ]]; then
    if command -v nproc >/dev/null 2>&1; then
        JOBS=$(nproc)
    else
        JOBS=$(getconf _NPROCESSORS_ONLN 2>/dev/null || printf '1')
    fi
fi

cmake -S "$REPO_ROOT" -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE=Release \
    -DSRR2_BUILD_TESTS=OFF \
    -DSRR2_USE_PCH=OFF
cmake --build "$BUILD_DIR" -j"$JOBS"

GAME_EXECUTABLE="$BUILD_DIR/code/SRR2"
[[ -x $GAME_EXECUTABLE ]] || die 2 "build completed without the expected executable: $GAME_EXECUTABLE"

# Override any inherited SRR2_EXECUTABLE so developer runs always exercise the
# binary just built above. The launcher still performs all normal data checks.
SRR2_EXECUTABLE="$GAME_EXECUTABLE" \
    "$RUNTIME_DIR/srr2-launch" --data-dir "$DATA_DIR" -- "${GAME_ARGS[@]}"

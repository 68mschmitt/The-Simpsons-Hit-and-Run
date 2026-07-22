#!/usr/bin/env bash
# Validate the minimum retail PC asset tree required by the Linux runtime.
# This is deliberately read-only so it is safe for read-only Docker mounts.
set -u

usage() {
    cat <<'EOF'
Usage: validate-assets.sh <retail-data-directory>

Checks the required PC data directories and frontend files without modifying
the data directory. Exit status is 0 when all required entries are present.
EOF
}

if [[ ${1:-} == "--help" || ${1:-} == "-h" ]]; then
    usage
    exit 0
fi

if [[ ${1:-} == "--" ]]; then
    shift
fi

if [[ $# -ne 1 || -z ${1:-} ]]; then
    usage >&2
    exit 2
fi

asset_root=$1
if [[ ! -d "$asset_root" ]]; then
    printf 'SRR2 assets: data directory does not exist or is not a directory: %s\n' "$asset_root" >&2
    exit 1
fi

missing=0
check_directory() {
    local relative_path=$1
    local path="$asset_root/$relative_path"

    if [[ ! -d "$path" ]]; then
        printf 'SRR2 assets: missing required directory: %s\n' "$path" >&2
        missing=1
    elif [[ ! -r "$path" || ! -x "$path" ]]; then
        printf 'SRR2 assets: required directory is not readable: %s\n' "$path" >&2
        missing=1
    fi
}

check_file() {
    local relative_path=$1
    local path="$asset_root/$relative_path"

    if [[ ! -f "$path" ]]; then
        printf 'SRR2 assets: missing required file: %s\n' "$path" >&2
        missing=1
    elif [[ ! -r "$path" ]]; then
        printf 'SRR2 assets: required file is not readable: %s\n' "$path" >&2
        missing=1
    elif [[ ! -s "$path" ]]; then
        printf 'SRR2 assets: required file is empty: %s\n' "$path" >&2
        missing=1
    fi
}

check_directory art
check_directory movies
check_directory scripts
check_directory sound
check_file simpsons.ini
# These case-sensitive files are loaded during boot/frontend startup and catch
# incomplete or incorrectly extracted PC installations early.
check_file art/frontend/dynaload/images/license/licensePC.p3d
check_file art/frontend/scrooby/frontend.p3d

if [[ $missing -ne 0 ]]; then
    printf 'SRR2 assets: validation failed for: %s\n' "$asset_root" >&2
    exit 1
fi

printf 'SRR2 assets: validation passed: %s\n' "$asset_root"

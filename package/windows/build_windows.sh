#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
SDL_VERSION="2.32.10"
MIXER_VERSION="2.8.2"
SDL_SHA256="83a5d74012311edc3c0d40ea6faecbe57ad692aa033fa5dc273cc937e3938ff2"
MIXER_SHA256="6872029bcca09b12985750d38de8865c66fbef5567715f0e4d89533ccaf3a0f2"
SDL_URL="https://github.com/libsdl-org/SDL/releases/download/release-$SDL_VERSION/SDL2-devel-$SDL_VERSION-mingw.tar.gz"
MIXER_URL="https://github.com/libsdl-org/SDL_mixer/releases/download/release-$MIXER_VERSION/SDL2_mixer-devel-$MIXER_VERSION-mingw.tar.gz"
MINGW_PREFIX="${MINGW_PREFIX:-x86_64-w64-mingw32}"
CC="${CC_WINDOWS:-$MINGW_PREFIX-gcc}"
WINDRES="${WINDRES_WINDOWS:-$MINGW_PREFIX-windres}"
STRIP="${STRIP_WINDOWS:-$MINGW_PREFIX-strip}"
DIST_DIR="$REPO_ROOT/dist"
OUTPUT_PATH="$DIST_DIR/Wolfenstein2026.exe"
STAGING_ROOT="$(mktemp -d)"
PACKAGE_DIR="$STAGING_ROOT/Wolfenstein2026"

cleanup() {
    rm -rf "$STAGING_ROOT"
}
trap cleanup EXIT

for tool in "$CC" "$WINDRES" "$STRIP" makensis shasum tar curl; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        echo "Required Windows build tool not found: $tool" >&2
        exit 1
    fi
done

download_dependency() {
    local url="$1"
    local output="$2"
    local checksum="$3"

    if [[ ! -f "$output" ]]; then
        mkdir -p "$(dirname "$output")"
        curl --fail --location --output "$output.download" "$url"
        mv "$output.download" "$output"
    fi

    if ! printf '%s  %s\n' "$checksum" "$output" | shasum -a 256 -c -; then
        echo "Dependency checksum failed: $output" >&2
        exit 1
    fi
}

CACHE_HOME="${XDG_CACHE_HOME:-$HOME/.cache}"
DEPENDENCY_CACHE="$CACHE_HOME/wolfenstein2026/windows"
SDL_ARCHIVE="${SDL2_WINDOWS_ARCHIVE:-$DEPENDENCY_CACHE/SDL2-devel-$SDL_VERSION-mingw.tar.gz}"
MIXER_ARCHIVE="${SDL2_MIXER_WINDOWS_ARCHIVE:-$DEPENDENCY_CACHE/SDL2_mixer-devel-$MIXER_VERSION-mingw.tar.gz}"

download_dependency "$SDL_URL" "$SDL_ARCHIVE" "$SDL_SHA256"
download_dependency "$MIXER_URL" "$MIXER_ARCHIVE" "$MIXER_SHA256"

tar -xzf "$SDL_ARCHIVE" -C "$STAGING_ROOT"
tar -xzf "$MIXER_ARCHIVE" -C "$STAGING_ROOT"

SDL_ROOT="$STAGING_ROOT/SDL2-$SDL_VERSION/x86_64-w64-mingw32"
MIXER_ROOT="$STAGING_ROOT/SDL2_mixer-$MIXER_VERSION/x86_64-w64-mingw32"
RESOURCE_OBJECT="$STAGING_ROOT/wolfenstein2026-resource.o"

if [[ ! -d "$SDL_ROOT" || ! -d "$MIXER_ROOT" ]]; then
    echo "The SDL dependency archives do not contain 64-bit MinGW libraries." >&2
    exit 1
fi

source_files=()
while IFS= read -r -d '' source_file; do
    source_files+=("$source_file")
done < <(find "$REPO_ROOT/src" -name '*.c' -print0)

mkdir -p "$PACKAGE_DIR" "$DIST_DIR"

"$WINDRES" --include-dir "$SCRIPT_DIR" "$SCRIPT_DIR/wolfenstein2026.rc" "$RESOURCE_OBJECT"
"$CC" -std=gnu11 -O2 \
    -I"$REPO_ROOT/src" \
    -I"$SDL_ROOT/include" \
    -I"$SDL_ROOT/include/SDL2" \
    -I"$MIXER_ROOT/include" \
    -I"$MIXER_ROOT/include/SDL2" \
    "${source_files[@]}" \
    "$RESOURCE_OBJECT" \
    -L"$SDL_ROOT/lib" \
    -L"$MIXER_ROOT/lib" \
    -lmingw32 \
    -lSDL2main \
    -lSDL2 \
    -lSDL2_mixer \
    -lm \
    -mwindows \
    -o "$PACKAGE_DIR/wolfenstein2026.exe"

"$STRIP" "$PACKAGE_DIR/wolfenstein2026.exe"
cp "$SDL_ROOT/bin/SDL2.dll" "$PACKAGE_DIR/"
cp "$MIXER_ROOT/bin/SDL2_mixer.dll" "$PACKAGE_DIR/"
cp -R "$REPO_ROOT/assets" "$PACKAGE_DIR/assets"

makensis \
    -DSOURCE_DIR="$PACKAGE_DIR" \
    -DOUTPUT_FILE="$OUTPUT_PATH" \
    -DICON_FILE="$SCRIPT_DIR/wolfenstein2026.ico" \
    "$SCRIPT_DIR/launcher.nsi"

echo "Created $OUTPUT_PATH"

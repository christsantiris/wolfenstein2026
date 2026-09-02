#!/bin/bash
set -e

cd "$(dirname "$0")/../.."

SDL_VERSION="2.32.10"
SDL_MIXER_VERSION="2.8.2"
SDL_DMG="build/macos-deps/SDL2-${SDL_VERSION}.dmg"
SDL_MIXER_DMG="build/macos-deps/SDL2_mixer-${SDL_MIXER_VERSION}.dmg"
FRAMEWORK_DIR="$(pwd)/build/macos-frameworks"
ARM_BUILD="build/macos-arm64"
INTEL_BUILD="build/macos-x86_64"
UNIVERSAL_DIR="build/macos-universal"

download_file() {
    local url="$1"
    local output="$2"
    if [ ! -f "$output" ]; then
        curl -fL "$url" -o "$output"
    fi
}

verify_sha256() {
    local expected="$1"
    local file="$2"
    local actual
    actual=$(shasum -a 256 "$file" | awk '{print $1}')
    if [ "$actual" != "$expected" ]; then
        echo "Checksum mismatch: $file"
        exit 1
    fi
}

extract_framework() {
    local dmg="$1"
    local framework="$2"
    local mount_dir
    mount_dir=$(mktemp -d /tmp/wolf_framework_mount.XXXXXX)
    hdiutil attach "$dmg" -readonly -nobrowse -mountpoint "$mount_dir" > /dev/null
    ditto "$mount_dir/$framework" "$FRAMEWORK_DIR/$framework"
    hdiutil detach "$mount_dir" > /dev/null
    rmdir "$mount_dir"
}

mkdir -p build/macos-deps "$FRAMEWORK_DIR" "$UNIVERSAL_DIR"

download_file "https://github.com/libsdl-org/SDL/releases/download/release-${SDL_VERSION}/SDL2-${SDL_VERSION}.dmg" "$SDL_DMG"
download_file "https://github.com/libsdl-org/SDL_mixer/releases/download/release-${SDL_MIXER_VERSION}/SDL2_mixer-${SDL_MIXER_VERSION}.dmg" "$SDL_MIXER_DMG"
verify_sha256 "4a7ac31640d70214e848f994be8a12849c0f97918a7e6c2e27a40036166d1a7f" "$SDL_DMG"
verify_sha256 "3ad8af90531596403d5a914aae4bcc3feff209cfa4ea493211ea21a10e0b9adf" "$SDL_MIXER_DMG"

if [ ! -d "$FRAMEWORK_DIR/SDL2.framework" ]; then
    extract_framework "$SDL_DMG" "SDL2.framework"
fi
if [ ! -d "$FRAMEWORK_DIR/SDL2_mixer.framework" ]; then
    extract_framework "$SDL_MIXER_DMG" "SDL2_mixer.framework"
fi

rm -rf "$FRAMEWORK_DIR/include"
mkdir -p "$FRAMEWORK_DIR/include/SDL2"
ditto "$FRAMEWORK_DIR/SDL2.framework/Headers" "$FRAMEWORK_DIR/include/SDL2"
ditto "$FRAMEWORK_DIR/SDL2_mixer.framework/Headers" "$FRAMEWORK_DIR/include/SDL2"

cmake -S . -B "$ARM_BUILD" -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES=arm64 -DCMAKE_OSX_DEPLOYMENT_TARGET=12.0 -DCMAKE_SKIP_BUILD_RPATH=TRUE -DMACOS_FRAMEWORK_DIR="$FRAMEWORK_DIR"
cmake --build "$ARM_BUILD"
cmake -S . -B "$INTEL_BUILD" -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES=x86_64 -DCMAKE_OSX_DEPLOYMENT_TARGET=12.0 -DCMAKE_SKIP_BUILD_RPATH=TRUE -DMACOS_FRAMEWORK_DIR="$FRAMEWORK_DIR"
cmake --build "$INTEL_BUILD"

lipo -create "$ARM_BUILD/wolf" "$INTEL_BUILD/wolf" -output "$UNIVERSAL_DIR/wolf"
lipo "$UNIVERSAL_DIR/wolf" -verify_arch arm64 x86_64

WOLF_BINARY="$UNIVERSAL_DIR/wolf" WOLF_FRAMEWORK_DIR="$FRAMEWORK_DIR" bash package/macos/build_dmg.sh

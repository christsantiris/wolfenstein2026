#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

if [[ "$(uname -s)" != "Linux" ]]; then
    echo "AppImages must be built on a Linux host." >&2
    exit 1
fi

ARCH="$(uname -m)"
case "$ARCH" in
    x86_64)
        LINUXDEPLOY_ARCH="x86_64"
        ;;
    aarch64|arm64)
        LINUXDEPLOY_ARCH="aarch64"
        ;;
    *)
        echo "Unsupported AppImage architecture: $ARCH" >&2
        exit 1
        ;;
esac

STAGING_ROOT="$(mktemp -d)"
BUILD_DIR="$STAGING_ROOT/build"
APP_DIR="$STAGING_ROOT/Wolfenstein2026.AppDir"
DIST_DIR="$REPO_ROOT/dist"

cleanup() {
    rm -rf "$STAGING_ROOT"
}
trap cleanup EXIT

if [[ -n "${LINUXDEPLOY:-}" ]]; then
    LINUXDEPLOY_BIN="$LINUXDEPLOY"
else
    CACHE_HOME="${XDG_CACHE_HOME:-$HOME/.cache}"
    TOOL_CACHE="$CACHE_HOME/wolfenstein2026"
    LINUXDEPLOY_BIN="$TOOL_CACHE/linuxdeploy-$LINUXDEPLOY_ARCH.AppImage"
    if [[ ! -x "$LINUXDEPLOY_BIN" ]]; then
        if ! command -v curl >/dev/null 2>&1; then
            echo "curl is required to download linuxdeploy." >&2
            exit 1
        fi
        mkdir -p "$TOOL_CACHE"
        curl --fail --location --output "$LINUXDEPLOY_BIN.download" "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-$LINUXDEPLOY_ARCH.AppImage"
        chmod 755 "$LINUXDEPLOY_BIN.download"
        mv "$LINUXDEPLOY_BIN.download" "$LINUXDEPLOY_BIN"
    fi
fi

if [[ ! -x "$LINUXDEPLOY_BIN" ]]; then
    echo "linuxdeploy is not executable: $LINUXDEPLOY_BIN" >&2
    exit 1
fi

cmake -S "$REPO_ROOT" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_DIR"

mkdir -p "$APP_DIR/usr/bin" "$APP_DIR/usr/share/metainfo" "$APP_DIR/usr/share/wolfenstein2026" "$DIST_DIR"
install -m 755 "$BUILD_DIR/wolf" "$APP_DIR/usr/bin/wolf"
install -m 755 "$SCRIPT_DIR/AppRun" "$APP_DIR/AppRun"
install -m 644 "$SCRIPT_DIR/com.wolfenstein2026.game.metainfo.xml" "$APP_DIR/usr/share/metainfo/wolfenstein2026-appimage.appdata.xml"
cp -R "$REPO_ROOT/assets" "$APP_DIR/usr/share/wolfenstein2026/assets"

if [[ "$LINUXDEPLOY_BIN" == *.AppImage ]]; then
    set -- --appimage-extract-and-run
else
    set --
fi

export LDAI_OUTPUT="${LDAI_OUTPUT:-$DIST_DIR/Wolfenstein2026-1.0.0-$LINUXDEPLOY_ARCH.AppImage}"
export LINUXDEPLOY_OUTPUT_VERSION="1.0.0"
"$LINUXDEPLOY_BIN" "$@" \
    --appdir "$APP_DIR" \
    --executable "$APP_DIR/usr/bin/wolf" \
    --desktop-file "$SCRIPT_DIR/wolfenstein2026-appimage.desktop" \
    --icon-file "$SCRIPT_DIR/wolfenstein2026.png" \
    --output appimage

chmod 755 "$LDAI_OUTPUT"
echo "Created $LDAI_OUTPUT"

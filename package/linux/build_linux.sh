#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

if [[ "$(uname -s)" != "Linux" ]]; then
    echo "Linux packages must be built on a Linux host." >&2
    exit 1
fi

ARCH="$(uname -m)"
case "$ARCH" in
    x86_64)
        DEB_ARCH="amd64"
        ;;
    aarch64|arm64)
        DEB_ARCH="arm64"
        ;;
    *)
        DEB_ARCH="$ARCH"
        ;;
esac

if ! command -v dpkg-deb >/dev/null 2>&1; then
    echo "dpkg-deb is required to build the Linux installer." >&2
    exit 1
fi

PACKAGE_NAME="wolfenstein2026_1.0.0_$DEB_ARCH.deb"
BUILD_DIR="$REPO_ROOT/build-linux"
DIST_DIR="$REPO_ROOT/dist"
STAGING_ROOT="$(mktemp -d)"
PACKAGE_ROOT="$STAGING_ROOT/package"

cleanup() {
    rm -rf "$STAGING_ROOT"
}
trap cleanup EXIT

cmake -S "$REPO_ROOT" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_DIR"

mkdir -p \
    "$PACKAGE_ROOT/DEBIAN" \
    "$PACKAGE_ROOT/usr/bin" \
    "$PACKAGE_ROOT/usr/lib/wolfenstein2026" \
    "$PACKAGE_ROOT/usr/share/applications" \
    "$PACKAGE_ROOT/usr/share/icons/hicolor/512x512/apps" \
    "$PACKAGE_ROOT/usr/share/wolfenstein2026" \
    "$DIST_DIR"

install -m 755 "$BUILD_DIR/wolf" "$PACKAGE_ROOT/usr/lib/wolfenstein2026/wolf"
install -m 755 "$SCRIPT_DIR/wolfenstein2026" "$PACKAGE_ROOT/usr/bin/wolfenstein2026"
install -m 644 "$SCRIPT_DIR/wolfenstein2026.desktop" "$PACKAGE_ROOT/usr/share/applications/wolfenstein2026.desktop"
install -m 644 "$SCRIPT_DIR/wolfenstein2026.png" "$PACKAGE_ROOT/usr/share/icons/hicolor/512x512/apps/wolfenstein2026.png"
cp -R "$REPO_ROOT/assets" "$PACKAGE_ROOT/usr/share/wolfenstein2026/assets"
sed "s/@ARCH@/$DEB_ARCH/" "$SCRIPT_DIR/control.in" > "$PACKAGE_ROOT/DEBIAN/control"

dpkg-deb --build --root-owner-group "$PACKAGE_ROOT" "$DIST_DIR/$PACKAGE_NAME"
echo "Created $DIST_DIR/$PACKAGE_NAME"

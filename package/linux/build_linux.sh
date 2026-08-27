#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

if [[ "$(uname -s)" != "Linux" ]]; then
    echo "Linux packages must be built on a Linux host." >&2
    exit 1
fi

ARCH="$(uname -m)"
PACKAGE_NAME="wolfenstein2026-linux-$ARCH"
BUILD_DIR="$REPO_ROOT/build-linux"
DIST_DIR="$REPO_ROOT/dist"
STAGING_ROOT="$(mktemp -d)"
PACKAGE_DIR="$STAGING_ROOT/$PACKAGE_NAME"

cleanup() {
    rm -rf "$STAGING_ROOT"
}
trap cleanup EXIT

cmake -S "$REPO_ROOT" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_DIR"

mkdir -p "$PACKAGE_DIR/payload" "$DIST_DIR"
install -m 755 "$BUILD_DIR/wolf" "$PACKAGE_DIR/payload/wolf"
install -m 755 "$SCRIPT_DIR/wolfenstein2026" "$PACKAGE_DIR/payload/wolfenstein2026"
install -m 755 "$SCRIPT_DIR/install.sh" "$PACKAGE_DIR/install.sh"
cp -R "$REPO_ROOT/assets" "$PACKAGE_DIR/payload/assets"

tar -C "$STAGING_ROOT" -czf "$DIST_DIR/$PACKAGE_NAME.tar.gz" "$PACKAGE_NAME"
echo "Created $DIST_DIR/$PACKAGE_NAME.tar.gz"

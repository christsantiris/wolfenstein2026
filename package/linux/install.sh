#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DATA_HOME="${XDG_DATA_HOME:-$HOME/.local/share}"
INSTALL_ROOT="${WOLF_INSTALL_ROOT:-$DATA_HOME/wolfenstein2026/app}"
BIN_DIR="${WOLF_BIN_DIR:-$HOME/.local/bin}"

mkdir -p "$INSTALL_ROOT/assets" "$BIN_DIR"
install -m 755 "$SCRIPT_DIR/payload/wolf" "$INSTALL_ROOT/wolf"
install -m 755 "$SCRIPT_DIR/payload/wolfenstein2026" "$INSTALL_ROOT/wolfenstein2026"
cp -R "$SCRIPT_DIR/payload/assets/." "$INSTALL_ROOT/assets"
ln -sfn "$INSTALL_ROOT/wolfenstein2026" "$BIN_DIR/wolfenstein2026"

echo "Installed Wolfenstein 2026 to $INSTALL_ROOT"
echo "Launcher: $BIN_DIR/wolfenstein2026"
if [[ ":$PATH:" != *":$BIN_DIR:"* ]]; then
    echo "Add $BIN_DIR to PATH before launching from a terminal."
fi

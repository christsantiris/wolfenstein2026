#!/bin/bash
set -e

cd "$(dirname "$0")/../.."

APP_NAME="Wolfenstein 2026"
BUNDLE="${APP_NAME}.app"
DMG_NAME="wolfenstein2026.dmg"
STAGING="/tmp/wolf_dmg_staging"
BUNDLE_DIR="$STAGING/$BUNDLE"
MACOS_DIR="$BUNDLE_DIR/Contents/MacOS"
RESOURCES_DIR="$BUNDLE_DIR/Contents/Resources"
FRAMEWORKS_DIR="$BUNDLE_DIR/Contents/Frameworks"

echo "==> Generating icon..."
cc tools/gen_icon.c -o /tmp/wolf_gen_icon
/tmp/wolf_gen_icon
rm /tmp/wolf_gen_icon

echo "==> Converting icon to ICNS..."
ICONSET="/tmp/wolf.iconset"
rm -rf "$ICONSET"
mkdir "$ICONSET"
sips -s format png package/macos/icon.ppm --out /tmp/wolf_icon_src.png > /dev/null
for size in 16 32 128 256 512; do
    sips -z $size $size /tmp/wolf_icon_src.png --out "$ICONSET/icon_${size}x${size}.png" > /dev/null
    double=$((size * 2))
    sips -z $double $double /tmp/wolf_icon_src.png --out "$ICONSET/icon_${size}x${size}@2x.png" > /dev/null
done
sips -z 1024 1024 /tmp/wolf_icon_src.png --out "$ICONSET/icon_512x512@2x.png" > /dev/null
iconutil -c icns "$ICONSET" -o package/macos/wolf.icns
echo "    wrote package/macos/wolf.icns"

echo "==> Creating app bundle..."
rm -rf "$STAGING"
mkdir -p "$MACOS_DIR" "$RESOURCES_DIR" "$FRAMEWORKS_DIR"

cp build/wolf "$MACOS_DIR/wolf_bin"
cp -r assets "$RESOURCES_DIR/assets"
cp package/macos/wolf.icns "$RESOURCES_DIR/wolf.icns"

cat > "$BUNDLE_DIR/Contents/Info.plist" <<'PLIST'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleName</key>
    <string>Wolfenstein 2026</string>
    <key>CFBundleDisplayName</key>
    <string>Wolfenstein 2026</string>
    <key>CFBundleIdentifier</key>
    <string>com.wolfenstein2026.game</string>
    <key>CFBundleVersion</key>
    <string>1.0</string>
    <key>CFBundleShortVersionString</key>
    <string>1.0</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleExecutable</key>
    <string>wolf</string>
    <key>CFBundleIconFile</key>
    <string>wolf</string>
    <key>NSHighResolutionCapable</key>
    <true/>
    <key>LSMinimumSystemVersion</key>
    <string>12.0</string>
</dict>
</plist>
PLIST

# Launcher: sets CWD so the binary finds assets/,
# and routes saves/ to ~/Library/Application Support/Wolfenstein2026/saves
cat > "$MACOS_DIR/wolf" <<'LAUNCHER'
#!/bin/bash
RESOURCES="$(cd "$(dirname "$0")/../Resources" && pwd)"
SAVES_DIR="$HOME/Library/Application Support/Wolfenstein2026/saves"
mkdir -p "$SAVES_DIR"
if [ ! -e "$RESOURCES/saves" ]; then
    ln -sf "$SAVES_DIR" "$RESOURCES/saves"
fi
cd "$RESOURCES"
exec "$(dirname "$0")/wolf_bin" "$@"
LAUNCHER
chmod +x "$MACOS_DIR/wolf"

echo "==> Bundling dylibs..."

bundle_lib() {
    local src="$1"
    local libname
    libname=$(basename "$src")
    [[ -f "$FRAMEWORKS_DIR/$libname" ]] && return
    echo "    $libname"
    cp "$src" "$FRAMEWORKS_DIR/$libname"
    chmod 755 "$FRAMEWORKS_DIR/$libname"
    install_name_tool -id "@rpath/$libname" "$FRAMEWORKS_DIR/$libname"
    for dep in $(otool -L "$FRAMEWORKS_DIR/$libname" | awk '{print $1}' | grep -E '^/usr/local|^/opt/homebrew'); do
        local depname
        depname=$(basename "$dep")
        install_name_tool -change "$dep" "@rpath/$depname" "$FRAMEWORKS_DIR/$libname"
        bundle_lib "$dep"
    done
}

BINARY="$MACOS_DIR/wolf_bin"
for dep in $(otool -L "$BINARY" | awk '{print $1}' | grep -E '^/usr/local|^/opt/homebrew'); do
    depname=$(basename "$dep")
    install_name_tool -change "$dep" "@rpath/$depname" "$BINARY"
    bundle_lib "$dep"
done
install_name_tool -add_rpath "@executable_path/../Frameworks" "$BINARY"

echo "==> Creating DMG..."
rm -f "$DMG_NAME"
ln -s /Applications "$STAGING/Applications"
hdiutil create \
    -volname "$APP_NAME" \
    -srcfolder "$STAGING" \
    -ov -format UDZO \
    "$DMG_NAME"

rm -rf "$STAGING"
echo "==> Done: $DMG_NAME"

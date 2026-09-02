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
BINARY_SOURCE="${WOLF_BINARY:-build/wolf}"
UNIVERSAL_FRAMEWORKS="${WOLF_FRAMEWORK_DIR:-}"
SIGNING_IDENTITY="${CODESIGN_IDENTITY:-}"

if [ ! -f package/macos/wolf.icns ]; then
    echo "Missing package/macos/wolf.icns"
    exit 1
fi

echo "==> Creating app bundle..."
rm -rf "$STAGING"
mkdir -p "$MACOS_DIR" "$RESOURCES_DIR" "$FRAMEWORKS_DIR"

cp "$BINARY_SOURCE" "$MACOS_DIR/wolf"
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

BINARY="$MACOS_DIR/wolf"
if [ -n "$UNIVERSAL_FRAMEWORKS" ]; then
    ditto "$UNIVERSAL_FRAMEWORKS/SDL2.framework" "$FRAMEWORKS_DIR/SDL2.framework"
    ditto "$UNIVERSAL_FRAMEWORKS/SDL2_mixer.framework" "$FRAMEWORKS_DIR/SDL2_mixer.framework"
else
    for dep in $(otool -L "$BINARY" | awk '{print $1}' | grep -E '^/usr/local|^/opt/homebrew'); do
        depname=$(basename "$dep")
        install_name_tool -change "$dep" "@rpath/$depname" "$BINARY"
        bundle_lib "$dep"
    done
fi
install_name_tool -add_rpath "@executable_path/../Frameworks" "$BINARY"

if [ -n "$SIGNING_IDENTITY" ]; then
    echo "==> Signing bundled code..."
    while IFS= read -r framework; do
        codesign --force --sign "$SIGNING_IDENTITY" --options runtime --timestamp "$framework"
    done < <(find "$FRAMEWORKS_DIR" -type d -name '*.framework' -prune | sort)
    while IFS= read -r dylib; do
        codesign --force --sign "$SIGNING_IDENTITY" --options runtime --timestamp "$dylib"
    done < <(find "$FRAMEWORKS_DIR" -type f -name '*.dylib' | sort)
    codesign --force --sign "$SIGNING_IDENTITY" --options runtime --timestamp "$BINARY"
    codesign --force --sign "$SIGNING_IDENTITY" --options runtime --timestamp "$BUNDLE_DIR"
    codesign --verify --strict --verbose=2 "$BUNDLE_DIR"
fi

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

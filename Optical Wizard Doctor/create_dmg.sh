#!/bin/bash
# Create professional DMG installer for OCR-Orc macOS distribution
# Based on macOS best practices for DMG creation

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"
APP_BUNDLE="${BUILD_DIR}/bin/ocr-orc.app"
APP_NAME="OCR-Orc"
DMG_NAME="${APP_NAME}-Installer"
DMG_PATH="${SCRIPT_DIR}/${DMG_NAME}.dmg"
TEMP_DMG_DIR="${SCRIPT_DIR}/dmg_temp"
TEMP_DMG="${DMG_PATH}.temp.dmg"

echo "=========================================="
echo "OCR-Orc DMG Installer Creator"
echo "=========================================="
echo ""

# Check if app bundle exists
if [ ! -d "${APP_BUNDLE}" ]; then
    echo "ERROR: App bundle not found at ${APP_BUNDLE}"
    echo "Please build and deploy the app first:"
    echo "  ./deploy_macos.sh"
    exit 1
fi

echo "✓ App bundle found: ${APP_BUNDLE}"
echo ""

# Clean up any existing temp directory and DMG
rm -rf "${TEMP_DMG_DIR}"
rm -f "${DMG_PATH}" "${TEMP_DMG}"

# Create temp directory structure
mkdir -p "${TEMP_DMG_DIR}"

# Copy app to temp directory (cp strips resource forks automatically)
echo "Preparing DMG contents..."
cp -R "${APP_BUNDLE}" "${TEMP_DMG_DIR}/${APP_NAME}.app"
# Remove any extended attributes from the copied app
xattr -cr "${TEMP_DMG_DIR}/${APP_NAME}.app" 2>/dev/null || true

# Create Applications symlink
ln -s /Applications "${TEMP_DMG_DIR}/Applications"

# Create README
cat > "${TEMP_DMG_DIR}/README.txt" << 'EOF'
OCR-Orc Installation Instructions
================================

1. Drag "OCR-Orc.app" to the "Applications" folder
2. Open Applications and double-click OCR-Orc
3. If macOS shows a security warning:
   - Right-click the app → Open → Open
   - Or go to System Settings → Privacy & Security → Allow

For more information, visit the project repository.

Enjoy using OCR-Orc!
EOF

# Calculate DMG size (app size + 100MB overhead for filesystem)
APP_SIZE=$(du -sm "${APP_BUNDLE}" | cut -f1)
DMG_SIZE=$((APP_SIZE + 100))

echo "App size: ${APP_SIZE}MB"
echo "DMG size: ${DMG_SIZE}MB"
echo ""

# Create DMG with HFS+ filesystem
echo "Creating DMG..."
hdiutil create -srcfolder "${TEMP_DMG_DIR}" \
    -volname "${APP_NAME}" \
    -fs HFS+ \
    -fsargs "-c c=64,a=16,e=16" \
    -format UDRW \
    -size ${DMG_SIZE}M \
    "${TEMP_DMG}" > /dev/null 2>&1

# Mount the DMG
echo "Mounting DMG for customization..."
MOUNT_OUTPUT=$(hdiutil attach -readwrite -noverify -noautoopen "${TEMP_DMG}" 2>&1)

# Extract mount point - look for /Volumes/ path
MOUNT_POINT=$(echo "${MOUNT_OUTPUT}" | grep -E '/Volumes/' | tail -1 | awk '{for(i=3;i<=NF;i++) printf "%s ", $i; print ""}' | sed 's/[[:space:]]*$//')

if [ -z "${MOUNT_POINT}" ] || [ ! -d "${MOUNT_POINT}" ]; then
    echo "ERROR: Could not mount DMG or find mount point"
    echo "Mount output: ${MOUNT_OUTPUT}"
    exit 1
fi

echo "Mounted at: ${MOUNT_POINT}"

# Wait for mount to settle
sleep 3

# Configure DMG window appearance using AppleScript
echo "Configuring DMG window layout..."
VOLUME_NAME=$(basename "${MOUNT_POINT}")
osascript <<EOF
tell application "Finder"
    try
        tell disk "${VOLUME_NAME}"
            open
            delay 2
            set current view of container window to icon view
            set toolbar visible of container window to false
            set statusbar visible of container window to false
            set the bounds of container window to {400, 100, 920, 420}
            
            set viewOptions to the icon view options of container window
            set arrangement of viewOptions to not arranged
            set icon size of viewOptions to 72
            
            -- Position Applications symlink (right side)
            try
                set position of item "Applications" to {380, 205}
            end try
            
            -- Position app (left side)
            try
                set position of item "${APP_NAME}.app" to {120, 205}
            end try
            
            -- Position README (bottom center)
            try
                set position of item "README.txt" to {250, 320}
            end try
            
            delay 1
            close
            delay 1
        end tell
    on error errMsg
        log "Warning: Could not configure DMG window: " & errMsg
    end try
end tell
EOF

# Unmount
echo "Unmounting DMG..."
hdiutil detach "${MOUNT_POINT}" -force > /dev/null 2>&1 || hdiutil detach "${MOUNT_POINT}" > /dev/null 2>&1
sleep 1

# Convert to compressed, read-only DMG
echo "Compressing DMG..."
hdiutil convert "${TEMP_DMG}" \
    -format UDZO \
    -imagekey zlib-level=9 \
    -o "${DMG_PATH}" > /dev/null 2>&1

# Remove temp file
rm -f "${TEMP_DMG}"

# Clean up temp directory
rm -rf "${TEMP_DMG_DIR}"

# Verify DMG
if [ -f "${DMG_PATH}" ]; then
    DMG_SIZE_MB=$(du -hm "${DMG_PATH}" | cut -f1)
    echo ""
    echo "=========================================="
    echo "DMG Created Successfully!"
    echo "=========================================="
    echo ""
    echo "DMG location: ${DMG_PATH}"
    echo "DMG size: ${DMG_SIZE_MB}MB"
    echo ""
    echo "DMG is ready for distribution!"
    echo "Users can download and double-click to install."
    echo ""
else
    echo "ERROR: DMG creation failed!"
    exit 1
fi

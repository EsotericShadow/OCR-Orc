#!/bin/bash
# macOS Deployment Script for OCR-Orc
# CORRECT WORKFLOW based on official Qt documentation and research
# Sequence: Clean → Deploy → Clean → Sign → Entitlements → Verify

set -e  # Exit on error

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"
APP_BUNDLE="${BUILD_DIR}/bin/ocr-orc.app"
APP_NAME="OCR-Orc"

echo "=========================================="
echo "OCR-Orc macOS Deployment Script"
echo "CORRECT WORKFLOW - Based on Qt Docs"
echo "=========================================="
echo ""

# Check if app bundle exists
if [ ! -d "${APP_BUNDLE}" ]; then
    echo "ERROR: App bundle not found at ${APP_BUNDLE}"
    echo "Please build the project first:"
    echo "  cd build && cmake .. && cmake --build ."
    exit 1
fi

echo "✓ App bundle found: ${APP_BUNDLE}"
echo ""

# STEP 1: Clean resource forks BEFORE macdeployqt (CRITICAL)
echo "Step 1: Cleaning resource forks and extended attributes..."
xattr -cr "${APP_BUNDLE}" 2>/dev/null || true
find "${APP_BUNDLE}" -name ".DS_Store" -delete 2>/dev/null || true
find "${APP_BUNDLE}" -name "._*" -delete 2>/dev/null || true
echo "✓ Resource forks cleaned"
echo ""

# Check for macdeployqt
if ! command -v macdeployqt &> /dev/null; then
    echo "ERROR: macdeployqt not found!"
    echo "Please install Qt6 with Homebrew:"
    echo "  brew install qt@6"
    exit 1
fi

echo "✓ macdeployqt found: $(which macdeployqt)"
echo ""

# STEP 2: Deploy Qt frameworks (DO NOT sign yet)
echo "Step 2: Deploying Qt frameworks..."
macdeployqt "${APP_BUNDLE}" -verbose=2 -no-strip
if [ $? -eq 0 ]; then
    echo "✓ Qt frameworks deployed successfully"
else
    echo "✗ Failed to deploy Qt frameworks"
    exit 1
fi
echo ""

# STEP 3: Clean resource forks AGAIN (macdeployqt may add them)
echo "Step 3: Cleaning resource forks again (after macdeployqt)..."
xattr -cr "${APP_BUNDLE}" 2>/dev/null || true
find "${APP_BUNDLE}" -name ".DS_Store" -delete 2>/dev/null || true
find "${APP_BUNDLE}" -name "._*" -delete 2>/dev/null || true
echo "✓ Resource forks cleaned again"
echo ""

# STEP 4: Bundle Poppler libraries
echo "Step 4: Bundling Poppler libraries..."

POPPLER_LIB=$(pkg-config --variable=libdir poppler-cpp)/libpoppler-cpp.dylib
if [ ! -f "${POPPLER_LIB}" ]; then
    if [ -f "/opt/homebrew/opt/poppler/lib/libpoppler-cpp.dylib" ]; then
        POPPLER_LIB="/opt/homebrew/opt/poppler/lib/libpoppler-cpp.dylib"
    elif [ -f "/usr/local/opt/poppler/lib/libpoppler-cpp.dylib" ]; then
        POPPLER_LIB="/usr/local/opt/poppler/lib/libpoppler-cpp.dylib"
    else
        echo "WARNING: Could not find Poppler library"
    fi
fi

if [ -f "${POPPLER_LIB}" ]; then
    FRAMEWORKS_DIR="${APP_BUNDLE}/Contents/Frameworks"
    mkdir -p "${FRAMEWORKS_DIR}"
    
    rm -f "${FRAMEWORKS_DIR}/libpoppler-cpp.dylib" "${FRAMEWORKS_DIR}/libpoppler-cpp.2.dylib" 2>/dev/null || true
    
    echo "  Copying ${POPPLER_LIB} to ${FRAMEWORKS_DIR}/"
    cp "${POPPLER_LIB}" "${FRAMEWORKS_DIR}/"
    
    EXECUTABLE="${APP_BUNDLE}/Contents/MacOS/ocr-orc"
    install_name_tool -change "${POPPLER_LIB}" \
        "@executable_path/../Frameworks/$(basename ${POPPLER_LIB})" \
        "${EXECUTABLE}"
    
    echo "✓ Poppler libraries bundled successfully"
else
    echo "⚠ Poppler library not found - skipping"
fi
echo ""

# STEP 5: Copy resources
echo "Step 5: Copying resources..."
RESOURCES_DIR="${APP_BUNDLE}/Contents/Resources"
mkdir -p "${RESOURCES_DIR}"

# Copy AppIcon.icns (for window icon)
if [ -f "${SCRIPT_DIR}/AppIcon.icns" ]; then
    cp "${SCRIPT_DIR}/AppIcon.icns" "${RESOURCES_DIR}/AppIcon.icns"
    echo "✓ AppIcon.icns copied"
elif [ -d "${SCRIPT_DIR}/AppIcon.iconset" ]; then
    iconutil -c icns "${SCRIPT_DIR}/AppIcon.iconset" -o "${RESOURCES_DIR}/AppIcon.icns" 2>/dev/null && echo "✓ AppIcon.icns generated" || true
fi

# Copy OCR-Orc-LOGO.png (for toolbar logo)
if [ -f "${SCRIPT_DIR}/../OCR-Orc-LOGO.png" ]; then
    cp "${SCRIPT_DIR}/../OCR-Orc-LOGO.png" "${RESOURCES_DIR}/OCR-Orc-LOGO.png"
    echo "✓ OCR-Orc-LOGO.png copied"
fi

# Copy SVG icons for IconManager (must be in resources/icons/ subdirectory)
if [ -d "${SCRIPT_DIR}/resources/icons" ]; then
    mkdir -p "${RESOURCES_DIR}/resources/icons"
    cp -R "${SCRIPT_DIR}/resources/icons"/*.svg "${RESOURCES_DIR}/resources/icons/" 2>/dev/null || true
    echo "✓ SVG icons copied to resources/icons/"
fi
echo "✓ Resources copied"
echo ""

# STEP 6: Code Sign using macdeployqt (handles everything)
echo "Step 6: Code signing with macdeployqt (ad-hoc)..."
macdeployqt "${APP_BUNDLE}" -codesign=-
if [ $? -eq 0 ]; then
    echo "✓ Code signing completed"
else
    echo "⚠ macdeployqt signing had issues, trying manual signing..."
    # Fallback to manual signing
    find "${APP_BUNDLE}/Contents/Frameworks" -name "*.framework" -exec codesign --force --sign - {} \; 2>/dev/null || true
    find "${APP_BUNDLE}/Contents/Frameworks" -name "*.dylib" -exec codesign --force --sign - {} \; 2>/dev/null || true
    find "${APP_BUNDLE}/Contents/PlugIns" -type f -exec codesign --force --sign - {} \; 2>/dev/null || true
    codesign --force --sign - "${APP_BUNDLE}/Contents/MacOS/ocr-orc" 2>/dev/null || true
    codesign --force --sign - "${APP_BUNDLE}" 2>/dev/null || true
fi
echo ""

# STEP 7: Apply entitlements
echo "Step 7: Applying entitlements..."
ENTITLEMENTS="${SCRIPT_DIR}/entitlements.plist"
if [ -f "${ENTITLEMENTS}" ]; then
    codesign --force --sign - --entitlements "${ENTITLEMENTS}" "${APP_BUNDLE}" 2>&1 | grep -v "replacing existing signature" || true
    echo "✓ Entitlements applied"
else
    echo "⚠ No entitlements file found"
fi
echo ""

# STEP 8: Verify signature
echo "Step 8: Verifying signature..."
codesign --verify --verbose=4 "${APP_BUNDLE}" 2>&1 | head -5 || {
    echo "⚠ Signature verification had issues"
}
spctl -a -vv -t install "${APP_BUNDLE}" 2>&1 | head -3 || {
    echo "⚠ Gatekeeper check had issues (normal for ad-hoc signing)"
}
echo ""

# Final summary
echo "=========================================="
echo "Deployment Complete!"
echo "=========================================="
echo ""
echo "App bundle location: ${APP_BUNDLE}"
echo ""
echo "Next step: Create DMG installer"
echo "  ./create_dmg.sh"
echo ""
echo "Note: For ad-hoc signed apps, users will need to:"
echo "  - Right-click → Open on first launch"
echo "  - Or disable Gatekeeper temporarily"
echo ""


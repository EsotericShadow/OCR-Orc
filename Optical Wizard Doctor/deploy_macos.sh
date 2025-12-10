#!/bin/bash
# macOS Deployment Script for OCR-Orc
# This script bundles Qt frameworks and Poppler libraries into the app bundle

set -e  # Exit on error

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"
APP_BUNDLE="${BUILD_DIR}/bin/ocr-orc.app"
APP_NAME="OCR-Orc"

echo "=========================================="
echo "OCR-Orc macOS Deployment Script"
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

# Check for macdeployqt
if ! command -v macdeployqt &> /dev/null; then
    echo "ERROR: macdeployqt not found!"
    echo "Please install Qt6 with Homebrew:"
    echo "  brew install qt@6"
    exit 1
fi

echo "✓ macdeployqt found: $(which macdeployqt)"
echo ""

# Step 1: Deploy Qt frameworks and sign with ad-hoc signature
# macdeployqt can handle code signing internally, which avoids resource fork issues
echo "Step 1: Deploying Qt frameworks and code signing..."
macdeployqt "${APP_BUNDLE}" -verbose=2 -no-strip -codesign=-
if [ $? -eq 0 ]; then
    echo "✓ Qt frameworks deployed and signed successfully"
else
    echo "✗ Failed to deploy Qt frameworks"
    exit 1
fi
echo ""

# Step 2: Find and bundle Poppler libraries
echo "Step 2: Bundling Poppler libraries..."

# Find Poppler library location
POPPLER_LIB=$(pkg-config --variable=libdir poppler-cpp)/libpoppler-cpp.dylib
if [ ! -f "${POPPLER_LIB}" ]; then
    # Try Homebrew location
    if [ -f "/opt/homebrew/opt/poppler/lib/libpoppler-cpp.dylib" ]; then
        POPPLER_LIB="/opt/homebrew/opt/poppler/lib/libpoppler-cpp.dylib"
    elif [ -f "/usr/local/opt/poppler/lib/libpoppler-cpp.dylib" ]; then
        POPPLER_LIB="/usr/local/opt/poppler/lib/libpoppler-cpp.dylib"
    else
        echo "WARNING: Could not find Poppler library"
        echo "The app may not work correctly without Poppler bundled"
    fi
fi

if [ -f "${POPPLER_LIB}" ]; then
    FRAMEWORKS_DIR="${APP_BUNDLE}/Contents/Frameworks"
    mkdir -p "${FRAMEWORKS_DIR}"
    
    # Remove old copy if it exists (to fix permissions)
    rm -f "${FRAMEWORKS_DIR}/libpoppler-cpp.dylib" "${FRAMEWORKS_DIR}/libpoppler-cpp.2.dylib" 2>/dev/null || true
    
    # Copy Poppler library
    echo "  Copying ${POPPLER_LIB} to ${FRAMEWORKS_DIR}/"
    cp "${POPPLER_LIB}" "${FRAMEWORKS_DIR}/"
    
    # Update library paths in executable
    EXECUTABLE="${APP_BUNDLE}/Contents/MacOS/ocr-orc"
    install_name_tool -change "${POPPLER_LIB}" \
        "@executable_path/../Frameworks/$(basename ${POPPLER_LIB})" \
        "${EXECUTABLE}"
    
    # Update library paths in Poppler library itself
    POPPLER_BUNDLED="${FRAMEWORKS_DIR}/$(basename ${POPPLER_LIB})"
    
    # Find all dependencies of Poppler and copy them
    for DEP in $(otool -L "${POPPLER_BUNDLED}" | grep -E "^\s+/opt/homebrew|^\s+/usr/local" | awk '{print $1}' | grep -v "@executable_path"); do
        if [ -f "${DEP}" ]; then
            DEP_NAME=$(basename "${DEP}")
            echo "  Copying dependency: ${DEP_NAME}"
            cp "${DEP}" "${FRAMEWORKS_DIR}/"
            install_name_tool -change "${DEP}" \
                "@executable_path/../Frameworks/${DEP_NAME}" \
                "${POPPLER_BUNDLED}"
        fi
    done
    
    echo "✓ Poppler libraries bundled successfully"
else
    echo "⚠ Poppler library not found - skipping"
fi
echo ""

# Step 3: Copy resources (icons)
echo "Step 3: Copying resources..."
RESOURCES_DIR="${APP_BUNDLE}/Contents/Resources"
mkdir -p "${RESOURCES_DIR}"

# Copy icon resources if they exist
if [ -d "${SCRIPT_DIR}/resources/icons" ]; then
    echo "  Copying icons..."
    cp -r "${SCRIPT_DIR}/resources/icons" "${RESOURCES_DIR}/"
    echo "✓ Icons copied"
fi

# Copy app icon if it exists
if [ -f "${SCRIPT_DIR}/AppIcon.icns" ]; then
    echo "  Copying app icon..."
    cp "${SCRIPT_DIR}/AppIcon.icns" "${RESOURCES_DIR}/AppIcon.icns"
    echo "✓ App icon copied"
elif [ -d "${SCRIPT_DIR}/AppIcon.iconset" ]; then
    echo "  Converting iconset to .icns..."
    iconutil -c icns "${SCRIPT_DIR}/AppIcon.iconset" -o "${RESOURCES_DIR}/AppIcon.icns" 2>/dev/null
    if [ $? -eq 0 ]; then
        echo "✓ App icon created from iconset"
    else
        echo "⚠ Failed to create .icns from iconset"
    fi
else
    echo "  Note: AppIcon.icns not found. Creating placeholder..."
    echo "  To add a custom icon, create AppIcon.icns and place it in ${SCRIPT_DIR}/"
fi

# Copy logo if it exists (for UI display)
if [ -f "${SCRIPT_DIR}/../OCR-Orc-LOGO.png" ]; then
    echo "  Copying logo..."
    cp "${SCRIPT_DIR}/../OCR-Orc-LOGO.png" "${RESOURCES_DIR}/OCR-Orc-LOGO.png"
    echo "✓ Logo copied"
fi
echo ""

# Step 4: Verify deployment
echo "Step 4: Verifying deployment..."
if [ -d "${APP_BUNDLE}/Contents/Frameworks" ]; then
    FRAMEWORK_COUNT=$(find "${APP_BUNDLE}/Contents/Frameworks" -name "*.framework" | wc -l | tr -d ' ')
    echo "✓ Found ${FRAMEWORK_COUNT} Qt framework(s)"
fi

if [ -f "${APP_BUNDLE}/Contents/MacOS/ocr-orc" ]; then
    echo "✓ Executable found"
fi

if [ -f "${APP_BUNDLE}/Contents/Info.plist" ]; then
    echo "✓ Info.plist found"
fi
echo ""

# Step 5: Fix code signing (required for macOS to run the app)
echo "Step 5: Fixing code signing..."

# Aggressively remove extended attributes and resource forks that cause signing issues
echo "  Cleaning extended attributes..."
xattr -cr "${APP_BUNDLE}" 2>/dev/null || true
# Also remove .DS_Store and other Finder metadata
find "${APP_BUNDLE}" -name ".DS_Store" -delete 2>/dev/null || true
find "${APP_BUNDLE}" -name "._*" -delete 2>/dev/null || true
# Remove resource forks and problematic attributes from all binaries
find "${APP_BUNDLE}/Contents/MacOS" -type f -exec sh -c 'xattr -c "$1" 2>/dev/null; xattr -d com.apple.provenance "$1" 2>/dev/null; true' _ {} \; || true
find "${APP_BUNDLE}/Contents/Frameworks" -type f -exec sh -c 'xattr -c "$1" 2>/dev/null; xattr -d com.apple.provenance "$1" 2>/dev/null; true' _ {} \; || true
find "${APP_BUNDLE}/Contents/PlugIns" -type f -exec sh -c 'xattr -c "$1" 2>/dev/null; xattr -d com.apple.provenance "$1" 2>/dev/null; true' _ {} \; || true

# CRITICAL: Sign everything in proper order for ad-hoc signing
# For ad-hoc signing, we need to sign each component individually, then the bundle
# Sign order: dylibs -> framework binaries -> framework wrappers -> plugins -> executable -> app
# Use entitlements to disable library validation (allows different Team IDs)
ENTITLEMENTS="${SCRIPT_DIR}/entitlements.plist"

# Sign all dylibs first (deepest dependencies)
echo "  Signing dynamic libraries..."
find "${APP_BUNDLE}/Contents/Frameworks" -name "*.dylib" -type f -exec codesign --force --sign - {} \; 2>/dev/null || true

# Sign all framework binaries (the actual Mach-O files inside)
echo "  Signing Qt framework binaries..."
find "${APP_BUNDLE}/Contents/Frameworks" -name "*.framework" -type d | while read framework; do
    framework_name=$(basename "$framework" .framework)
    framework_binary="${framework}/Versions/A/${framework_name}"
    if [ -f "$framework_binary" ]; then
        echo "    Signing binary: ${framework_name}"
        codesign --force --sign - "$framework_binary" 2>&1 | grep -v "replacing existing signature" || true
    fi
done

# Sign framework wrappers
echo "  Signing Qt framework wrappers..."
find "${APP_BUNDLE}/Contents/Frameworks" -name "*.framework" -type d | while read framework; do
    framework_name=$(basename "$framework" .framework)
    echo "    Signing wrapper: ${framework_name}"
    codesign --force --sign - "$framework" 2>&1 | grep -v "replacing existing signature" || true
done

# Sign all plugins
echo "  Signing plugins..."
find "${APP_BUNDLE}/Contents/PlugIns" -type f \( -name "*.dylib" -o -name "*.so" \) -exec codesign --force --sign - {} \; 2>/dev/null || true

# Sign the main executable (strip resource fork by copying)
echo "  Signing executable..."
EXECUTABLE="${APP_BUNDLE}/Contents/MacOS/ocr-orc"
# Copy to strip resource forks (cp and ditto both strip resource forks)
TEMP_EXEC="${EXECUTABLE}.tmp"
cp "${EXECUTABLE}" "${TEMP_EXEC}" 2>/dev/null
mv "${TEMP_EXEC}" "${EXECUTABLE}"
chmod +x "${EXECUTABLE}"
# Remove all extended attributes (including resource forks)
xattr -c "${EXECUTABLE}" 2>/dev/null || true
# Try to sign - if it fails due to resource fork, we'll handle it
SIGN_OUTPUT=$(codesign --force --sign - "${EXECUTABLE}" 2>&1)
if echo "$SIGN_OUTPUT" | grep -q "resource fork"; then
    echo "    WARNING: Resource fork detected, using alternative method..."
    # Last resort: read binary and write it back (strips everything)
    cat "${EXECUTABLE}" > "${TEMP_EXEC}"
    mv "${TEMP_EXEC}" "${EXECUTABLE}"
    chmod +x "${EXECUTABLE}"
    codesign --force --sign - "${EXECUTABLE}" 2>&1 | grep -v "replacing existing signature" || {
        echo "    ERROR: Still cannot sign executable after resource fork removal"
        exit 1
    }
elif echo "$SIGN_OUTPUT" | grep -q "error"; then
    echo "    ERROR: Failed to sign executable: $SIGN_OUTPUT"
    exit 1
else
    echo "$SIGN_OUTPUT" | grep -v "replacing existing signature" || true
fi
echo "    ✓ Executable signed successfully"

# Finally, sign the entire app bundle with entitlements (without --deep to avoid re-signing)
# The --deep flag can cause issues, so we sign the bundle wrapper only
if [ -f "${ENTITLEMENTS}" ]; then
    echo "  Signing app bundle with entitlements..."
    codesign --force --sign - --entitlements "${ENTITLEMENTS}" "${APP_BUNDLE}" 2>&1 | grep -v -E "(resource fork|Permission denied|replacing existing signature)" || {
        echo "⚠ Code signing completed with warnings"
    }
else
    echo "  Signing app bundle..."
    codesign --force --sign - "${APP_BUNDLE}" 2>&1 | grep -v -E "(resource fork|Permission denied|replacing existing signature)" || {
        echo "⚠ Code signing completed with warnings"
    }
fi

# Verify the signature
echo "  Verifying signature..."
codesign --verify --verbose "${APP_BUNDLE}" 2>&1 | head -3 || {
    echo "⚠ Signature verification had issues"
}

echo "✓ Code signing complete"
echo ""

# Final summary
echo "=========================================="
echo "Deployment Complete!"
echo "=========================================="
echo ""
echo "App bundle location: ${APP_BUNDLE}"
echo ""
echo "To test the app:"
echo "  1. Open Finder"
echo "  2. Navigate to: ${APP_BUNDLE}"
echo "  3. Double-click 'OCR-Orc.app'"
echo ""
echo "To install to Applications:"
echo "  cp -r '${APP_BUNDLE}' /Applications/"
echo ""
echo "To add to Dock:"
echo "  Drag '${APP_BUNDLE}' to your Dock"
echo ""
echo "Note: First launch may require right-click → Open due to macOS security"
echo ""


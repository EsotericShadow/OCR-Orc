# DMG and Code Signing Issues

## Current Status

The app bundle has a persistent `com.apple.provenance` extended attribute on the main executable that prevents proper code signing. This attribute appears to be added during the build process itself and cannot be easily removed.

## Impact

1. **Code Signing**: `codesign` fails with: "resource fork, Finder information, or similar detritus not allowed"
2. **Finder Crashes**: Finder may crash when trying to display the app's metadata/icon
3. **Distribution**: DMG can be created but app signature is invalid

## Workarounds

### Option 1: Use Unsigned App (Current)
- DMG contains unsigned app bundle
- Users must disable Gatekeeper or use workarounds
- App functions normally once launched

### Option 2: Build in Non-iCloud Location
The provenance xattr may be added because the build directory is in an iCloud-synced location. Try:
1. Move project to `/Users/main/Developer/` (not synced)
2. Rebuild from scratch
3. Check if provenance xattr is absent

### Option 3: Use Developer ID (Paid)
- Get Apple Developer account ($99/year)
- Use Developer ID certificate instead of ad-hoc
- Notarize the app
- This bypasses the resource fork restriction

### Option 4: Strip Resource Fork at Build Time
Modify CMake to strip xattrs immediately after building:
```cmake
add_custom_command(TARGET ocr-orc POST_BUILD
    COMMAND xattr -cr $<TARGET_FILE:ocr-orc>
    COMMAND xattr -d com.apple.provenance $<TARGET_FILE:ocr-orc> || true
)
```

## Testing DMG

The DMG can be tested without opening in Finder:
```bash
# Mount read-only
hdiutil attach OCR-Orc-Installer.dmg -readonly -noverify

# Check contents
ls -la /Volumes/OCR-Orc*/

# Unmount
hdiutil detach /Volumes/OCR-Orc*
```

## Next Steps

1. **Immediate**: Test if DMG works when opened via Terminal instead of Finder
2. **Short-term**: Try building in non-iCloud location
3. **Long-term**: Consider Developer ID for production distribution

## Known Limitations

- App cannot be properly code-signed with ad-hoc signature
- Finder may crash when displaying app metadata
- Users will see stronger Gatekeeper warnings
- Not suitable for App Store distribution


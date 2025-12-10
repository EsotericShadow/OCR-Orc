# OCR-Orc Distribution Guide

## Overview

This document explains how to distribute OCR-Orc without a paid Apple Developer account. The app is **fully functional** but requires users to bypass Gatekeeper warnings on first launch.

## Distribution Status

- ✅ **App Bundle**: Complete and functional
- ✅ **Code Signing**: Ad-hoc signed (works locally)
- ✅ **DMG Installer**: Created (`OCR-Orc-Installer.dmg`)
- ⚠️ **Notarization**: Not available (requires paid Developer ID)
- ⚠️ **Gatekeeper**: Users will see security warning on first launch

## What Users Will Experience

### First Launch
1. User downloads `OCR-Orc-Installer.dmg`
2. User opens DMG and drags app to Applications
3. User tries to launch app
4. **macOS shows**: "OCR-Orc cannot be opened because the developer cannot be verified"

### Solutions for Users

**Option A (Recommended - Easiest):**
- Right-click the app → **Open** → Click **"Open"** in the dialog
- This only needs to be done once per user

**Option B (System Settings):**
- Go to **System Settings** → **Privacy & Security**
- Scroll down to find the OCR-Orc warning
- Click **"Open Anyway"**

**Option C (Terminal - Advanced):**
```bash
sudo xattr -rd com.apple.quarantine /Applications/OCR-Orc.app
```

After the first launch, the app opens normally.

## Distribution Options

### Option 1: Direct Download
- Host `OCR-Orc-Installer.dmg` on your website/GitHub releases
- Include clear instructions about the first-launch warning
- Most users are familiar with this process

### Option 2: GitHub Releases
- Upload DMG to GitHub Releases
- Add installation instructions in release notes
- Free and reliable hosting

### Option 3: Homebrew Cask (Advanced)
- Create a Homebrew cask formula
- Users can install via: `brew install --cask ocr-orc`
- Homebrew handles the quarantine removal automatically

## Future: Getting a Developer ID (Optional)

If you want to eliminate Gatekeeper warnings:

1. **Sign up for Apple Developer Program**: $99/year
   - https://developer.apple.com/programs/
   
2. **Get Developer ID Certificate**:
   - Xcode → Preferences → Accounts → Manage Certificates
   - Create "Developer ID Application" certificate

3. **Update Signing**:
   - Replace ad-hoc signing (`-`) with Developer ID
   - Add notarization step
   - Update `deploy_macos_CORRECT.sh`

4. **Notarize**:
   ```bash
   xcrun notarytool submit OCR-Orc-Installer.dmg \
     --apple-id your@email.com \
     --team-id YOUR_TEAM_ID \
     --password app-specific-password \
     --wait
   ```

5. **Staple**:
   ```bash
   xcrun stapler staple OCR-Orc-Installer.dmg
   ```

## Current DMG Contents

- `OCR-Orc.app` - The application bundle
- `Applications` - Symlink to /Applications for easy drag-and-drop
- `README.txt` - Installation instructions

## Technical Details

### Code Signing
- **Type**: Ad-hoc signing (`codesign --sign -`)
- **Entitlements**: `com.apple.security.cs.disable-library-validation` (allows unsigned libraries)
- **Status**: Valid signature, but not from trusted developer

### App Bundle Contents
- ✅ All Qt frameworks bundled
- ✅ QtDBus framework included
- ✅ Poppler libraries bundled
- ✅ All SVG icons in `Resources/resources/icons/`
- ✅ App icon (`AppIcon.icns`)
- ✅ Logo (`OCR-Orc-LOGO.png`)

### File Size
- App Bundle: ~88 MB
- DMG (compressed): ~60-70 MB

## Troubleshooting

### User Can't Open App
- Ensure they follow Option A, B, or C above
- Check System Settings → Privacy & Security for blocked apps
- Verify app is in `/Applications/` (not Downloads)

### App Crashes on Launch
- Check Console.app for crash logs
- Verify all frameworks are bundled correctly
- Ensure macOS version is 10.15+ (Catalina or later)

### Icons Not Showing
- Verify `Resources/resources/icons/` exists in app bundle
- Check that SVG files are present
- IconManager should find them automatically

## Summary

**You CAN distribute without a Developer ID!** Users just need to:
1. Right-click → Open on first launch
2. That's it - works perfectly after that

The app is production-ready, just not notarized. Many open-source macOS apps distribute this way.


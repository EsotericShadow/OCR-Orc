# Complete Qt macOS Deployment Workflow
## Based on Official Qt Documentation and Best Practices (December 2025)

### Critical Sequence (DO NOT DEVIATE):

1. **Build Application**
   - Build in Release mode
   - Ensure `MACOSX_BUNDLE TRUE` is set in CMake
   - Output: `build/bin/ocr-orc.app`

2. **Clean Resource Forks/Extended Attributes (CRITICAL FIRST STEP)**
   ```bash
   xattr -cr build/bin/ocr-orc.app
   find build/bin/ocr-orc.app -name ".DS_Store" -delete
   find build/bin/ocr-orc.app -name "._*" -delete
   ```
   - **Why:** Resource forks prevent code signing
   - **When:** BEFORE macdeployqt AND before any signing attempts

3. **Deploy Qt Frameworks**
   ```bash
   macdeployqt build/bin/ocr-orc.app -verbose=2 -no-strip
   ```
   - Copies Qt frameworks and plugins into bundle
   - **DO NOT** use `-codesign` here if resource forks are present

4. **Clean Resource Forks Again (macdeployqt may add them)**
   ```bash
   xattr -cr build/bin/ocr-orc.app
   ```

5. **Code Sign (Ad-Hoc for Distribution)**
   
   **Option A: Using macdeployqt (Recommended)**
   ```bash
   macdeployqt build/bin/ocr-orc.app -codesign=-
   ```
   - The `-` means ad-hoc signing
   - Signs all frameworks, plugins, and executable
   
   **Option B: Manual Signing (If Option A fails)**
   ```bash
   # Sign frameworks first
   find build/bin/ocr-orc.app/Contents/Frameworks -name "*.framework" -exec codesign --force --sign - {} \;
   find build/bin/ocr-orc.app/Contents/Frameworks -name "*.dylib" -exec codesign --force --sign - {} \;
   # Sign plugins
   find build/bin/ocr-orc.app/Contents/PlugIns -type f -exec codesign --force --sign - {} \;
   # Sign executable
   codesign --force --sign - build/bin/ocr-orc.app/Contents/MacOS/ocr-orc
   # Sign bundle
   codesign --force --sign - build/bin/ocr-orc.app
   ```

6. **Apply Entitlements**
   ```bash
   codesign --force --sign - --entitlements entitlements.plist build/bin/ocr-orc.app
   ```

7. **Verify Signature**
   ```bash
   codesign --verify --verbose=4 build/bin/ocr-orc.app
   spctl -a -vv -t install build/bin/ocr-orc.app
   ```

8. **Create DMG**
   ```bash
   # Use macdeployqt
   macdeployqt build/bin/ocr-orc.app -dmg
   # OR use custom script (create_dmg.sh)
   ```

9. **Sign DMG (Optional but Recommended)**
   ```bash
   codesign --force --sign - OCR-Orc-Installer.dmg
   ```

### Key Insights from Research:

1. **Resource Forks are the #1 Problem**
   - Must be removed BEFORE signing
   - Use `xattr -cr` recursively
   - Clean after macdeployqt too

2. **macdeployqt Can Sign**
   - Use `-codesign=-` for ad-hoc
   - Handles all components automatically
   - BUT: Only works if resource forks are removed first

3. **Ad-Hoc Signing Limitations**
   - Works for distribution
   - Users must right-click → Open on first launch
   - No notarization possible (requires Developer ID)

4. **Signing Order Matters**
   - Frameworks → Plugins → Executable → Bundle
   - macdeployqt handles this automatically

5. **Build Cleanliness**
   - Rebuild if resource forks persist
   - Check for `com.apple.provenance` attribute
   - May need to rebuild in clean directory

### Troubleshooting:

**If signing fails with "resource fork" error:**
1. Rebuild the app completely
2. Clean resource forks BEFORE macdeployqt
3. Clean again AFTER macdeployqt
4. Then sign

**If app won't open:**
1. Check signature: `codesign -vvv app.app`
2. Check Gatekeeper: `spctl -a -vv -t install app.app`
3. Try right-click → Open
4. Check Console.app for crash logs

### For Production Distribution:

1. Get Developer ID certificate ($99/year)
2. Sign with Developer ID instead of `-`
3. Notarize with Apple
4. Staple notarization ticket


# Phase 2.3: Region Creation - Test Checklist

## Application Status
- ✅ Build: Successful
- ✅ Application: Running
- ✅ Logs: Tracking to `/tmp/owd_test.log`

---

## Test Actions (Perform in Order)

### 1. Basic UI Verification
- [x] Application window opens
- [x] All UI elements visible:
  - [x] Toolbar with Mode buttons (Create/Select)
  - [x] Control Panel with "Load PDF" button
  - [x] Canvas area (should show "No document loaded")
  - [x] Side Panel with Regions/Groups tabs
- [x] Text is readable (black on light backgrounds)
- [x] No crashes or errors visible
- *there is one issue i noticed, its that the buttons with long text when the screen isnt as wide(less than 1000px wide perse) the button text is being cut off by the size of the button. this is fine for development. but we will need to address this at somepoint(tech debt) possibly by assigning Icons to those longer text buttons, and giving them a tooltip(planned) 

### 2. PDF Loading
- [x] Click "Load PDF" button
- [x] File dialog opens
- [x] Select a PDF file
- [x] PDF loads successfully
- [x] PDF first page displays in canvas
- [x] Image is centered and scaled properly
- [x] File name appears in control panel
- [x] Status bar shows "PDF loaded successfully"

### 3. Mode Switching
- [x] "Create" mode button is selected by default
- [x] Cursor changes to crosshair in Create mode
- [x] Click "Select/Move" mode button
- [x] Cursor changes to arrow in Select mode
- [x] Click "Create" mode button again
- [x] Cursor returns to crosshair

### 4. Region Creation - Basic
- [x] Ensure in "Create" mode
- [x] Ensure PDF is loaded
- [x] Click and hold on PDF image
- [x] Drag to create a rectangle
- [x] **OBSERVE:** Red dashed rectangle appears while dragging
- [x] Release mouse button ** observed that they are blue. not red.
- [x] **OBSERVE:** Region appears with colored border
- [x] **OBSERVE:** Region name appears above region
- [x] **OBSERVE:** Region appears in side panel "Regions" tab

### 5. Region Creation - With Name
- [x] Enter region name in "Region Name" field (e.g., "Test Region 1")
- [x] Select a color from dropdown (e.g., "red")
- [x] Enter group name (optional, e.g., "testgroup")
- [x] Click and drag on PDF to create region * while in create? this isnt how groups are created. groups are create in select/move by selecting multiple(2 or more) regions and then creating a group, either in the side panel, or in right click drop down, or by pressing command+g while having the regions selected. this is how its stored correctly right?
- [x] **OBSERVE:** Region created with specified name
- [x] **OBSERVE:** Region has correct color
- [x] **OBSERVE:** Region appears in side panel with correct name

### 6. Region Creation - Auto-Naming
- [x] Leave "Region Name" field empty
- [x] Click and drag to create region
- [x] **OBSERVE:** Region created with auto-generated name (e.g., "Region 1")
- [x] Create another region with empty name
- [x] **OBSERVE:** Second region named "Region 2"
- [x] Enter "Cell 1" in name field
- [x] Create a region
- [x] **OBSERVE:** Next region name auto-increments to "Cell 2" ** this does not work if i name the region "xx" or something, only with "Cell"??

### 7. Region Creation - Validation
- [x] Try to create a very small region (< 10x10 pixels)
- [x] **OBSERVE:** Region is rejected (no region created)
- [x] Try to create region outside PDF bounds
- [x] **OBSERVE:** Region is clamped to bounds or rejected
- [x] Try to create region with duplicate name
- [x] **OBSERVE:** Warning message appears, region not created ** this should probably just auto increment as {name}_n

### 8. Multiple Regions
- [x] Create 3-4 regions with different names and colors
- [x] **OBSERVE:** All regions display correctly
- [x] **OBSERVE:** All regions appear in side panel
- [x] **OBSERVE:** Each region has correct color
- [x] **OBSERVE:** Each region has label above it

### 9. Visual States (If Selection Works)
- [!] Move mouse over a region
- [!] **OBSERVE:** Region border thickens (hover state)
- [!] Click on a region (if in Select mode)
- [!] **OBSERVE:** Region border thickens more (selected state)
- [!] **OBSERVE:** Resize handles appear (if single selection)

### 10. Error Handling
- [x] Try to create region before loading PDF
- [x] **OBSERVE:** No region created (or appropriate error)
- [!] Try to create region with invalid coordinates
- [!] **OBSERVE:** Region rejected appropriately ** i couldnt figure out how to do this, but im sure its good! so maybe x, and x for the one above. 

---

## What to Report

After testing, please report:

1. **What worked:**
   - List all features that worked as expected

2. **What didn't work:**
   - List any features that failed or behaved unexpectedly
   - Describe the exact behavior you observed

3. **Visual issues:**
   - Any rendering problems
   - Any UI layout issues
   - Any text visibility issues

4. **Errors or crashes:**
   - Any error messages
   - Any application crashes
   - When they occurred (during which action)

5. **Performance:**
   - Any lag or slowness
   - Any UI freezing

---

## Logs Location

Application logs are being tracked to: `/tmp/owd_test.log`

I will analyze the logs after you report your findings.


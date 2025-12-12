# Detection Analysis - Student Registration Form

## Detection Results Summary
- **Total Detected:** 41 regions
- **All High Confidence:** 41 regions (96.5-97.1%)
- **Detection Method:** Rectangle detection ("rectang")
- **Groups Found:** 4 groups
  - Group_1: 20 regions (yellow)
  - Group_2: 16 regions (yellow)
  - Lastname: 16 regions (blue)
  - Studentnumber: 16 regions (green)

## Expected Form Fields (What SHOULD Be Detected)

### Student Information Section:
1. **Last Name** - Empty text field
2. **First Name** - Empty text field
3. **Mailing Address** - Empty text field
4. **Postal Code** - Empty text field (currently only detecting the dash "-")
5. **Telephone** - Empty text field
6. **Email** - Empty text field
7. **Date of Birth (DD/MM/YYYY)** - Empty text field
8. **Student #** - Empty text field (shows "3" - might be pre-filled)
9. **Have you take courses with us before?** - Checkbox (Yes/No)
10. **Do you self-identify as -** - Multiple checkboxes (First Nations, Metis, Inuit, Visible Minority, Other)

### Employer/Sponsor Information Section:
11. **Employer Name** - Empty text field
12. **Sponsor Name** - Empty text field

### Course Section:
13. **KIST SAFETY TRAINING** - Multiple checkboxes:
    - Orientation to Workplace Safety
    - Working at Height/Equipment
    - Fall Protection
    - Confined Space Entry & Standby
    - Lockout/Tagout Control (LOTO)
    - Arc Flash Awareness
    - Rigging/Signalperson (Level 1)
    - Rigging/Signalperson (Level 2)
    - Equipment/Vehicle Spotter
    - Traffic Control
    - Transportation of Dangerous Goods
    - Electrical Safety
    - Site Traffic Control Person (TCP)

14. **IVES OPERATOR CERTIFICATION** - Multiple checkboxes:
    - Standard Forklift
    - Narrow Aisle Forklift
    - Orderpicker
    - MDRP
    - Pallet Mover
    - Wheel Loader
    - Skid-Steer
    - Excavator
    - Other

## What Was Actually Detected

### Issues Observed:
1. **Numbers/OCR Artifacts Detected:**
   - "31 397 8 121415166 33325 3389 10" above Last Name field
   - "3 2 3 4 1" near title
   - "302928 35272625242336222120191837" below First Name
   - "11 1317" and "41" near Postal Code
   - These appear to be OCR artifacts or numbers, NOT empty form fields

2. **Grouping Issues:**
   - "Lastname" group has 16 regions (should be 1 field)
   - "Studentnumber" group has 16 regions (should be 1 field)
   - This suggests individual characters/numbers are being detected, not whole fields

3. **Missing Fields:**
   - Most actual empty form fields are likely NOT detected
   - Checkboxes are probably not detected
   - Only detecting rectangles/boxes that contain numbers or have visible borders

## Analysis: What's Wrong?

### Problem 1: Detecting Numbers Instead of Empty Fields
- The detection is finding regions that contain numbers/text
- Empty form fields (which should be bright/empty) are being missed
- The "Very Lenient" settings might be TOO lenient, allowing text-containing regions

### Problem 2: Character-Level Detection
- Groups with 16 regions suggest individual characters are being detected
- Should detect whole fields, not individual characters
- This might be from rectangle detection finding small boxes around characters

### Problem 3: Missing Empty Fields
- Empty form fields (Last Name, First Name, etc.) are likely not detected
- These should be bright, empty rectangles with borders
- The edge density thresholds might still be filtering them out

## Root Cause Analysis

### Problem 1: Character-Level Detection
**Evidence:**
- "Lastname" group: 16 regions (should be 1 field)
- "Studentnumber" group: 16 regions (should be 1 field)
- Rectangle detector minimum size: 15x10 pixels (too small)

**What's happening:**
- Rectangle detection is finding small boxes around individual characters/numbers
- These small regions pass the text filter (they're empty boxes around characters)
- They get grouped together, but should be merged into single field regions

### Problem 2: Numbers/OCR Artifacts Being Detected
**Evidence:**
- Numbers like "31 397 8 121415166" are being detected as regions
- These are likely OCR artifacts or pre-filled numbers
- They should be filtered out as text-containing regions

**What's happening:**
- OCR finds these numbers
- Rectangle detection finds boxes around them
- Text filtering might not be catching them if they're small enough

### Problem 3: Missing Empty Form Fields
**Expected:**
- Last Name field: ~200-300px wide, ~30-40px tall
- First Name field: ~200-300px wide, ~30-40px tall
- Postal Code field: ~100-150px wide, ~30-40px tall
- Other text fields: Similar sizes

**What's happening:**
- Large empty form fields might not have strong enough borders
- Edge density thresholds might be filtering them out
- Rectangle detection might not be finding them if borders are faint

## Recommendations

### Fix 1: Increase Minimum Rectangle Size
**Current:** `minWidth_ = 15px, minHeight_ = 10px`  
**Recommended:** `minWidth_ = 30px, minHeight_ = 20px`

**Why:**
- Filters out character-level detections
- Form fields are typically 30px+ wide
- Still catches small checkboxes (if needed)

### Fix 2: Add Region Merging Logic
**Problem:** Multiple small regions should be merged into one field

**Solution:**
- After detection, find nearby small regions (< 50px apart)
- Merge them into larger bounding boxes
- Only merge if they're horizontally aligned (same row)

### Fix 3: Improve Text Filtering
**Current OCR Overlap:** 0.20 (20%)  
**Issue:** Small regions might not overlap enough with OCR text

**Solution:**
- For small regions (< 50px width), use stricter OCR overlap (0.10 or 0.05)
- For large regions (> 100px width), current threshold is OK
- Add size-based filtering: reject regions that are too small AND have OCR overlap

### Fix 4: Better Edge Density Logic
**Current:** Horizontal edge density = 0.25 threshold  
**Issue:** Form field borders have horizontal edges, but so does text

**Solution:**
- Distinguish: Text has dense horizontal lines (many lines close together)
- Form fields have sparse horizontal edges (just borders)
- Use line spacing analysis: if horizontal lines are too close together, it's text

### Fix 5: Checkbox Detection
**Current:** Checkboxes might not be detected  
**Solution:**
- Ensure checkbox detector is running
- Checkboxes should be detected as "square" type
- Minimum size for checkboxes: 10x10px (smaller than text fields)

## Immediate Actions

1. **Increase minimum rectangle size** to filter out character-level detections
2. **Add region merging** to combine nearby small regions into fields
3. **Improve text filtering** for small regions
4. **Verify checkbox detection** is working
5. **Add debug logging** to show region sizes and positions

## Expected Results After Fixes

**Before:** 41 regions (many character-level, missing actual fields)  
**After:** ~15-20 regions (actual form fields + checkboxes)

**Fields that should be detected:**
- Last Name ✓
- First Name ✓
- Mailing Address ✓
- Postal Code ✓
- Telephone ✓
- Email ✓
- Date of Birth ✓
- Student # ✓
- Employer Name ✓
- Sponsor Name ✓
- All checkboxes ✓


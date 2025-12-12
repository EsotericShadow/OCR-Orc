# Magic Detect - 3 Drastic Test Configurations

## Configuration 1: "Very Lenient" (Catch Everything)
**Goal:** Detect as many regions as possible, even if some are false positives
**Use when:** You're getting too few detections and want to see what's being filtered out

### Settings:
- **OCR Overlap Threshold:** `0.20` (20% - more lenient, allows nearby labels)
- **Base Brightness:** `0.50` (50% - much lower, accepts darker regions)
- **Brightness Adaptive Factor:** `0.70` (70% - more lenient adaptive threshold)
- **Total Edge Density:** `0.15` (0.15 - higher, allows more edges)
- **Horizontal Edge Density:** `0.25` (0.25 - much higher, allows borders)
- **Vertical Edge Density:** `0.15` (0.15 - higher, allows vertical borders)
- **IoU Threshold:** `0.40` (40% - more lenient matching)
- **OCR Confidence:** `40.0` (40% - lower, includes lower confidence text)
- **Horizontal Overfit:** `50.0%` (50% - more expansion)
- **Vertical Overfit:** `70.0%` (70% - more expansion)
- **Min Horizontal Lines:** `4` (4 - need more lines to reject)
- **Strict Consensus:** `OFF` (Lenient mode)

**Expected Result:** Should detect many more regions, including some that might be false positives (like borders, lines, etc.)

---

## Configuration 2: "Very Strict" (Only Perfect Matches)
**Goal:** Only detect regions that are clearly empty form fields with high confidence
**Use when:** You're getting too many false positives and want only the best matches

### Settings:
- **OCR Overlap Threshold:** `0.05` (5% - very strict, no text overlap allowed)
- **Base Brightness:** `0.85` (85% - very high, only bright empty fields)
- **Brightness Adaptive Factor:** `0.95` (95% - very strict adaptive)
- **Total Edge Density:** `0.05` (0.05 - very low, rejects anything with edges)
- **Horizontal Edge Density:** `0.05` (0.05 - very low, no text lines)
- **Vertical Edge Density:** `0.03` (0.03 - very low, minimal borders)
- **IoU Threshold:** `0.75` (75% - strict matching required)
- **OCR Confidence:** `80.0` (80% - only high confidence OCR)
- **Horizontal Overfit:** `20.0%` (20% - minimal expansion)
- **Vertical Overfit:** `30.0%` (30% - minimal expansion)
- **Min Horizontal Lines:** `1` (1 - even 1 line rejects it)
- **Strict Consensus:** `ON` (Strict mode - both pipelines must agree)

**Expected Result:** Should detect very few regions, only the most obvious empty form fields with no text, no borders, very bright

---

## Configuration 3: "Edge-Friendly" (For Forms with Borders)
**Goal:** Detect empty fields even when they have borders/edges (like your form)
**Use when:** Your form has visible borders around fields (like boxes, lines, etc.)

### Settings:
- **OCR Overlap Threshold:** `0.15` (15% - moderate, allows some label proximity)
- **Base Brightness:** `0.65` (65% - moderate, accepts fields with slight shadows)
- **Brightness Adaptive Factor:** `0.80` (80% - moderate adaptive)
- **Total Edge Density:** `0.20` (0.20 - HIGH, allows borders/edges)
- **Horizontal Edge Density:** `0.30` (0.30 - HIGH, allows horizontal borders)
- **Vertical Edge Density:** `0.25` (0.25 - HIGH, allows vertical borders)
- **IoU Threshold:** `0.55` (55% - moderate matching)
- **OCR Confidence:** `55.0` (55% - moderate OCR confidence)
- **Horizontal Overfit:** `45.0%` (45% - good expansion)
- **Vertical Overfit:** `65.0%` (65% - good expansion)
- **Min Horizontal Lines:** `3` (3 - need multiple text lines to reject)
- **Strict Consensus:** `OFF` (Lenient mode)

**Expected Result:** Should detect empty form fields even when they have visible borders, boxes, or lines around them. This is likely what you need for your form!

---

## How to Use:

1. Click **Magic Detect** button
2. In the parameter dialog, manually set each value OR use the **Reset to Defaults** button first
3. Set all values according to one of the configurations above
4. Click **Run Magic Detect**
5. Review results
6. Try the next configuration and compare

## Quick Tips:

- **If you get too few detections:** Try Configuration 1 (Very Lenient)
- **If you get too many false positives:** Try Configuration 2 (Very Strict)  
- **If your form has borders/boxes:** Try Configuration 3 (Edge-Friendly) ← **Start here!**

## What to Look For:

- **Good detection:** Empty form fields are detected (like "Last Name", "First Name", etc.)
- **Bad detection:** 
  - Text labels are detected (should be filtered out)
  - Borders/lines are detected as separate regions
  - Checkboxes are detected as text fields
  - Non-form areas are detected


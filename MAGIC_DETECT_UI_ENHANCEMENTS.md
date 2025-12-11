# Magic Detect UI Enhancements

## Overview
Two major UI enhancements to improve Magic Detect usability and flexibility:
1. **Magic Detect Settings Dialog** - Adjustable parameters and weights
2. **Spot Magic Detect** - Region-based detection mode

---

## 1. Magic Detect Settings Dialog

### Purpose
Allow users to fine-tune Magic Detect parameters in real-time without code changes.

### UI Design

#### Dialog Layout
```
┌─────────────────────────────────────────────────────┐
│  Magic Detect Settings                              │
├─────────────────────────────────────────────────────┤
│                                                      │
│  Detection Sensitivity                               │
│  ┌────────────────────────────────────────────┐   │
│  │ ●●●○○○○○○○  [70%]                          │   │
│  └────────────────────────────────────────────┘   │
│                                                      │
│  Pipeline Weights                                    │
│  ┌────────────────────────────────────────────┐   │
│  │ OCR-First Pipeline:     ●●●●○○○○○○ [40%]     │   │
│  │ Rectangle Detection:    ●●●●○○○○○○ [40%]     │   │
│  │ Consensus Matching:     ●●○○○○○○○○ [20%]    │   │
│  └────────────────────────────────────────────┘   │
│                                                      │
│  Rectangle Detection Parameters                     │
│  ┌────────────────────────────────────────────┐   │
│  │ Sensitivity:           ●●●●○○○○○○ [40%]     │   │
│  │ Min Width:            [15] px                │   │
│  │ Min Height:           [10] px                │   │
│  │ Max Width:            [800] px                │   │
│  │ Max Height:           [300] px                │   │
│  │ Overfit Expansion:    ●●●●●○○○○○ [50%]      │   │
│  └────────────────────────────────────────────┘   │
│                                                      │
│  OCR-First Parameters                                │
│  ┌────────────────────────────────────────────┐   │
│  │ Overfit Horizontal:    ●●●●○○○○○○ [40%]      │   │
│  │ Overfit Vertical:     ●●●●●●○○○○ [60%]      │   │
│  │ Hard Edge Sensitivity: ●●●●●○○○○○ [50%]     │   │
│  │ Cell Group Detection:  ☑ Enabled            │   │
│  └────────────────────────────────────────────┘   │
│                                                      │
│  Matching Thresholds                                 │
│  ┌────────────────────────────────────────────┐   │
│  │ IoU Threshold:         ●●●○○○○○○○ [30%]     │   │
│  │ Overlap Threshold:    ●●●●○○○○○○ [40%]     │   │
│  │ Consensus Required:   ☐ (unchecked)        │   │
│  └────────────────────────────────────────────┘   │
│                                                      │
│  Detection Types to Include                          │
│  ┌────────────────────────────────────────────┐   │
│  │ ☑ Text Input Fields                         │   │
│  │ ☑ Text Lines                                │   │
│  │ ☑ Text Blocks                               │   │
│  │ ☑ Checkboxes                                │   │
│  │ ☑ Grid Cells                                │   │
│  │ ☑ Squares                                   │   │
│  │ ☑ Rectangles                                │   │
│  └────────────────────────────────────────────┘   │
│                                                      │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐         │
│  │  Reset   │  │  Preview │  │  Apply    │         │
│  └──────────┘  └──────────┘  └──────────┘         │
│                                                      │
└─────────────────────────────────────────────────────┘
```

### Implementation

#### New Files
- `src/ui/components/dialogs/MagicDetectSettingsDialog.h`
- `src/ui/components/dialogs/MagicDetectSettingsDialog.cpp`

#### Data Structure
```cpp
struct MagicDetectSettings {
    // Overall sensitivity
    double overallSensitivity = 0.7;
    
    // Pipeline weights (must sum to 1.0)
    double ocrFirstWeight = 0.4;
    double rectangleDetectionWeight = 0.4;
    double consensusWeight = 0.2;
    
    // Rectangle detection
    double rectangleSensitivity = 0.15;
    int minWidth = 15;
    int minHeight = 10;
    int maxWidth = 800;
    int maxHeight = 300;
    double overfitExpansion = 0.1;  // 10%
    
    // OCR-first
    double overfitHorizontal = 0.4;  // 40%
    double overfitVertical = 0.6;   // 60%
    double hardEdgeSensitivity = 0.5;
    bool cellGroupDetection = true;
    
    // Matching
    double iouThreshold = 0.3;
    double overlapThreshold = 0.4;
    bool consensusRequired = false;  // If false, include OCR-only regions
    
    // Detection types
    bool detectTextInput = true;
    bool detectTextLine = true;
    bool detectTextBlock = true;
    bool detectCheckbox = true;
    bool detectCell = true;
    bool detectSquare = true;
    bool detectRectangle = true;
};
```

#### Integration Points
1. **MainWindow**: Add "Settings..." button next to Magic Detect button
2. **RegionDetector**: Accept `MagicDetectSettings` parameter
3. **RectangleDetector**: Use settings for parameters
4. **FormFieldDetector**: Use settings for overfitting

### Features
- **Live Preview**: "Preview" button runs detection with current settings (non-destructive)
- **Reset**: Restore defaults
- **Presets**: Dropdown for "Sensitive", "Balanced", "Precise"
- **Save/Load**: Save settings to JSON file

---

## 2. Spot Magic Detect

### Purpose
Allow users to draw a region on the canvas and run Magic Detect only on that specific area.

### User Flow
1. User clicks "Spot Magic Detect" button (or keyboard shortcut)
2. Cursor changes to crosshair
3. User draws rectangle on canvas (click-drag-release)
4. Magic Detect runs ONLY on that region
5. Detected regions are shown (only those within/overlapping the drawn region)
6. User can draw another region or exit Spot mode

### UI Design

#### Button Placement
```
[Magic Detect] [Spot Magic Detect] [Settings...]
```

#### Visual Feedback
- **Drawing Mode**: Cursor = crosshair, canvas shows "Draw region for Spot Magic Detect"
- **While Drawing**: Show semi-transparent rectangle with dimensions
- **After Detection**: Highlight detected regions within the drawn area
- **Status Bar**: "Spot Magic Detect: Draw a region on the canvas"

### Implementation

#### New Files
- `src/ui/components/SpotMagicDetectMode.h`
- `src/ui/components/SpotMagicDetectMode.cpp`

#### Integration
1. **MainWindow**: 
   - Add "Spot Magic Detect" button
   - Handle mouse events when in Spot mode
   - Draw selection rectangle
   
2. **RegionDetector**:
   - Add `detectRegionsInArea(const QImage& image, const QRect& area)`
   - Crop image to area, run detection, transform coordinates back

3. **Canvas/View**:
   - Handle mouse press/move/release for region drawing
   - Visual feedback for drawn region

#### Code Structure
```cpp
class SpotMagicDetectMode : public QObject {
    Q_OBJECT
    
public:
    explicit SpotMagicDetectMode(QObject* parent = nullptr);
    void activate();
    void deactivate();
    bool isActive() const { return active_; }
    
    // Mouse event handlers
    void onMousePress(const QPoint& pos);
    void onMouseMove(const QPoint& pos);
    void onMouseRelease(const QPoint& pos);
    
signals:
    void regionDrawn(const QRect& region);
    void detectionComplete(const DetectionResult& result);
    
private:
    bool active_ = false;
    QPoint startPos_;
    QPoint currentPos_;
    QRect drawnRegion_;
};
```

### Features
- **Keyboard Shortcut**: `Cmd+Shift+M` (or `Ctrl+Shift+M`)
- **Cancel**: `Esc` key cancels Spot mode
- **Multiple Regions**: Can draw multiple regions sequentially
- **Undo**: Undo last Spot detection
- **Visual Overlay**: Show drawn region with semi-transparent fill

---

## 3. Enhanced Detection Strategy

### Current Improvements Made
1. **More Sensitive Rectangle Detection**:
   - Lowered sensitivity threshold (0.15)
   - Multiple Canny thresholds (10/30, 20/60, 30/90)
   - More thresholding levels (5 instead of 3)
   - Larger morphological kernels (5x5 instead of 3x3)
   - More iterations (3 instead of 2)
   - RETR_TREE instead of RETR_EXTERNAL (catches nested rectangles)
   - Overfitting: 10% expansion on detected rectangles

2. **More Lenient Matching**:
   - IoU threshold lowered (0.3 instead of 0.5)
   - Overlap threshold lowered (0.4 instead of 0.6)
   - Always include OCR regions (even without matches)
   - Lower rectangle confidence threshold (0.5 instead of 0.7)

3. **Fallback Mechanism**:
   - If no consensus matches, still use OCR-first results
   - OCR-only regions get 0.70 confidence (still good)
   - Consensus regions get 0.85 confidence (boosted)

### Future Enhancements
1. **Adaptive Sensitivity**: Automatically adjust based on document characteristics
2. **Multi-Scale Detection**: Run at multiple resolutions and merge
3. **Confidence-Based Filtering**: User-adjustable minimum confidence threshold
4. **Region Type Filtering**: User can exclude certain types (e.g., "no checkboxes")

---

## Implementation Priority

### Phase 1: Core Detection Improvements ✅
- [x] More sensitive rectangle detection
- [x] More lenient matching
- [x] Fallback mechanism

### Phase 2: Settings Dialog
- [ ] Create MagicDetectSettingsDialog UI
- [ ] Integrate settings into RegionDetector
- [ ] Add settings persistence (save/load JSON)
- [ ] Add presets dropdown

### Phase 3: Spot Magic Detect
- [ ] Create SpotMagicDetectMode class
- [ ] Add mouse event handling in MainWindow/Canvas
- [ ] Implement region cropping and coordinate transformation
- [ ] Add visual feedback (drawing rectangle, highlighting results)

### Phase 4: Polish
- [ ] Keyboard shortcuts
- [ ] Tooltips and help text
- [ ] Status bar messages
- [ ] Undo/redo for Spot detections

---

## Technical Considerations

### Performance
- **Settings Dialog**: Preview should run in background thread
- **Spot Magic Detect**: Cropping reduces image size, should be faster
- **Multiple Regions**: Cache detection results per region

### Coordinate Transformation
When cropping for Spot Magic Detect:
1. Crop image to drawn region
2. Run detection on cropped image
3. Transform detected region coordinates back to full image coordinates:
   ```cpp
   NormalizedCoords transformToFullImage(const NormalizedCoords& croppedCoords, 
                                        const QRect& cropRegion, 
                                        const QSize& fullImageSize) {
       // Add crop region offset to coordinates
       double offsetX = static_cast<double>(cropRegion.x()) / fullImageSize.width();
       double offsetY = static_cast<double>(cropRegion.y()) / fullImageSize.height();
       // ... transform logic
   }
   ```

### Settings Persistence
Save to: `~/.config/ocr-orc/magic_detect_settings.json`
Load on app startup, save on dialog close.

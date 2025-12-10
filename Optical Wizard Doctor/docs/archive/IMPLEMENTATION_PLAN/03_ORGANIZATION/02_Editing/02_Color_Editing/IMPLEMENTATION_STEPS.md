# Color Editing - Implementation Steps

## Objective

Enable users to change region colors for visual organization.

## Prerequisites

- Region selection working
- DocumentState ready
- Color system implemented

## Available Colors

blue, red, green, yellow, purple, orange, cyan

## Steps

### Step 1: Create Change Color Dialog

1. Create method `void showChangeColorDialog(const QString& regionName)`
2. Create QDialog window:
   - Size: 250x150
   - Title: "Change Region Color"
3. Add QLabel: "Select Color:"
4. Add QComboBox:
   - Values: blue, red, green, yellow, purple, orange, cyan
   - Pre-select current region color
5. Add QPushButton: "Save"
6. Connect to save function

### Step 2: Implement Change Color Function

1. Create method in DocumentState: `void changeRegionColor(const QString& name, const QString& color)`
2. Validate:
   - Region exists
   - Color is valid (in color list)
3. Update region's color field:
   ```cpp
   regions[name].color = color;
   ```
4. Save state to undo stack
5. Trigger canvas redraw

### Step 3: Handle Color Change in UI

1. Connect "Change Color" button to change function
2. Get selected region from listbox or canvas
3. Show color dialog
4. On save:
   - Call changeRegionColor()
   - Redraw canvas (region color updates)
   - Update info display

### Step 4: Support Multiple Change Methods

1. **Right-click → Change Color**:
   - Show context menu
   - "Change Color" option
   - Show color dialog
2. **Select in listbox → Change Color button**:
   - Get selection from listbox
   - Show color dialog

### Step 5: Update Color Mapping

1. Ensure color map is consistent:
   ```cpp
   QMap<QString, QColor> colorMap = {
       {"blue", QColor(0, 102, 255)},
       {"red", QColor(255, 0, 0)},
       // ... etc
   };
   ```
2. Use color map for rendering
3. Validate color names against map

### Step 6: Visual Feedback

1. After color change, immediately redraw region
2. Update region border color
3. Update region label color (if applicable)
4. Ensure color change is visible

### Step 7: Handle Invalid Colors

1. If invalid color provided:
   - Default to blue
   - Log warning
   - Continue operation

### Step 8: Test Color Editing

1. Test change color via right-click
2. Test change color via listbox button
3. Test all color options
4. Test color updates on canvas
5. Test undo/redo for color change
6. Test with multiple regions (if supported)

## Acceptance Criteria

- [ ] Users can change region colors
- [ ] Color dialog shows current color
- [ ] All color options work
- [ ] Color updates immediately on canvas
- [ ] Undo/redo works for color change
- [ ] Invalid colors are handled

## Code Structure

```cpp
class MainWindow {
    void showChangeColorDialog(const QString& regionName);
    void changeRegionColor(const QString& name, const QString& color);
};

class DocumentState {
    void changeRegionColor(const QString& name, const QString& color);
};
```

## Notes

- Colors are stored as strings ("blue", "red", etc.)
- Convert to QColor when rendering
- Keep color list consistent
- Provide visual feedback immediately


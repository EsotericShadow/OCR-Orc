# Basic Layout - Implementation Steps

## Objective

Create the basic UI layout with toolbar, canvas area, and side panel as specified in the UI specification.

## Prerequisites

- MainWindow class created
- Qt6 Widgets working
- Basic window displays

## Layout Structure

```
MainWindow
├── Toolbar (top, full width)
├── Control Panel (below toolbar, full width)
└── Main Frame (below control panel, fills remaining)
    ├── Canvas Area (left, expandable)
    └── Side Panel (right, fixed width ~300px)
```

## Steps

### Step 1: Create Toolbar Frame

1. In MainWindow, create `QFrame* toolbarFrame`
2. Set background color: `#e0e0e0` (light grey)
3. Set relief: `QFrame::Raised`
4. Add to top of main layout
5. Set fixed height: ~40px

### Step 2: Create Control Panel Frame

1. Create `QFrame* controlPanelFrame`
2. Set layout: `QHBoxLayout`
3. Add to main layout below toolbar
4. Set fixed height: ~60px
5. Add padding: 10px horizontal, 5px vertical

### Step 3: Create Main Frame (Canvas + Side Panel)

1. Create `QFrame* mainFrame`
2. Set layout: `QHBoxLayout`
3. Add to main layout (below control panel)
4. Set to expand and fill remaining space
5. This will contain canvas and side panel side-by-side

### Step 4: Create Canvas Container

1. Create `QWidget* canvasContainer`
2. Set background: `#d0d0d0` (light grey)
3. Add to left side of main frame
4. Set to expand and fill available space
5. Add vertical and horizontal layouts for status label and canvas

### Step 5: Add Status Label

1. Create `QLabel* statusLabel` in canvas container
2. Set text: "Click and drag to create regions"
3. Set font: Arial, 10pt
4. Set alignment: left
5. Add to top of canvas container layout

### Step 6: Create Side Panel Frame

1. Create `QFrame* sidePanelFrame`
2. Set fixed width: 300px
3. Set layout: `QVBoxLayout`
4. Add to right side of main frame
5. Set background: default (white/light)

### Step 7: Create Tabbed Interface for Side Panel

1. Create `QTabWidget* sidePanelTabs`
2. Add two tabs: "Regions" and "Groups"
3. Add to side panel layout
4. Set to expand and fill space

### Step 8: Create Regions Tab Content

1. Create `QWidget* regionsTab`
2. Add layout: `QVBoxLayout`
3. Add label: "Defined Regions" (bold, 12pt)
4. Create `QListWidget* regionListBox` (scrollable)
5. Add button frame with: Edit Name, Change Color, Delete buttons
6. Add to Regions tab

### Step 9: Create Groups Tab Content

1. Create `QWidget* groupsTab`
2. Add layout: `QVBoxLayout`
3. Add label: "Region Groups" (bold, 12pt)
4. Create `QListWidget* groupListBox` (scrollable)
5. Add button frame with: Create Group, Add to Group, Remove from Group, Delete Group buttons
6. Add to Groups tab

### Step 10: Create Info Display

1. Create `QTextEdit* infoText` below tabs
2. Set read-only: true
3. Set word wrap: true
4. Set font: Arial, default size
5. Add to side panel layout

### Step 11: Add Help Button

1. Create `QPushButton* helpButton`
2. Set text: "?"
3. Set font: Arial, 12pt, bold
4. Set size: 30x30px
5. Add to bottom of side panel
6. Connect to help dialog (placeholder for now)

### Step 12: Set Up Layout Proportions

1. Canvas container: expandable, minimum width 400px
2. Side panel: fixed width 300px
3. Use `setStretchFactor()` to control proportions
4. Test resizing behavior

### Step 13: Add Spacing and Padding

1. Set spacing between widgets: 5-10px
2. Add margins to frames: 10px
3. Add padding to layouts: 5px
4. Ensure consistent spacing throughout

### Step 14: Test Layout

1. Build and run application
2. Verify all components are visible
3. Test window resizing
4. Verify minimum sizes are enforced
5. Check layout on different screen sizes

### Step 15: Add Placeholder Content

1. Add placeholder text to region list: "No regions defined"
2. Add placeholder text to group list: "No groups defined"
3. Add placeholder text to info display: "Select a region or group to view details"
4. This helps users understand the interface

## Acceptance Criteria

- [ ] Toolbar is visible at top
- [ ] Control panel is below toolbar
- [ ] Canvas area is on left, expandable
- [ ] Side panel is on right, fixed width
- [ ] Tabs work correctly (Regions/Groups)
- [ ] Lists are scrollable
- [ ] Layout resizes properly
- [ ] All components are visible and accessible
- [ ] Spacing is consistent

## Code Structure

```cpp
class MainWindow : public QMainWindow {
    // Layout components
    QFrame* toolbarFrame;
    QFrame* controlPanelFrame;
    QFrame* mainFrame;
    QWidget* canvasContainer;
    QFrame* sidePanelFrame;
    
    // Side panel components
    QTabWidget* sidePanelTabs;
    QListWidget* regionListBox;
    QListWidget* groupListBox;
    QTextEdit* infoText;
    QPushButton* helpButton;
    
    // Status
    QLabel* statusLabel;
    
    void setupLayout();
    void setupToolbar();
    void setupControlPanel();
    void setupMainFrame();
    void setupSidePanel();
};
```

## Notes

- Use Qt layouts (QVBoxLayout, QHBoxLayout) for flexibility
- Set minimum sizes to prevent UI from becoming unusable
- Test on different screen resolutions
- Follow Qt best practices for layout management
- Keep layout code organized and maintainable


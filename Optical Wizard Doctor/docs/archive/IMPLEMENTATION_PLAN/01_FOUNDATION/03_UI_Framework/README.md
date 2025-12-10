# UI Framework - Implementation Steps

## Objective

Set up the basic UI framework: main window, layout structure, and initial UI components.

## Prerequisites

- Qt6 installed and working
- Basic CMake configuration complete
- Main application compiles

## Sub-Projects

1. **Window Setup** - Create main application window
2. **Basic Layout** - Set up toolbar, canvas area, and side panel

## Order of Implementation

Complete these in order:
1. Window Setup
2. Basic Layout

## Steps

### Step 1: Create MainWindow Class

1. Create `src/ui/MainWindow.h` and `MainWindow.cpp`
2. Inherit from `QMainWindow`
3. Add Q_OBJECT macro for signals/slots
4. Set basic window properties (title, size, minimum size)

### Step 2: Implement Window Setup

1. Create constructor that initializes window
2. Set window title: "OCR-Orc"
3. Set default size: 1200x800
4. Set minimum size: 800x600
5. Enable window resizing

### Step 3: Create Layout Structure

1. Create central widget
2. Create main vertical layout
3. Add placeholder frames for:
   - Toolbar (top)
   - Control panel (below toolbar)
   - Main content area (canvas + side panel)

### Step 4: Set Up Status Bar

1. Create status bar
2. Add status label
3. Show initial message: "Ready"

### Step 5: Test Window Display

1. Create MainWindow instance in main.cpp
2. Show window
3. Verify it displays correctly
4. Test resizing behavior

## Acceptance Criteria

- [ ] MainWindow class created
- [ ] Window displays with correct size
- [ ] Window is resizable
- [ ] Minimum size is enforced
- [ ] Basic layout structure is visible

## Next Steps

After completing Window Setup, proceed to Basic Layout to add the detailed UI components.

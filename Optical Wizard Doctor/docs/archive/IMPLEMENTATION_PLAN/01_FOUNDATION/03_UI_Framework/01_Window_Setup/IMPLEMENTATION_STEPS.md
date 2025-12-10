# Window Setup - Implementation Steps

## Objective

Create the main application window with proper initialization and basic structure.

## Prerequisites

- Qt6 installed and working
- Basic CMake configuration complete
- Main application compiles

## Steps

### Step 1: Create MainWindow Class

1. Create `src/ui/MainWindow.h`
2. Inherit from `QMainWindow`
3. Add basic window properties:
   - Window title: "OCR-Orc"
   - Default size: 1200x800
   - Minimum size: 800x600
   - Resizable: Yes

### Step 2: Implement Constructor

1. Set window properties in constructor
2. Initialize member variables
3. Call `setupUI()` method
4. Connect signals/slots (if any)

### Step 3: Create Basic Layout Structure

1. Create central widget
2. Create main layout (vertical)
3. Add placeholder widgets:
   - Toolbar area (top)
   - Canvas area (center, expandable)
   - Side panel area (right, fixed width)

### Step 4: Set Up Menu Bar (Optional)

1. Create File menu
2. Add "Load PDF" action (placeholder)
3. Add "Exit" action
4. Connect to slots

### Step 5: Set Up Status Bar

1. Create status bar
2. Add status label
3. Show initial message: "Ready"

### Step 6: Handle Window Events

1. Override `closeEvent()` to handle window closing
2. Add confirmation dialog if there's unsaved work (later)
3. Save window geometry/state (optional)

### Step 7: Test Window Display

1. Create instance in `main.cpp`
2. Show window
3. Verify it displays correctly
4. Test resizing
5. Test closing

### Step 8: Add Basic Styling

1. Set window background color
2. Apply basic stylesheet (if needed)
3. Test on different platforms

### Step 9: Add Window Icon

1. Create or find application icon
2. Set window icon: `setWindowIcon(QIcon(":/icons/app.png"))`
3. Test icon displays

### Step 10: Document Window Setup

1. Add code comments
2. Document window properties
3. Note any platform-specific considerations

## Acceptance Criteria

- [ ] Window displays correctly
- [ ] Window is resizable
- [ ] Minimum size is enforced
- [ ] Window closes properly
- [ ] Basic layout structure is visible
- [ ] No crashes or errors

## Code Structure

```cpp
class MainWindow : public QMainWindow {
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    
private:
    void setupUI();
    void setupMenuBar();
    void setupStatusBar();
    
    // Member variables
    QWidget* centralWidget;
    QVBoxLayout* mainLayout;
    // ... other widgets
};
```

## Notes

- Keep it simple initially
- Add complexity incrementally
- Test on all target platforms
- Follow Qt best practices


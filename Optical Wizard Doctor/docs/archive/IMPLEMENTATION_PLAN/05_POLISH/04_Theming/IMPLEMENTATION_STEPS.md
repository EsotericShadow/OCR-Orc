# Theming - Implementation Steps

## Objective

Implement light and dark mode themes with system integration.

## Prerequisites

- UI framework complete
- Qt stylesheet support
- Color system defined

## Theme Requirements

1. **Light Mode** - Default light theme
2. **Dark Mode** - Dark theme for low-light
3. **System Integration** - Detect system theme
4. **Manual Toggle** - User can override

## Steps

### Step 1: Define Color Schemes

1. Create theme color definitions:
   ```cpp
   struct ThemeColors {
       QColor background;
       QColor surface;
       QColor text;
       QColor primary;
       QColor selection;
   };
   
   ThemeColors lightTheme = {
       QColor(255, 255, 255),  // background
       QColor(255, 255, 255),  // surface
       QColor(0, 0, 0),        // text
       QColor(0, 102, 255),    // primary
       QColor(0, 102, 255)     // selection
   };
   
   ThemeColors darkTheme = {
       QColor(30, 30, 30),     // background
       QColor(45, 45, 45),     // surface
       QColor(201, 209, 217),  // text
       QColor(88, 166, 255),   // primary
       QColor(88, 166, 255)    // selection
   };
   ```

### Step 2: Create Theme Manager

1. Create `src/ui/ThemeManager.h` and `.cpp`
2. Create class `ThemeManager`
3. Add methods:
   - `void setTheme(Theme theme)` - Set theme
   - `Theme getTheme() const` - Get current theme
   - `void applyTheme(QWidget* widget)` - Apply to widget

### Step 3: Implement Light Theme

1. Define light theme colors
2. Create stylesheet:
   ```css
   QMainWindow {
       background-color: #ffffff;
       color: #000000;
   }
   QToolBar {
       background-color: #e0e0e0;
   }
   QCanvas {
       background-color: #d0d0d0;
   }
   ```
3. Apply to application

### Step 4: Implement Dark Theme

1. Define dark theme colors
2. Create stylesheet:
   ```css
   QMainWindow {
       background-color: #1e1e1e;
       color: #c9d1d9;
   }
   QToolBar {
       background-color: #2d2d2d;
   }
   QCanvas {
       background-color: #1e1e1e;
   }
   ```
3. Ensure sufficient contrast (WCAG AA)

### Step 5: Detect System Theme

1. Check system theme preference:
   ```cpp
   bool isDarkMode() {
       // macOS
       #ifdef Q_OS_MAC
           // Check NSAppearance
       #endif
       // Windows
       #ifdef Q_OS_WIN
           // Check registry
       #endif
       // Linux
       #ifdef Q_OS_LINUX
           // Check desktop environment
       #endif
   }
   ```
2. Or use Qt's system palette detection
3. Set initial theme based on system

### Step 6: Add Theme Toggle

1. Add theme toggle in settings or menu
2. Or add theme button in toolbar
3. Toggle between light and dark
4. Save preference to settings

### Step 7: Apply Theme to All Components

1. Apply to:
   - MainWindow
   - Toolbar
   - Canvas
   - Side Panel
   - Dialogs
   - Buttons
   - Lists
2. Use Qt stylesheets or palette
3. Ensure consistency across all UI

### Step 8: Handle Theme Transitions

1. Smooth transition (optional animation)
2. Update all components simultaneously
3. Redraw canvas with new colors
4. Update region colors if needed

### Step 9: Test Theming

1. Test light theme
2. Test dark theme
3. Test theme toggle
4. Test system theme detection
5. Verify contrast ratios (WCAG AA)
6. Test on all platforms

## Acceptance Criteria

- [ ] Light theme works correctly
- [ ] Dark theme works correctly
- [ ] System theme is detected
- [ ] Theme can be manually toggled
- [ ] All UI components use theme colors
- [ ] Contrast ratios meet WCAG AA
- [ ] Theme preference is saved

## Code Structure

```cpp
enum class Theme {
    Light,
    Dark,
    System
};

class ThemeManager {
public:
    void setTheme(Theme theme);
    Theme getTheme() const;
    void applyTheme(QApplication* app);
    static bool isSystemDarkMode();
    
private:
    Theme currentTheme;
    QString getStylesheet(Theme theme) const;
};
```

## Notes

- Ensure sufficient contrast for accessibility
- Test on all platforms
- Save user preference
- Support system theme detection


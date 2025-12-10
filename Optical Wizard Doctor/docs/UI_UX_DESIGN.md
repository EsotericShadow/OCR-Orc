# UI/UX Design Documentation

## Overview

The OCR-Orc (OCR-Orc) application features a modern, intuitive user interface designed for professional OCR coordinate calibration workflows. The design emphasizes clarity, consistency, and visual feedback.

## Design Principles

### 1. Visual Hierarchy
- **Primary Actions**: Blue buttons (#2196F3) - File operations, navigation
- **Success Actions**: Green buttons (#4CAF50) - Grouping, exporting
- **Warning Actions**: Orange buttons (#FF9800) - Ungrouping, importing
- **Destructive Actions**: Red buttons (#f44336) - Delete operations
- **Secondary Actions**: Gray buttons (#757575) - Clear, reset operations
- **Special Actions**: Purple buttons (#9C27B0) - Mask export, special features

### 2. Icon System

The application uses a centralized `IconManager` that provides:
- **Qt Standard Icons**: Uses native Qt icons where available for platform consistency
- **Text Fallbacks**: Unicode symbols as fallbacks when standard icons aren't available
- **Theme Awareness**: Icons adapt to light/dark themes

#### Icon Mapping

| Action | Icon | Color |
|--------|------|-------|
| Open/Load | Folder icon | Blue |
| Save/Export | Save icon | Green |
| Import | Import icon | Orange |
| Undo | Back arrow | Blue |
| Redo | Forward arrow | Blue |
| Zoom In | Plus symbol | Gray |
| Zoom Out | Minus symbol | Gray |
| Group | Folder icon | Green |
| Ungroup | Open folder | Orange |
| Delete | Trash icon | Red |
| Edit | Info icon | Teal |
| Help | Question mark | Blue |
| Clear | Reset icon | Gray |

### 3. Button Styling

All buttons follow a consistent design pattern:

```css
QPushButton {
    border: 1px solid [color];
    border-radius: 4px;
    background-color: [color];
    color: white;
    padding: 4px-6px 8px-16px;
    font-weight: bold;
}

QPushButton:hover {
    background-color: [darker color];
    border-color: [darker color];
}

QPushButton:pressed {
    background-color: [darkest color];
}

QPushButton:disabled {
    background-color: #cccccc;
    border-color: #aaaaaa;
    color: #666666;
}
```

### 4. Layout Structure

#### Toolbar (Top)
- **Height**: 50px
- **Background**: Light gray (#e0e0e0)
- **Sections**: Mode | Zoom | Groups | Undo/Redo
- **Spacing**: 10px between elements, 20px between sections

#### Control Panel (Top)
- **Height**: 80px
- **Background**: Light gray (#f0f0f0)
- **Sections**: File Operations | Region Input | Actions
- **Spacing**: 10px between elements, 20px between sections

#### Side Panel (Right)
- **Width**: 250px
- **Background**: Light gray (#f5f5f5)
- **Tabs**: Regions | Groups
- **Info Display**: 100px height at bottom
- **Help Button**: Circular, 36x36px

#### Canvas (Center)
- **Background**: White/Theme-appropriate
- **Status Label**: Top-left, transparent background

### 5. Color Palette

#### Primary Colors
- **Blue** (#2196F3): Primary actions, navigation
  - Hover: #1976D2
  - Pressed: #1565C0

- **Green** (#4CAF50): Success actions, grouping
  - Hover: #45a049
  - Pressed: #3d8b40

- **Orange** (#FF9800): Warning actions, ungrouping
  - Hover: #F57C00
  - Pressed: #EF6C00

- **Red** (#f44336): Destructive actions, deletion
  - Hover: #d32f2f
  - Pressed: #c62828

#### Neutral Colors
- **Gray** (#757575): Secondary actions
  - Hover: #616161
  - Pressed: #424242

- **Teal** (#009688): Edit actions
  - Hover: #00796B
  - Pressed: #00695C

- **Purple** (#9C27B0): Special features
  - Hover: #7B1FA2
  - Pressed: #6A1B9A

#### Background Colors
- **Toolbar**: #e0e0e0
- **Control Panel**: #f0f0f0
- **Side Panel**: #f5f5f5
- **Canvas**: White (light theme) / Dark gray (dark theme)

### 6. Typography

- **Labels**: Bold, 12pt
- **Buttons**: Bold, default size
- **Input Fields**: Regular, default size
- **Status Text**: Regular, 10pt

### 7. Spacing and Padding

- **Button Padding**: 4-6px vertical, 8-16px horizontal
- **Section Spacing**: 20px
- **Element Spacing**: 10px
- **Layout Margins**: 5-10px

### 8. Interactive Feedback

#### Hover States
- All buttons darken on hover (10-15% darker)
- Border color matches background on hover
- Smooth transitions (handled by Qt)

#### Pressed States
- Buttons darken further on press (20-25% darker)
- Provides tactile feedback

#### Disabled States
- Gray background (#cccccc)
- Gray border (#aaaaaa)
- Gray text (#666666)
- No hover effects

### 9. Icon Sizes

- **Toolbar Icons**: 16-20px
- **Control Panel Icons**: 18px
- **Side Panel Icons**: 16px
- **Help Button Icon**: 24px

### 10. Accessibility

- **Tooltips**: All buttons have descriptive tooltips
- **Keyboard Shortcuts**: Displayed in tooltips
- **Color Contrast**: WCAG AA compliant
- **Icon + Text**: Buttons show both icon and text for clarity

## Component-Specific Design

### Toolbar Widget

**Mode Selection**
- Radio buttons for Create/Select modes
- Clear labels with tooltips
- Keyboard shortcuts (1/2)

**Zoom Controls**
- Icon buttons for zoom in/out (32x32px)
- Text label showing percentage
- Reset button with icon

**Group Controls**
- Color-coded buttons (Green/Orange)
- Icons for visual clarity
- Compact labels

**Undo/Redo**
- Blue buttons with arrow icons
- Disabled state when unavailable
- Clear visual feedback

### Control Panel Widget

**File Operations**
- Primary blue for Load PDF
- Green for Export (success action)
- Orange for Import (warning action)
- Purple for Mask Export (special feature)

**Region Input**
- Standard input fields
- Color combo box with visual swatches
- Clear labels

**Action Buttons**
- Teal for Edit (modification)
- Gray for Clear (neutral action)

### Side Panel Widget

**Group Management**
- Green for Create/Add (positive actions)
- Orange for Remove (warning action)
- Red for Delete (destructive action)
- Compact button layout

**Help Button**
- Circular design
- Large, prominent icon
- Blue accent color

## Theme Integration

The UI design adapts to light and dark themes:

- **Light Theme**: Light backgrounds, dark text, standard colors
- **Dark Theme**: Dark backgrounds, light text, adjusted button colors
- **System Theme**: Follows OS preference

Button colors remain consistent across themes, but backgrounds and text colors adapt.

## Future Enhancements

1. **Custom Icon Set**: Replace text fallbacks with custom SVG icons
2. **Animation**: Add smooth transitions for state changes
3. **High DPI Support**: Ensure icons scale properly on Retina displays
4. **Icon Themes**: Support multiple icon themes (outlined, filled, etc.)
5. **Accessibility Icons**: Larger icons for accessibility mode

## Design Resources

- **Icon Library**: Qt Standard Icons (QStyle::StandardPixmap)
- **Color Reference**: Material Design Color Palette
- **Spacing System**: 4px base unit (4, 8, 12, 16, 20px)
- **Border Radius**: 4px for buttons, 18px for circular buttons

## Implementation Notes

- Icons are managed centrally via `IconManager`
- Styles are applied via Qt Stylesheets
- Theme colors are managed by `ThemeManager`
- All styling is theme-aware and responsive


#ifndef MAIN_WINDOW_KEYBOARD_SHORTCUTS_H
#define MAIN_WINDOW_KEYBOARD_SHORTCUTS_H

namespace ocr_orc {

// Forward declaration
class MainWindow;

/**
 * @brief Handles keyboard shortcuts setup for MainWindow
 * 
 * Manages all keyboard shortcuts for the application.
 * This module organizes the setup code but requires MainWindow*
 * because keyboard shortcuts are inherently tied to the main window.
 */
class MainWindowKeyboardShortcuts {
public:
    MainWindowKeyboardShortcuts();
    ~MainWindowKeyboardShortcuts();
    
    /**
     * @brief Setup all keyboard shortcuts
     * @param mainWindow Main window instance (for signal/slot connections and action registration)
     * 
     * Sets up 17 keyboard shortcuts:
     * - File operations (4): Load PDF, Export, Import, Export Mask
     * - Mode switching (2): Create mode, Select mode
     * - Zoom operations (3): Zoom in, Zoom out, Reset zoom
     * - Region operations (3): Delete, Backspace, Edit
     * - Selection operations (2): Select all, Invert selection
     * - Group operations (2): Create group, Remove from group
     * - Undo/Redo (1): Redo alternative (Cmd+Y)
     * - Help (1): Show help (F1)
     */
    void setupShortcuts(MainWindow* mainWindow);
};

} // namespace ocr_orc

#endif // MAIN_WINDOW_KEYBOARD_SHORTCUTS_H


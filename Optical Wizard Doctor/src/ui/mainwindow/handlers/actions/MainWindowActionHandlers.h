#ifndef MAIN_WINDOW_ACTION_HANDLERS_H
#define MAIN_WINDOW_ACTION_HANDLERS_H

#include <QtWidgets/QStatusBar>

namespace ocr_orc {

// Forward declarations
class MainWindow;
class DocumentState;
class Canvas;
class ControlPanelWidget;

/**
 * @brief Handles action handlers for MainWindow (File/Export operations)
 * 
 * Manages:
 * - PDF loading
 * - Coordinate export/import
 * - Mask export
 */
class MainWindowActionHandlers {
public:
    MainWindowActionHandlers();
    ~MainWindowActionHandlers();
    
    /**
     * @brief Handle PDF load action
     * Creates all necessary lambdas internally by accessing MainWindow members
     */
    void onLoadPdf(MainWindow* mainWindow);
    
    /**
     * @brief Handle coordinate export action
     */
    void onExportCoordinates(MainWindow* mainWindow);
    
    /**
     * @brief Handle coordinate load action
     */
    void onLoadCoordinates(MainWindow* mainWindow);
    
    /**
     * @brief Handle mask export action
     */
    void onExportMask(MainWindow* mainWindow);
};

} // namespace ocr_orc

#endif // MAIN_WINDOW_ACTION_HANDLERS_H


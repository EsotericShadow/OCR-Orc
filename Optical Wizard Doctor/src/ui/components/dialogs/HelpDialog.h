#ifndef HELP_DIALOG_H
#define HELP_DIALOG_H

#include <QtWidgets/QWidget>

namespace ocr_orc {

/**
 * @brief Handles help dialog for MainWindow
 * 
 * Manages the help dialog with comprehensive documentation.
 */
class MainWindowHelpDialog {
public:
    MainWindowHelpDialog();
    ~MainWindowHelpDialog();
    
    /**
     * @brief Show the help dialog
     * @param parentWidget Parent widget for the dialog
     * 
     * Displays a comprehensive help window with:
     * - Basic workflow instructions
     * - Selection modes explanation
     * - Keyboard shortcuts reference
     * - Export formats documentation
     * - Coordinate systems explanation
     * - Tips and tricks
     */
    void showHelp(QWidget* parentWidget);
};

} // namespace ocr_orc

#endif // HELP_DIALOG_H


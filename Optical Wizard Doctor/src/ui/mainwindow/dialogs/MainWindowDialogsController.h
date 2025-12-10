#ifndef MAIN_WINDOW_DIALOGS_CONTROLLER_H
#define MAIN_WINDOW_DIALOGS_CONTROLLER_H

#include <QtCore/QString>
#include <QtWidgets/QWidget>
#include <functional>

namespace ocr_orc {

// Forward declarations
class DocumentState;
class Canvas;
class ExportDialog;
struct RegionData;

/**
 * @brief Centralizes dialog launches for MainWindow
 * 
 * Manages all dialog creation and display:
 * - Help dialog
 * - Preferences dialog
 * - Export dialog
 * - Name edit dialog
 * - Color picker dialog
 * 
 * Dialogs stay dumb views; controller handles orchestration.
 * Dependencies (DocumentState/Canvas) passed explicitly.
 */
class MainWindowDialogsController {
public:
    // Callback types
    using GetRegionCallback = std::function<RegionData(const QString&)>;
    
    MainWindowDialogsController();
    ~MainWindowDialogsController();
    
    /**
     * @brief Show help dialog
     * @param parent Parent widget
     */
    void showHelp(::QWidget* parent);
    
    /**
     * @brief Show preferences dialog
     * @param parent Parent widget
     */
    void showPreferences(::QWidget* parent);
    
    /**
     * @brief Show export dialog
     * @param parent Parent widget
     * @return ExportDialog instance (caller handles exec() and export logic)
     */
    ExportDialog* createExportDialog(::QWidget* parent);
    
    /**
     * @brief Show name edit dialog
     * @param parent Parent widget
     * @param currentName Current region name to edit
     * @return New name if user accepted, empty string if cancelled
     */
    QString showEditNameDialog(::QWidget* parent, const QString& currentName);
    
    /**
     * @brief Show color picker dialog
     * @param parent Parent widget
     * @param documentState Document state
     * @param regionName Region to change color for
     * @param getRegion Callback to get region data
     * @return New color name if user accepted, empty string if cancelled
     */
    QString showChangeColorDialog(::QWidget* parent,
                                 DocumentState* documentState,
                                 const QString& regionName,
                                 const GetRegionCallback& getRegion);
};

} // namespace ocr_orc

#endif // MAIN_WINDOW_DIALOGS_CONTROLLER_H


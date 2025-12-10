#ifndef MAIN_WINDOW_FILE_OPERATIONS_H
#define MAIN_WINDOW_FILE_OPERATIONS_H

#include <QtCore/QString>
#include <QtWidgets/QWidget>
#include <QtWidgets/QStatusBar>  // Full include for QStatusBar
#include <functional>

namespace ocr_orc {

// Forward declarations
class DocumentState;
class Canvas;
class ControlPanelWidget;

/**
 * @brief Handles all file operations for MainWindow
 * 
 * Manages:
 * - PDF loading
 * - Coordinate export/import (JSON, CSV)
 * - Mask image export
 */
class MainWindowFileOperations {
public:
    // Callback types for UI updates
    using UpdateZoomLabelCallback = std::function<void()>;
    using UpdateUndoRedoButtonsCallback = std::function<void()>;
    using UpdateRegionListBoxCallback = std::function<void()>;
    using UpdateGroupListBoxCallback = std::function<void()>;
    using ShowStatusMessageCallback = std::function<void(const QString&, int)>;
    using ShowWarningCallback = std::function<void(const QString&, const QString&)>;
    using ShowInformationCallback = std::function<void(const QString&, const QString&)>;
    using ShowCriticalCallback = std::function<void(const QString&, const QString&)>;
    using SetFileLabelCallback = std::function<void(const QString&)>;
    using ZoomResetCallback = std::function<void()>;
    
    MainWindowFileOperations();
    ~MainWindowFileOperations();
    
    /**
     * @brief Load a PDF file
     * @param parentWidget Parent widget for dialogs
     * @param documentState Document state to update
     * @param canvas Canvas widget to load PDF into
     * @param controlPanelWidget Control panel to update file label
     * @param statusBar Status bar to show messages
     * @param callbacks Callback functions for UI updates
     */
    void loadPdf(QWidget* parentWidget,
                 DocumentState* documentState,
                 Canvas* canvas,
                 ControlPanelWidget* controlPanelWidget,
                 QStatusBar* statusBar,
                 const UpdateZoomLabelCallback& updateZoomLabel,
                 const UpdateUndoRedoButtonsCallback& updateUndoRedoButtons,
                 const UpdateRegionListBoxCallback& updateRegionListBox,
                 const UpdateGroupListBoxCallback& updateGroupListBox,
                 const SetFileLabelCallback& setFileLabel);
    
    /**
     * @brief Export coordinates to file (JSON or CSV)
     * @param parentWidget Parent widget for dialogs
     * @param documentState Document state containing regions
     * @param statusBar Status bar to show messages
     * @param callbacks Callback functions for UI updates
     */
    void exportCoordinates(QWidget* parentWidget,
                           DocumentState* documentState,
                           QStatusBar* statusBar,
                           const ShowInformationCallback& showInformation,
                           const ShowWarningCallback& showWarning,
                           const ShowCriticalCallback& showCritical);
    
    /**
     * @brief Import coordinates from file (JSON)
     * @param parentWidget Parent widget for dialogs
     * @param documentState Document state to update
     * @param canvas Canvas widget to update
     * @param statusBar Status bar to show messages
     * @param callbacks Callback functions for UI updates
     */
    void loadCoordinates(QWidget* parentWidget,
                        DocumentState* documentState,
                        Canvas* canvas,
                        QStatusBar* statusBar,
                        const UpdateRegionListBoxCallback& updateRegionListBox,
                        const UpdateGroupListBoxCallback& updateGroupListBox,
                        const UpdateUndoRedoButtonsCallback& updateUndoRedoButtons,
                        const ZoomResetCallback& zoomReset,
                        const ShowInformationCallback& showInformation,
                        const ShowCriticalCallback& showCritical);
    
    /**
     * @brief Export mask image
     * @param parentWidget Parent widget for dialogs
     * @param documentState Document state containing regions and image
     * @param statusBar Status bar to show messages
     * @param callbacks Callback functions for UI updates
     */
    void exportMask(QWidget* parentWidget,
                   DocumentState* documentState,
                   QStatusBar* statusBar,
                   const ShowInformationCallback& showInformation,
                   const ShowWarningCallback& showWarning,
                   const ShowCriticalCallback& showCritical);
    
    /**
     * @brief Handle PDF drop event (from drag and drop)
     * @param filePath Path to dropped PDF file
     * @param documentState Document state to update
     * @param canvas Canvas widget to load PDF into
     * @param controlPanelWidget Control panel to update file label
     * @param statusBar Status bar to show messages
     * @param callbacks Callback functions for UI updates
     * @return true if PDF loaded successfully, false otherwise
     */
    bool handlePdfDrop(const QString& filePath,
                       DocumentState* documentState,
                       Canvas* canvas,
                       ControlPanelWidget* controlPanelWidget,
                       QStatusBar* statusBar,
                       const UpdateZoomLabelCallback& updateZoomLabel,
                       const UpdateUndoRedoButtonsCallback& updateUndoRedoButtons,
                       const UpdateRegionListBoxCallback& updateRegionListBox,
                       const UpdateGroupListBoxCallback& updateGroupListBox,
                       const SetFileLabelCallback& setFileLabel,
                       const ShowWarningCallback& showWarning);
};

} // namespace ocr_orc

#endif // MAIN_WINDOW_FILE_OPERATIONS_H


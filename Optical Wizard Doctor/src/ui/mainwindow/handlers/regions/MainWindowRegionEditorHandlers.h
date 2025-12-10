#ifndef MAIN_WINDOW_REGION_EDITOR_HANDLERS_H
#define MAIN_WINDOW_REGION_EDITOR_HANDLERS_H

#include <QtCore/QString>
#include <QtGui/QImage>
#include <QtCore/QPointF>
#include <functional>

namespace ocr_orc {

// Forward declarations
class MainWindow;
class DocumentState;
class Canvas;

/**
 * @brief Handles region editor property changes from SidePanelWidget
 * 
 * Groups handlers by concern:
 * - Name/color/group handlers
 * - Type/fill handlers
 * - Coordinates handlers
 * 
 * Ensures saveState-before-mutate timing for undo/redo.
 */
class MainWindowRegionEditorHandlers {
public:
    // Callback types
    using SaveStateCallback = std::function<void()>;
    using InvalidateCacheCallback = std::function<void()>;
    using UpdateCanvasCallback = std::function<void()>;
    using UpdateUndoRedoButtonsCallback = std::function<void()>;
    using UpdateRegionListBoxCallback = std::function<void()>;
    using GetDocumentImageCallback = std::function<QImage()>;
    using GetScaleFactorCallback = std::function<double()>;
    using GetImageOffsetCallback = std::function<QPointF()>;
    using SynchronizeCoordinatesCallback = std::function<void()>;
    
    MainWindowRegionEditorHandlers();
    ~MainWindowRegionEditorHandlers();
    
    /**
     * @brief Handle region name change from editor
     * @param mainWindow Main window instance
     * @param oldName Original region name
     * @param newName New region name
     */
    void onRegionNameChanged(MainWindow* mainWindow, const QString& oldName, const QString& newName);
    
    /**
     * @brief Handle region color change from editor
     * @param mainWindow Main window instance
     * @param regionName Name of the region
     * @param newColor New color value
     */
    void onRegionColorChanged(MainWindow* mainWindow, const QString& regionName, const QString& newColor);
    
    /**
     * @brief Handle region group change from editor
     * @param mainWindow Main window instance
     * @param regionName Name of the region
     * @param newGroup New group name (empty string to remove from group)
     */
    void onRegionGroupChanged(MainWindow* mainWindow, const QString& regionName, const QString& newGroup);
    
    /**
     * @brief Handle region type change from editor
     * @param mainWindow Main window instance
     * @param regionName Name of the region
     * @param newType New region type
     */
    void onRegionTypeChanged(MainWindow* mainWindow,
                            DocumentState* documentState,
                            Canvas* canvas,
                            const QString& regionName,
                            const QString& newType,
                            const SaveStateCallback& saveState,
                            const InvalidateCacheCallback& invalidateCache,
                            const UpdateCanvasCallback& updateCanvas,
                            const UpdateUndoRedoButtonsCallback& updateUndoRedoButtons);
    
    /**
     * @brief Handle region percentage fill change from editor
     * @param mainWindow Main window instance
     * @param regionName Name of the region
     * @param newPercentageFill New percentage fill value
     */
    void onRegionPercentageFillChanged(MainWindow* mainWindow,
                                      DocumentState* documentState,
                                      Canvas* canvas,
                                      const QString& regionName,
                                      const QString& newPercentageFill,
                                      const SaveStateCallback& saveState,
                                      const InvalidateCacheCallback& invalidateCache,
                                      const UpdateCanvasCallback& updateCanvas,
                                      const UpdateUndoRedoButtonsCallback& updateUndoRedoButtons);
    
    /**
     * @brief Handle region coordinates change from editor
     * @param mainWindow Main window instance
     * @param regionName Name of the region
     * @param x1, y1, x2, y2 New normalized coordinates
     */
    void onRegionCoordinatesChanged(MainWindow* mainWindow,
                                   DocumentState* documentState,
                                   Canvas* canvas,
                                   const QString& regionName,
                                   double x1, double y1, double x2, double y2,
                                   const SaveStateCallback& saveState,
                                   const InvalidateCacheCallback& invalidateCache,
                                   const UpdateCanvasCallback& updateCanvas,
                                   const UpdateRegionListBoxCallback& updateRegionListBox,
                                   const UpdateUndoRedoButtonsCallback& updateUndoRedoButtons,
                                   const GetDocumentImageCallback& getDocumentImage,
                                   const GetScaleFactorCallback& getScaleFactor,
                                   const GetImageOffsetCallback& getImageOffset,
                                   const SynchronizeCoordinatesCallback& synchronizeCoordinates);
};

} // namespace ocr_orc

#endif // MAIN_WINDOW_REGION_EDITOR_HANDLERS_H


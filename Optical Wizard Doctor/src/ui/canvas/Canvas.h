#ifndef CANVAS_H
#define CANVAS_H

#include <QtWidgets/QWidget>
#include <QtGui/QImage>
#include <QtCore/QPointF>
#include <QtCore/QString>
#include <QtCore/QSet>
#include <QtCore/QMap>
#include <QtCore/QTimer>
#include <QtGui/QMouseEvent>
#include "../../models/DocumentState.h"
#include "../../core/CoordinateSystem.h"
#include "../../core/Constants.h"
#include "core/coordinate/CanvasCoordinateCache.h"
#include "core/rendering/CanvasRenderer.h"
#include "core/zoom/CanvasZoomController.h"
#include "core/regions/CanvasRegionOperations.h"
#include "core/selection/CanvasSelectionManager.h"
#include "core/coordinate/CanvasHitTester.h"
#include "core/regions/CanvasRegionCreator.h"
#include "input/CanvasInputHandler.h"
#include "managers/CanvasRegionManager.h"
#include "ui/CanvasContextMenuBuilder.h"
#include "managers/CanvasZoomManager.h"
#include "managers/CanvasStateManager.h"
#include "managers/CanvasRegionCreationManager.h"
#include "CanvasMouseMode.h"

namespace ocr_orc {

/**
 * @brief Canvas widget for displaying PDF documents and rendering regions
 * 
 * The Canvas widget is responsible for:
 * - Displaying PDF pages as images
 * - Rendering regions with proper styling
 * - Handling mouse events for region creation and manipulation
 * - Managing zoom and pan state
 * - Coordinate transformations
 */
class Canvas : public QWidget {
    Q_OBJECT

public:
    explicit Canvas(QWidget *parent = nullptr);
    ~Canvas();

    /**
     * @brief Load a PDF file and display the first page
     * @param filePath Path to the PDF file
     * @return true if PDF loaded successfully, false otherwise
     */
    bool loadPdf(const QString& filePath);
    
    /**
     * @brief Set the document image directly (for testing or alternative loading)
     * @param image The image to display
     */
    void setImage(const QImage& image);
    
    /**
     * @brief Check if a document is currently loaded
     * @return true if document is loaded, false otherwise
     */
    bool hasDocument() const { return !documentImage.isNull(); }
    
    /**
     * @brief Get the current document image (for DocumentState)
     * @return QImage copy of the document image
     */
    QImage getDocumentImage() const { return documentImage; }
    
    /**
     * @brief Set the DocumentState to use for region rendering
     * @param state Pointer to DocumentState (not owned, must outlive Canvas)
     */
    void setDocumentState(DocumentState* state);
    
    /**
     * @brief Set the hovered region (for visual feedback)
     * @param regionName Name of hovered region, or empty string for none
     */
    void setHoveredRegion(const QString& regionName);
    
    /**
     * @brief Set the selected regions
     * @param regionNames Set of selected region names
     */
    void setSelectedRegions(const QSet<QString>& regionNames);
    
    /**
     * @brief Get the currently selected regions
     * @return Set of selected region names
     */
    QSet<QString> getSelectedRegions() const { return selectedRegions; }
    
    /**
     * @brief Get the primary selected region (last selected when multiple are selected)
     * @return Name of primary selected region, or empty string if none
     */
    QString getPrimarySelectedRegion() const { return primarySelectedRegion; }
    
    /**
     * @brief Get current scale factor
     * @return Current scale factor
     */
    double getScaleFactor() const { return scaleFactor; }
    
    /**
     * @brief Get current image offset
     * @return Current image offset
     */
    QPointF getImageOffset() const { return imageOffset; }
    
    /**
     * @brief Invalidate coordinate cache (call when region coordinates change)
     * Forces recalculation of cached canvas coordinates
     */
    void invalidateCoordinateCache();
    
    /**
     * @brief Clear all selections
     */
    void clearSelection();
    
    /**
     * @brief Select a single region (clears other selections)
     * @param regionName Name of region to select
     */
    void selectRegion(const QString& regionName);
    
    /**
     * @brief Toggle selection of a region (for multi-select)
     * @param regionName Name of region to toggle
     */
    void toggleRegionSelection(const QString& regionName);
    
    /**
     * @brief Add regions to selection (for box selection)
     * @param regionNames Set of region names to add
     */
    void addToSelection(const QSet<QString>& regionNames);
    
    /**
     * @brief Mouse mode enum
     */
    // MouseMode enum is defined in CanvasMouseMode.h
    
    /**
     * @brief Set the mouse mode
     * @param mode Create or Select mode
     */
    void setMode(MouseMode mode);
    
    /**
     * @brief Get the current mouse mode
     * @return Current mouse mode
     */
    MouseMode getMode() const { return mouseMode; }
    
    /**
     * @brief Set rotate mode (replaces resize handles with rotate icon)
     * @param enabled True to enable rotate mode, false to disable
     */
    void setRotateMode(bool enabled);
    
    /**
     * @brief Get rotate mode state
     */
    bool getRotateMode() const { return isRotateMode; }
    
    /**
     * @brief Set shape type for region creation
     * @param shapeType Shape type: "rect", "circle", "triangle", "poly"
     */
    void setShapeType(const QString& shapeType);
    
    /**
     * @brief Get current shape type
     */
    QString getShapeType() const { return shapeType; }
    
            // Zoom functions
            void zoomIn();
            void zoomOut();
            void zoomReset();
            void setZoom(double zoomLevel); // Set zoom level (1.0 = 100%)
            void setZoomAtPoint(double zoomLevel, const QPointF& widgetPos); // Zoom centered on a point
            double getZoom() const;
    
    /**
     * @brief Finish region creation and add to DocumentState
     * Called by MainWindow after mouse release to complete region creation
     * @param regionName Name for the new region
     * @param color Color for the new region
     * @param group Group name (optional)
     * @return true if region was created successfully, false otherwise
     */
    bool finishRegionCreation(const QString& regionName, 
                              const QString& color, 
                              const QString& group = QString());
    
signals:
    /**
     * @brief Signal emitted when a region is created
     * @param regionName Name of the created region
     */
    void regionCreated(const QString& regionName);
    
    /**
     * @brief Signal emitted when selection changes
     */
    void selectionChanged();
    
    /**
     * @brief Signal emitted when zoom level changes
     * @param zoomLevel The new zoom level
     */
    void zoomChanged(double zoomLevel);
    
    /**
     * @brief Signal emitted when mouse is released during region creation
     * MainWindow should respond by calling finishRegionCreation() with name/color/group
     */
    void regionCreationRequested();
    
    /**
     * @brief Signal emitted when regions are duplicated
     * @param duplicatedNames List of names of newly created duplicate regions
     */
    void regionsDuplicated(const QList<QString>& duplicatedNames);
    
    /**
     * @brief Signal emitted when undo is requested (keyboard shortcut)
     */
    void undoRequested();
    
    /**
     * @brief Signal emitted when redo is requested (keyboard shortcut)
     */
    void redoRequested();
    
    /**
     * @brief Signal emitted when region name should be edited
     * @param regionName Name of region to edit
     */
    void regionEditRequested(const QString& regionName);
    
    /**
     * @brief Signal emitted when region color should be changed
     * @param regionName Name of region to change color for
     */
    void regionColorChangeRequested(const QString& regionName);
    
    /**
     * @brief Signal emitted when document state changes (for UI updates)
     * Used to notify MainWindow to update undo/redo buttons and lists
     */
    void stateChanged();

protected:
    /**
     * @brief Paint event - renders the PDF image and regions
     */
    void paintEvent(QPaintEvent* event) override;
    
    /**
     * @brief Resize event - recalculates layout when canvas is resized
     */
    void resizeEvent(QResizeEvent* event) override;
    
    /**
     * @brief Mouse press event - handle region creation start or selection
     */
    void mousePressEvent(QMouseEvent* event) override;
    
    /**
     * @brief Mouse move event - handle region creation drag or selection drag
     */
    void mouseMoveEvent(QMouseEvent* event) override;
    
    /**
     * @brief Mouse release event - finalize region creation or selection
     */
    void mouseReleaseEvent(QMouseEvent* event) override;
    
    /**
     * @brief Mouse double-click event - edit region name
     */
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    
    /**
     * @brief Wheel event - handle zoom with Ctrl/Cmd + wheel
     */
    void wheelEvent(QWheelEvent* event) override;
    
    /**
     * @brief Key press event - handle Escape to cancel drag/resize, Command+D to duplicate
     */
    void keyPressEvent(QKeyEvent* event) override;
    
    /**
     * @brief Context menu event - show right-click menu with duplicate option
     */
    void contextMenuEvent(QContextMenuEvent* event) override;
    
    /**
     * @brief Duplicate selected regions
     * Creates copies of all selected regions with auto-incremented names
     * @return List of names of newly created duplicate regions
     */
    QList<QString> duplicateSelectedRegions();

private:
    /**
     * @brief Calculate image position and scale to fit canvas
     * Maintains aspect ratio and centers the image
     */
    void calculateLayout();
    
    /**
     * @brief Start region creation at the given position
     * @param pos Canvas coordinates where creation starts
     */
    void startRegionCreation(const QPointF& pos);
    
    /**
     * @brief Update region creation during drag
     * @param pos Current canvas coordinates
     */
    void updateRegionCreation(const QPointF& pos);
    
    /**
     * @brief Find all regions intersecting with the given box
     * @param box Selection box rectangle in canvas coordinates
     * @return Set of region names that intersect the box
     */
    QSet<QString> findRegionsInBox(const QRectF& box) const;
    
    /**
     * @brief Move a region by delta in canvas coordinates
     * @param regionName Name of region to move
     * @param delta Movement delta in canvas coordinates
     */
    void moveRegion(const QString& regionName, const QPointF& delta);
    
    /**
     * @brief Move all selected regions by delta
     * @param delta Movement delta in canvas coordinates
     */
    void moveSelectedRegions(const QPointF& delta);
    
    /**
     * @brief Constrain region coordinates to valid bounds
     * @param region Region to constrain
     */
    void constrainToBounds(RegionData& region);
    
    /**
     * @brief Resize a region using a handle
     * @param regionName Name of region to resize
     * @param handle Handle being dragged ("nw", "ne", "sw", "se", "n", "s", "e", "w")
     * @param newPos New position of the handle in canvas coordinates
     */
    void resizeRegion(const QString& regionName, const QString& handle, const QPointF& newPos);
    
    
    /**
     * @brief Convert widget coordinates to canvas coordinates
     * @param widgetPos Position in widget coordinates
     * @return Position in canvas coordinates
     */
    QPointF widgetToCanvas(const QPointF& widgetPos) const;
    
    /**
     * @brief Validate region before creation
     * @param rect Canvas rectangle
     * @return true if valid, false otherwise
     */
    bool validateRegion(const QRectF& rect) const;
    
    /**
     * @brief Find region at the given canvas position (hit testing)
     * @param canvasPos Position in canvas coordinates
     * @return Region name if found, empty string otherwise
     */
    QString getRegionAt(const QPointF& canvasPos) const;
    
    /**
     * @brief Check if point is on a resize handle
     * @param canvasPos Position in canvas coordinates
     * @param regionRect Region rectangle in canvas coordinates
     * @return Handle position string ("nw", "ne", "sw", "se", "n", "s", "e", "w") or empty
     */
    QString getHandleAt(const QPointF& canvasPos, const QRectF& regionRect) const;
    
    /**
     * @brief Update hover state based on mouse position
     * @param canvasPos Current mouse position in canvas coordinates
     */
    void updateHoverState(const QPointF& canvasPos);
    
    /**
     * @brief Batched update (queues update instead of immediate repaint)
     * @param rect Optional dirty region to update
     */
    void batchedUpdate(const QRect& rect = QRect());

    // Document image
    QImage documentImage;
    
    // Layout calculations
    double scaleFactor;      // Scale to fit canvas (maintains aspect ratio)
    QPointF imageOffset;     // Offset to center image
    QRectF imageRect;        // Calculated image rectangle in canvas coordinates
    
    // Document state (not owned)
    DocumentState* documentState;
    
    // Selection state
    QString hoveredRegion;      // Currently hovered region name
    QSet<QString> selectedRegions;  // Set of selected region names
    QString primarySelectedRegion;  // Primary selected region (for resize handles)
    
    // Box selection state
    bool isBoxSelecting;        // True when performing box selection
    QPointF boxSelectionStart;   // Start position for box selection
    QRectF selectionBox;        // Current selection box rectangle
    
    // Drag state (for region movement)
    bool isDragging;            // True when dragging selected regions
    QPointF dragStartPos;       // Starting position of drag
    QPointF lastDragPos;        // Last position (for delta calculation)
    QMap<QString, RegionData> dragStartState;  // Original state before drag (for cancel)
    
    // Resize state
    bool isResizing;            // True when resizing a region
    QString resizingRegion;      // Region being resized
    QString resizeHandle;        // Which handle is being dragged ("nw", "ne", etc.)
    QRectF resizeStartRect;      // Original region rectangle before resize
    RegionData resizeStartRegionData;  // Original region data before resize (for cancel)
    
    // Rotation state
    bool isRotating;            // True when rotating a region
    QString rotatingRegion;     // Region being rotated
    QPointF rotationCenter;     // Center point for rotation
    double rotationStartAngle;  // Starting angle when rotation began
    double rotationAngle;       // Current rotation angle (in degrees)
    
    // Pan state
    bool isPanning;              // True when panning (middle-click or space+drag)
    QPointF panStartPos;         // Starting position for pan
    QPointF panStartOffset;      // Starting imageOffset when pan begins
    
    // Mouse mode
    MouseMode mouseMode;        // Current mouse mode (Create or Select)
    bool isRotateMode;          // True when rotate mode is active (replaces resize handles with rotate icon)
    QString shapeType;          // Current shape type for creation: "rect", "circle", "triangle", "poly"
    
    // Region creation state
    bool isCreating;            // True when creating a region
    QPointF creationStartPos;  // Start position for region creation
    QRectF tempRect;           // Temporary rectangle during creation
    
    // Performance optimization: Coordinate caching
    CanvasCoordinateCache* coordinateCache;  // Coordinate cache manager
    
    // Rendering
    CanvasRenderer* renderer;  // Renderer for all painting operations
    
    // Zoom and pan
    CanvasZoomController* zoomController;  // Zoom and pan controller
    
    // Region operations
    CanvasRegionOperations* regionOperations;  // Region manipulation operations
    
    // Selection management
    CanvasSelectionManager* selectionManager;  // Selection operations
    
    // Hit testing
    CanvasHitTester* hitTester;  // Hit testing operations
    
    // Region creation
    CanvasRegionCreator* regionCreator;  // Region creation logic
    
    // Input handling
    CanvasInputHandler* inputHandler;  // Input event handler
    
    // Region management
    CanvasRegionManager* regionManager;  // Region operations manager
    
    // Context menu
    CanvasContextMenuBuilder* contextMenuBuilder;  // Context menu builder
    
    // Zoom management
    CanvasZoomManager* zoomManager;  // Zoom operations manager
    
    // State management
    CanvasStateManager* stateManager;  // State operations manager
    
    // Region creation management
    CanvasRegionCreationManager* regionCreationManager;  // Region creation operations
    
    // Performance optimization: Batched updates
    QTimer* updateTimer;                   // Timer for batching update() calls
    QRect pendingUpdateRect;               // Accumulated dirty region for batched update
    
    // Performance optimization: Cached colors
    mutable QMap<QString, QColor> colorCache;  // Cache QColor objects by color name
    
    // Constants
    static constexpr double SHADOW_OFFSET = 5.0;
    static constexpr int SHADOW_ALPHA = 100;
    // Constants moved to Constants.h
};

} // namespace ocr_orc

#endif // CANVAS_H


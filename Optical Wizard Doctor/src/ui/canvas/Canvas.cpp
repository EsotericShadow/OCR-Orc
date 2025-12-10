#include "Canvas.h"
#include "core/coordinate/CanvasCoordinateCache.h"
#include "../../utils/PdfLoader.h"
#include "../../core/Constants.h"
#include <QtWidgets/QApplication>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QKeyEvent>
#include <QtGui/QWheelEvent>
#include <QtGui/QContextMenuEvent>
#include <QtWidgets/QMenu>
#if OCR_ORC_DEBUG_ENABLED
#include <QtCore/QDebug>
#endif
#include <QtCore/QSet>
#include <QtCore/QMap>
#include <QtCore/QPointF>
#include <QtCore/QRectF>
#include <algorithm>
#include <cmath>

namespace ocr_orc {

Canvas::Canvas(QWidget *parent)
    : QWidget(parent)
    , scaleFactor(1.0)
    , imageOffset(0.0, 0.0)
    , documentState(nullptr)
    , mouseMode(ocr_orc::Create)  // Use fully qualified name
    , isRotateMode(false)
    , shapeType("rect")  // Default to rectangle
    , isCreating(false)
    , isBoxSelecting(false)
    , isDragging(false)
    , isResizing(false)
    , resizingRegion()
    , resizeHandle()
    , resizeStartRect()
    , isRotating(false)
    , rotatingRegion()
    , rotationCenter()
    , rotationStartAngle(0.0)
    , rotationAngle(0.0)
    , isPanning(false)
    , panStartPos()
    , panStartOffset()
    , coordinateCache(new CanvasCoordinateCache())
    , renderer(new CanvasRenderer())
    , zoomController(new CanvasZoomController())
    , regionOperations(new CanvasRegionOperations())
    , selectionManager(new CanvasSelectionManager())
    , hitTester(new CanvasHitTester())
    , regionCreator(new CanvasRegionCreator())
    , inputHandler(new CanvasInputHandler())
    , regionManager(new CanvasRegionManager())
    , contextMenuBuilder(new CanvasContextMenuBuilder())
    , zoomManager(new CanvasZoomManager())
    , stateManager(new CanvasStateManager())
    , regionCreationManager(new CanvasRegionCreationManager())
    , updateTimer(new QTimer(this))
{
    setMinimumSize(400, 300);
    setStyleSheet("background-color: #d0d0d0; color: #000000;");
    setMouseTracking(true); // Enable mouse tracking for hover effects
    setCursor(Qt::CrossCursor); // Default to crosshair for Create mode
    setFocusPolicy(Qt::StrongFocus); // Enable keyboard focus for shortcuts
    
    // Setup update batching timer (single shot, 16ms = ~60fps)
    updateTimer->setSingleShot(true);
    updateTimer->setInterval(16);
    connect(updateTimer, &QTimer::timeout, this, [this]() {
        if (!pendingUpdateRect.isEmpty()) {
            QWidget::update(pendingUpdateRect);
            pendingUpdateRect = QRect();
        } else {
            QWidget::update();
        }
    });
}

Canvas::~Canvas() {
    // QImage is automatically cleaned up
    delete coordinateCache;
    delete renderer;
    delete zoomController;
    delete regionOperations;
    delete selectionManager;
    delete hitTester;
    delete regionCreator;
    delete inputHandler;
    delete regionManager;
    delete contextMenuBuilder;
    delete zoomManager;
    delete stateManager;
    delete regionCreationManager;
}

bool Canvas::loadPdf(const QString& filePath) {
    // Load PDF first page using PdfLoader
    QImage image = PdfLoader::loadPdfFirstPage(filePath, PdfConstants::DEFAULT_DPI);
    
    if (image.isNull()) {
        OCR_ORC_WARNING("Canvas: Failed to load PDF:" << filePath);
        return false;
    }
    
    // Set the image
    setImage(image);
    return true;
}

void Canvas::setImage(const QImage& image) {
    // QImage uses implicit sharing (copy-on-write), so regular assignment is efficient
    documentImage = image;
    
    // Invalidate coordinate cache (image changed)
    invalidateCoordinateCache();
    
    // Recalculate layout
    calculateLayout();
    
    // Trigger repaint
    update();
}

void Canvas::calculateLayout() {
    if (!zoomController) {
        // Fallback to old implementation if controller not available
        if (documentImage.isNull()) {
            scaleFactor = 1.0;
            imageOffset = QPointF(0.0, 0.0);
            imageRect = QRectF();
            invalidateCoordinateCache();
            return;
        }
        return;
    }
    
    // Delegate to zoom controller
    zoomController->calculateLayout(documentImage, width(), height(),
                                    documentState, scaleFactor, imageOffset, imageRect);
    
    // Invalidate cache if image was cleared
    if (documentImage.isNull()) {
        invalidateCoordinateCache();
    }
}

void Canvas::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    
    // Set render hints once (optimization: don't set on every paint)
    static bool hintsSet = false;
    if (!hintsSet) {
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        hintsSet = true;
    } else {
        // Re-enable hints (they may be reset by Qt)
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    }
    
    // Get the exposed region (for dirty region updates)
    QRect exposedRect = event->rect();
    
    // Draw background (only exposed region for optimization)
    painter.fillRect(exposedRect, QColor(0xd0, 0xd0, 0xd0));
    
    // Draw document image if loaded
    if (!documentImage.isNull() && !imageRect.isEmpty()) {
        // Only draw image if it intersects exposed region
        if (imageRect.intersects(exposedRect)) {
            if (renderer) {
                renderer->drawDocumentImage(painter, documentImage, imageRect);
            }
            
            // Draw regions if DocumentState is available
            if (documentState && renderer) {
                renderer->renderRegions(painter, documentState, coordinateCache,
                                       documentImage, scaleFactor, imageOffset,
                                       rect(), hoveredRegion, selectedRegions,
                                       primarySelectedRegion, isRotateMode,
                                       isRotating ? rotatingRegion : QString(), rotationAngle);
            }
        }
        
        // Draw temporary rectangle during region creation (always visible)
        if (isCreating && !tempRect.isEmpty() && renderer) {
            renderer->drawTempShape(painter, tempRect, shapeType);
        }
        
        // Draw selection box during box selection (always visible)
        if (isBoxSelecting && !selectionBox.isEmpty() && renderer) {
            renderer->drawSelectionBox(painter, selectionBox);
        }
    } else {
        // Draw placeholder text (only if exposed region contains center)
        if (exposedRect.contains(rect().center())) {
            painter.setPen(QPen(QColor(0x00, 0x00, 0x00))); // Black text for contrast
            painter.setFont(QFont("Arial", 14));
            painter.drawText(rect(), Qt::AlignCenter, "No document loaded\n\nLoad a PDF to begin");
        }
    }
}

void Canvas::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    
    // Recalculate layout when canvas is resized
    calculateLayout();
    
    // Trigger repaint
    update();
}

void Canvas::setDocumentState(DocumentState* state) {
    documentState = state;
    invalidateCoordinateCache(); // Invalidate cache when state changes
    update(); // Trigger repaint
}

void Canvas::setHoveredRegion(const QString& regionName) {
    if (!stateManager) return;
    stateManager->setHoveredRegion(regionName, hoveredRegion);
    batchedUpdate();
}

void Canvas::setSelectedRegions(const QSet<QString>& regionNames) {
    selectedRegions = regionNames;
    // Update primarySelectedRegion - needed for resize handles
    if (selectedRegions.isEmpty()) {
        primarySelectedRegion = QString();
    } else if (selectedRegions.size() == 1) {
        primarySelectedRegion = *selectedRegions.begin();
    } else {
        // Multiple regions selected - keep primary if still selected, otherwise pick first
        if (!selectedRegions.contains(primarySelectedRegion)) {
            primarySelectedRegion = *selectedRegions.begin();
        }
    }
    emit selectionChanged();
    update();
}

void Canvas::invalidateCoordinateCache() {
    if (!stateManager) return;
    stateManager->invalidateCoordinateCache(coordinateCache);
}

void Canvas::batchedUpdate(const QRect& rect) {
    if (rect.isEmpty()) {
        // Full update
        if (!updateTimer->isActive()) {
            updateTimer->start();
        }
    } else {
        // Partial update - accumulate dirty region
        if (pendingUpdateRect.isEmpty()) {
            pendingUpdateRect = rect;
        } else {
            pendingUpdateRect = pendingUpdateRect.united(rect);
        }
        if (!updateTimer->isActive()) {
            updateTimer->start();
        }
    }
}

void Canvas::setMode(MouseMode mode) {
    mouseMode = mode;
    
    // Update cursor based on mode
    if (mode == Create) {
        setCursor(Qt::CrossCursor);
    } else {
        setCursor(Qt::ArrowCursor);
    }
    
    // Clear any ongoing creation
    if (isCreating) {
        isCreating = false;
        tempRect = QRectF();
        update();
    }
}

void Canvas::setRotateMode(bool enabled) {
    isRotateMode = enabled;
    update();
}

void Canvas::setShapeType(const QString& type) {
    shapeType = type;
    update();
}

void Canvas::mousePressEvent(QMouseEvent* event) {
    if (!inputHandler || !documentState || documentImage.isNull()) {
        return;
    }
    
    // Store previous selection to detect changes
    QSet<QString> previousSelection = selectedRegions;
    QString previousPrimary = primarySelectedRegion;
    
    inputHandler->handleMousePress(event,
                                   static_cast<ocr_orc::MouseMode>(mouseMode),
                                   isRotateMode,
                                   documentState, documentImage, imageRect,
                                   scaleFactor, imageOffset,
                                   selectedRegions, primarySelectedRegion,
                                   isCreating, creationStartPos, tempRect,
                                   isBoxSelecting, boxSelectionStart, selectionBox,
                                   isDragging, dragStartPos, lastDragPos, dragStartState,
                                   isResizing, resizingRegion, resizeHandle, resizeStartRect, resizeStartRegionData,
                                   isRotating, rotatingRegion, rotationCenter, rotationStartAngle, rotationAngle,
                                   isPanning, panStartPos, panStartOffset, imageOffset,
                                   hitTester, selectionManager, regionCreator, regionOperations,
                                   [this](const QPointF& pos) { return this->widgetToCanvas(pos); },
                                   [this]() { this->update(); },
                                   [this](Qt::CursorShape shape) { this->setCursor(shape); },
                                   [this]() { this->calculateLayout(); },
                                   [this](const QString&) { emit this->regionCreationRequested(); });
    
    // Emit selectionChanged if selection was modified
    if (selectedRegions != previousSelection || primarySelectedRegion != previousPrimary) {
        emit selectionChanged();
    }
}

void Canvas::mouseMoveEvent(QMouseEvent* event) {
    if (!inputHandler || !documentState || documentImage.isNull()) {
        return;
    }
    
    inputHandler->handleMouseMove(event,
                                   static_cast<ocr_orc::MouseMode>(mouseMode),
                                   isRotateMode,
                                  documentState, documentImage, imageRect,
                                  scaleFactor, imageOffset,
                                  selectedRegions, primarySelectedRegion, hoveredRegion,
                                  isCreating, creationStartPos, tempRect,
                                  isBoxSelecting, boxSelectionStart, selectionBox,
                                  isDragging, lastDragPos,
                                  isResizing, resizingRegion, resizeHandle, resizeStartRect,
                                  isRotating, rotatingRegion, rotationCenter, rotationStartAngle, rotationAngle,
                                  isPanning, panStartPos, panStartOffset, imageOffset,
                                  hitTester, selectionManager, regionCreator, regionOperations,
                                  [this](const QPointF& pos) { return this->widgetToCanvas(pos); },
                                  [this](const QString& name, const QPointF& delta) { this->moveRegion(name, delta); },
                                  [this](const QPointF& delta) { this->moveSelectedRegions(delta); },
                                  [this](const QString& name, const QString& handle, const QPointF& pos) { this->resizeRegion(name, handle, pos); },
                                  [this]() { this->update(); },
                                  [this]() { this->calculateLayout(); },
                                  [this](Qt::CursorShape shape) { this->setCursor(shape); });
}

void Canvas::mouseReleaseEvent(QMouseEvent* event) {
    if (!inputHandler || !documentState || documentImage.isNull()) {
        return;
    }
    
    // Store previous selection to detect changes
    QSet<QString> previousSelection = selectedRegions;
    QString previousPrimary = primarySelectedRegion;
    bool wasBoxSelecting = isBoxSelecting;
    
    inputHandler->handleMouseRelease(event,
                                    static_cast<ocr_orc::MouseMode>(mouseMode),
                                    documentState, documentImage, imageRect,
                                    scaleFactor, imageOffset,
                                    selectedRegions, primarySelectedRegion,
                                    isCreating, creationStartPos, tempRect,
                                    isBoxSelecting, boxSelectionStart, selectionBox,
                                    isDragging, lastDragPos, dragStartState,
                                    isResizing, resizingRegion, resizeHandle, resizeStartRegionData,
                                    isRotating, rotatingRegion, rotationStartAngle, rotationAngle,
                                    isPanning, panStartPos, panStartOffset, imageOffset,
                                    selectionManager, regionCreator, regionOperations,
                                    [this](const QPointF& pos) { return this->widgetToCanvas(pos); },
                                    [this](const QRectF& rect) { return this->validateRegion(rect); },
                                    [this](const QRectF& box) { return this->findRegionsInBox(box); },
                                    [this](const QString& name, const QPointF& delta) { this->moveRegion(name, delta); },
                                    [this](const QPointF& delta) { this->moveSelectedRegions(delta); },
                                    [this](const QString& name, const QString& handle, const QPointF& pos) { this->resizeRegion(name, handle, pos); },
                                    [this]() { this->update(); },
                                    [this]() { this->calculateLayout(); },
                                    [this](Qt::CursorShape shape) { this->setCursor(shape); },
                                    [this](const QString&) { emit this->regionCreationRequested(); },
                                    [this]() { emit this->stateChanged(); });
    
    // Emit selectionChanged if selection was modified (especially after box selection)
    if (selectedRegions != previousSelection || primarySelectedRegion != previousPrimary || wasBoxSelecting) {
        emit selectionChanged();
    }
}

void Canvas::mouseDoubleClickEvent(QMouseEvent* event) {
    if (!inputHandler || !documentState || documentImage.isNull()) {
        return;
    }
    
    inputHandler->handleMouseDoubleClick(event,
                                        static_cast<ocr_orc::MouseMode>(mouseMode),
                                        documentState, documentImage, imageRect,
                                        scaleFactor, imageOffset,
                                        hitTester,
                                        [this](const QPointF& pos) { return this->widgetToCanvas(pos); },
                                        [this](const QString& name) { emit this->regionEditRequested(name); });
}

QPointF Canvas::widgetToCanvas(const QPointF& widgetPos) const {
    // Widget coordinates are already in canvas coordinates
    // (since Canvas is the widget itself)
    return widgetPos;
}

void Canvas::startRegionCreation(const QPointF& pos) {
    if (!regionCreator) {
        return;
    }
    if (regionCreator->startRegionCreation(pos, imageRect, isCreating, creationStartPos, tempRect)) {
        update();
    }
}

void Canvas::updateRegionCreation(const QPointF& pos) {
    if (!regionCreator) {
        return;
    }
    regionCreator->updateRegionCreation(pos, isCreating, creationStartPos, imageRect, tempRect);
    update(); // Trigger repaint to show temporary rectangle
}

bool Canvas::finishRegionCreation(const QString& regionName, 
                                  const QString& color, 
                                  const QString& group) {
    if (!regionCreator || !documentState) {
        return false;
    }
    
    // Create lambda for validateRegion
    auto validateFunc = [this](const QRectF& rect, const QRectF& /*imgRect*/) -> bool {
        return this->validateRegion(rect);
    };
    
    bool result = regionCreator->finishRegionCreation(regionName, color, group, shapeType,
                                                      isCreating, tempRect, documentState,
                                                      documentImage, scaleFactor, imageOffset,
                                                      validateFunc);
    
    if (result) {
        update();
        emit regionCreated(regionName);
    }
    
    return result;
}

bool Canvas::validateRegion(const QRectF& rect) const {
    if (!regionOperations) {
        return rect.width() >= RegionConstants::MIN_REGION_SIZE &&
               rect.height() >= RegionConstants::MIN_REGION_SIZE &&
               imageRect.contains(rect);
    }
    return regionOperations->validateRegion(rect, imageRect);
}

QString Canvas::getRegionAt(const QPointF& canvasPos) const {
    if (!hitTester) {
        return QString();
    }
    return hitTester->getRegionAt(canvasPos, documentState, documentImage,
                                  imageRect, scaleFactor, imageOffset);
}

QString Canvas::getHandleAt(const QPointF& canvasPos, const QRectF& regionRect) const {
    if (!hitTester) {
        return QString();
    }
    return hitTester->getHandleAt(canvasPos, regionRect);
}

void Canvas::updateHoverState(const QPointF& canvasPos) {
    if (!inputHandler) {
        return;
    }
    
    inputHandler->updateHoverState(canvasPos,
                                   static_cast<ocr_orc::MouseMode>(mouseMode),
                                   isRotateMode,
                                   documentState, documentImage, imageRect,
                                   scaleFactor, imageOffset,
                                   selectedRegions, primarySelectedRegion, hoveredRegion,
                                   hitTester,
                                   [this]() { this->update(); });
    // Note: Cursor is set in mouse move handler via hover manager
}

void Canvas::clearSelection() {
    if (!stateManager) return;
    stateManager->clearSelection(selectionManager, selectedRegions, primarySelectedRegion);
    emit selectionChanged();
    update();
}

void Canvas::selectRegion(const QString& regionName) {
    if (!stateManager) return;
    stateManager->selectRegion(regionName, selectionManager, selectedRegions, primarySelectedRegion);
    emit selectionChanged();
    update();
}

void Canvas::toggleRegionSelection(const QString& regionName) {
    if (!stateManager) return;
    stateManager->toggleRegionSelection(regionName, selectionManager, selectedRegions, primarySelectedRegion);
    emit selectionChanged();
    update();
}

void Canvas::addToSelection(const QSet<QString>& regionNames) {
    if (!stateManager) return;
    stateManager->addToSelection(regionNames, selectionManager, selectedRegions, primarySelectedRegion);
    emit selectionChanged();
    update();
}

QSet<QString> Canvas::findRegionsInBox(const QRectF& box) const {
    return selectionManager ? selectionManager->findRegionsInBox(box, documentState, documentImage,
                                                                  scaleFactor, imageOffset) : QSet<QString>();
}

void Canvas::moveRegion(const QString& regionName, const QPointF& delta) {
    if (!regionManager) return;
    regionManager->moveRegion(regionName, delta, documentState, documentImage,
                             scaleFactor, imageOffset, regionOperations);
    // Invalidate coordinate cache after region move
    invalidateCoordinateCache();
    
    // If this region is currently selected, emit selectionChanged to update region editor
    if (selectedRegions.contains(regionName)) {
        emit selectionChanged();
    }
    
    update();
}

void Canvas::moveSelectedRegions(const QPointF& delta) {
    if (!regionManager) return;
    regionManager->moveSelectedRegions(delta, documentState, documentImage,
                                      scaleFactor, imageOffset, selectedRegions, regionOperations);
    // Invalidate coordinate cache after region move
    invalidateCoordinateCache();
    
    // If any regions are selected, emit selectionChanged to update region editor
    if (!selectedRegions.isEmpty()) {
        emit selectionChanged();
    }
    
    update();
}

QList<QString> Canvas::duplicateSelectedRegions() {
    if (!regionManager) return QList<QString>();
    
    QList<QString> duplicatedNames = regionManager->duplicateSelectedRegions(
        documentState, selectedRegions, selectedRegions, primarySelectedRegion);
    
    if (!duplicatedNames.isEmpty()) {
        emit regionsDuplicated(duplicatedNames);
        update();
    }
    return duplicatedNames;
}

void Canvas::contextMenuEvent(QContextMenuEvent* event) {
    if (!documentState || documentImage.isNull() || !contextMenuBuilder) {
        return;
    }
    
    QPointF canvasPos = widgetToCanvas(event->pos());
    QString regionAtPos = getRegionAt(canvasPos);
    
    QMenu menu(this);
    contextMenuBuilder->buildContextMenu(&menu, documentState, documentImage, canvasPos,
                                        selectedRegions, regionAtPos,
                                        [this]() { return this->duplicateSelectedRegions(); },
                                        [this](const QString& name) { this->selectRegion(name); },
                                        [this](const QString& name) { emit this->regionEditRequested(name); },
                                        [this](const QString& name) { emit this->regionColorChangeRequested(name); },
                                        [this](const QList<QString>& names) { emit this->regionsDuplicated(names); },
                                        mouseMode,
                                        [this](const QString& type) { this->setShapeType(type); });
    
    if (!menu.isEmpty()) {
        menu.exec(event->globalPos());
    }
}

void Canvas::constrainToBounds(RegionData& region) {
    if (regionOperations) {
        regionOperations->constrainToBounds(region);
    }
}

void Canvas::resizeRegion(const QString& regionName, const QString& handle, const QPointF& newPos) {
    OCR_ORC_DEBUG("[CANVAS RESIZE] === Canvas::resizeRegion ===");
    OCR_ORC_DEBUG("[CANVAS RESIZE]   regionName: " << regionName);
    OCR_ORC_DEBUG("[CANVAS RESIZE]   handle: " << handle);
    OCR_ORC_DEBUG("[CANVAS RESIZE]   newPos: (" << newPos.x() << "," << newPos.y() << ")");
    
    if (!documentState || !regionOperations) {
        OCR_ORC_WARNING("[CANVAS RESIZE]   Invalid documentState or regionOperations");
        return;
    }
    
    // Get rotation angle from region data
    double rotationAngle = 0.0;
    if (documentState->hasRegion(regionName)) {
        RegionData region = documentState->getRegion(regionName);
        rotationAngle = region.rotationAngle;
        OCR_ORC_DEBUG("[CANVAS RESIZE]   region.rotationAngle: " << rotationAngle << "degrees");
        OCR_ORC_DEBUG("[CANVAS RESIZE]   region.normalizedCoords: x1=" << region.normalizedCoords.x1 << " y1=" << region.normalizedCoords.y1 << " x2=" << region.normalizedCoords.x2 << " y2=" << region.normalizedCoords.y2);
    }
    
    OCR_ORC_DEBUG("[CANVAS RESIZE]   resizeStartRect: (" << resizeStartRect.left() << "," << resizeStartRect.top() << ") size: " << resizeStartRect.width() << "x" << resizeStartRect.height());
    OCR_ORC_DEBUG("[CANVAS RESIZE]   scaleFactor: " << scaleFactor);
    OCR_ORC_DEBUG("[CANVAS RESIZE]   imageOffset: (" << imageOffset.x() << "," << imageOffset.y() << ")");
    OCR_ORC_DEBUG("[CANVAS RESIZE]   imageRect: (" << imageRect.left() << "," << imageRect.top() << ") size: " << imageRect.width() << "x" << imageRect.height());
    
    // CRITICAL FIX: Pass the original normalized coords from resizeStartRegionData
    // so we can calculate the original unrotated rectangle, not the current one
    NormalizedCoords originalNorm = resizeStartRegionData.normalizedCoords;
    regionOperations->resizeRegion(regionName, handle, newPos, resizeStartRect,
                                  documentState, documentImage, scaleFactor,
                                  imageOffset, imageRect, rotationAngle, originalNorm);
    // Invalidate coordinate cache after region resize
    invalidateCoordinateCache();
    
    // If this region is currently selected, emit selectionChanged to update region editor
    if (selectedRegions.contains(regionName)) {
        emit selectionChanged();
    }
    
    update();
}

void Canvas::keyPressEvent(QKeyEvent* event) {
    if (!inputHandler) {
        QWidget::keyPressEvent(event);
        return;
    }
    
    inputHandler->handleKeyPress(event,
                                documentState,
                                selectedRegions,
                                isDragging, dragStartState,
                                isResizing, resizingRegion, resizeHandle, resizeStartRegionData,
                                 reinterpret_cast<ocr_orc::MouseMode&>(mouseMode),
                                [this]() { this->duplicateSelectedRegions(); },
                                [this]() { this->zoomIn(); },
                                [this]() { this->zoomOut(); },
                                [this]() { this->zoomReset(); },
                                [this](ocr_orc::MouseMode mode) { this->setMode(static_cast<MouseMode>(mode)); },
                                [this]() { this->update(); },
                                [this](Qt::CursorShape shape) { this->setCursor(shape); },
                                [this](const QString&) { emit this->undoRequested(); },
                                [this](const QString&) { emit this->redoRequested(); },
                                [this]() { emit this->stateChanged(); });
    
    // If event wasn't accepted, pass to base class
    if (!event->isAccepted()) {
        QWidget::keyPressEvent(event);
    }
}

void Canvas::zoomIn() {
    if (!zoomManager) return;
    zoomManager->zoomIn(documentState, documentImage, width(), height(),
                       scaleFactor, imageOffset, zoomController, coordinateCache);
    calculateLayout();
    update();
}

void Canvas::zoomOut() {
    if (!zoomManager) return;
    zoomManager->zoomOut(documentState, documentImage, width(), height(),
                        scaleFactor, imageOffset, zoomController, coordinateCache);
    calculateLayout();
    update();
}

void Canvas::zoomReset() {
    if (!zoomManager) return;
    zoomManager->zoomReset(documentState, documentImage, width(), height(),
                          scaleFactor, imageOffset, zoomController, coordinateCache);
    calculateLayout();
    update();
}

void Canvas::setZoom(double zoomLevel) {
    if (!zoomManager) return;
    zoomManager->setZoom(zoomLevel, documentState, documentImage,
                        width(), height(), scaleFactor, imageOffset,
                        zoomController, coordinateCache);
    calculateLayout();
    update();
}

void Canvas::setZoomAtPoint(double zoomLevel, const QPointF& widgetPos) {
    if (!zoomManager) return;
    zoomManager->setZoomAtPoint(zoomLevel, widgetPos, documentState, documentImage,
                               width(), height(), scaleFactor, imageOffset,
                               zoomController, coordinateCache);
    calculateLayout();
    update();
}

double Canvas::getZoom() const {
    if (!zoomManager) {
        return documentState ? documentState->zoomLevel : 1.0;
    }
    return zoomManager->getZoom(documentState, zoomController);
}

// handleSelectModePress and handleSelectModeRelease are now in CanvasInputHandler

void Canvas::wheelEvent(QWheelEvent* event) {
    if (!documentState || documentImage.isNull()) {
        return;
    }
    
    // Disable wheel/pan during resize operations to prevent interference
    if (isResizing) {
        event->ignore();
        return;
    }
    
    bool isModifierPressed = (event->modifiers() & Qt::ControlModifier) ||
                             (event->modifiers() & Qt::MetaModifier);
    
    if (isModifierPressed) {
        QPointF mousePos = event->position().toPoint();
        double delta = event->angleDelta().y() / static_cast<double>(CanvasConstants::WHEEL_DELTA_NORMALIZATION);
        double currentZoom = documentState->zoomLevel;
        double zoomFactor = 1.0 - (delta * CanvasConstants::ZOOM_WHEEL_FACTOR);
        double newZoom = currentZoom * zoomFactor;
        setZoomAtPoint(newZoom, mousePos);
        event->accept();
    } else {
        QPointF delta = event->angleDelta();
        if (zoomController) {
            zoomController->panWithWheel(delta, documentState, imageOffset);
        }
        calculateLayout();
        update();
        event->accept();
    }
}

} // namespace ocr_orc


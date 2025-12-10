#include "CanvasKeyboardHandler.h"
#include "../../../core/Constants.h"
#if OCR_ORC_DEBUG_ENABLED
#include <QtCore/QDebug>
#endif

namespace ocr_orc {

CanvasKeyboardHandler::CanvasKeyboardHandler() {
}

CanvasKeyboardHandler::~CanvasKeyboardHandler() {
}

void CanvasKeyboardHandler::handleKeyPress(QKeyEvent* event,
                                           DocumentState* documentState,
                                           QSet<QString>& selectedRegions,
                                           bool& isDragging,
                                           QMap<QString, RegionData>& dragStartState,
                                           bool& isResizing,
                                           QString& resizingRegion,
                                           QString& resizeHandle,
                                           RegionData& resizeStartRegionData,
                                           MouseMode& mouseMode,
                                           const DuplicateCallback& duplicateSelectedRegions,
                                           const ZoomInCallback& zoomIn,
                                           const ZoomOutCallback& zoomOut,
                                           const ZoomResetCallback& zoomReset,
                                           const SetModeCallback& setMode,
                                           const UpdateCallback& update,
                                           const SetCursorCallback& setCursor,
                                           const EmitSignalCallback& emitUndoRequested,
                                           const EmitSignalCallback& emitRedoRequested,
                                           const EmitStateChangedCallback& emitStateChanged) {
    OCR_ORC_DEBUG("Canvas::keyPressEvent: key=" << event->key() << "modifiers=" << event->modifiers());
    
    if (event->key() == Qt::Key_Escape) {
        if (isDragging) {
            if (documentState && !dragStartState.isEmpty()) {
                for (auto it = dragStartState.begin(); it != dragStartState.end(); ++it) {
                    documentState->addRegion(it.key(), it.value());
                }
                documentState->synchronizeCoordinates();
                dragStartState.clear();
                update();
                emitStateChanged();
            }
            isDragging = false;
            setCursor(Qt::ArrowCursor);
        } else if (isResizing) {
            if (documentState && !resizingRegion.isEmpty()) {
                documentState->addRegion(resizingRegion, resizeStartRegionData);
                documentState->synchronizeCoordinates();
                update();
                emitStateChanged();
            }
            isResizing = false;
            resizingRegion = QString();
            resizeHandle = QString();
            resizeStartRegionData = RegionData();
            setCursor(Qt::ArrowCursor);
        }
    } else if (event->key() == Qt::Key_D && 
               (event->modifiers() & Qt::ControlModifier || event->modifiers() & Qt::MetaModifier)) {
        if (!selectedRegions.isEmpty()) {
            duplicateSelectedRegions();
        }
        event->accept();
        return;
    } else if (event->key() == Qt::Key_Z) {
        bool hasCtrl = (event->modifiers() & Qt::ControlModifier);
        bool hasMeta = (event->modifiers() & Qt::MetaModifier);
        bool hasAlt = (event->modifiers() & Qt::AltModifier);
        bool hasShift = (event->modifiers() & Qt::ShiftModifier);
        
        bool isUndo = (hasCtrl || hasMeta || hasAlt) && !hasShift;
        bool isRedo = (hasCtrl || hasMeta || hasAlt) && hasShift;
        
        if (isUndo) {
            emitUndoRequested(QString());
            event->accept();
            return;
        } else if (isRedo) {
            emitRedoRequested(QString());
            event->accept();
            return;
        }
    } else if (event->key() == Qt::Key_Plus || event->key() == Qt::Key_Equal) {
        zoomIn();
        event->accept();
        return;
    } else if (event->key() == Qt::Key_Minus) {
        zoomOut();
        event->accept();
        return;
    } else if (event->key() == Qt::Key_0) {
        zoomReset();
        event->accept();
        return;
    } else if (event->key() == Qt::Key_1) {
        setMode(Create);
        event->accept();
        return;
    } else if (event->key() == Qt::Key_2) {
        setMode(Select);
        event->accept();
        return;
    }
}

} // namespace ocr_orc


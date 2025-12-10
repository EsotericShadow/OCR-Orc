#include "MainWindowToolbarAdapter.h"
#include "../../../models/DocumentState.h"
#include "../../canvas/Canvas.h"

namespace ocr_orc {

MainWindowToolbarAdapter::MainWindowToolbarAdapter() {
}

MainWindowToolbarAdapter::~MainWindowToolbarAdapter() {
}

void MainWindowToolbarAdapter::updateUndoRedoButtons(DocumentState* documentState,
                                                     const GetCanUndoCallback& getCanUndo,
                                                     const GetCanRedoCallback& getCanRedo,
                                                     const UpdateUndoRedoButtonsCallback& updateButtons) {
    if (!documentState || !updateButtons) {
        return;
    }
    
    bool canUndo = getCanUndo ? getCanUndo() : documentState->canUndo();
    bool canRedo = getCanRedo ? getCanRedo() : documentState->canRedo();
    
    updateButtons(canUndo, canRedo);
}

void MainWindowToolbarAdapter::updateZoomLabel(Canvas* canvas,
                                               const GetZoomCallback& getZoom,
                                               const UpdateZoomLabelCallback& updateLabel) {
    if (!canvas || !updateLabel) {
        return;
    }
    
    double zoom = getZoom ? getZoom() : (canvas ? canvas->getZoom() : 1.0);
    updateLabel(zoom);
}

void MainWindowToolbarAdapter::updateRotateButton(Canvas* canvas,
                                                  const GetRotateModeCallback& getRotateMode,
                                                  const UpdateRotateButtonCallback& updateButton) {
    if (!canvas || !updateButton) {
        return;
    }
    
    bool rotateMode = getRotateMode ? getRotateMode() : (canvas ? canvas->getRotateMode() : false);
    updateButton(rotateMode);
}

void MainWindowToolbarAdapter::updateAll(DocumentState* documentState,
                                        Canvas* canvas,
                                        const GetCanUndoCallback& getCanUndo,
                                        const GetCanRedoCallback& getCanRedo,
                                        const GetZoomCallback& getZoom,
                                        const GetRotateModeCallback& getRotateMode,
                                        const UpdateUndoRedoButtonsCallback& updateUndoRedoButtons,
                                        const UpdateZoomLabelCallback& updateZoomLabel,
                                        const UpdateRotateButtonCallback& updateRotateButton) {
    // Update all toolbar states in one call
    this->updateUndoRedoButtons(documentState, getCanUndo, getCanRedo, updateUndoRedoButtons);
    this->updateZoomLabel(canvas, getZoom, updateZoomLabel);
    this->updateRotateButton(canvas, getRotateMode, updateRotateButton);
}

} // namespace ocr_orc


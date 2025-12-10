#include "MainWindowUndoRedo.h"
#include "../../../../models/DocumentState.h"
#include "../../../canvas/Canvas.h"
#include "../../../canvas/ui/CanvasUiSync.h"
#if OCR_ORC_DEBUG_ENABLED
#include <QtCore/QDebug>
#endif

namespace ocr_orc {

MainWindowUndoRedo::MainWindowUndoRedo() {
}

MainWindowUndoRedo::~MainWindowUndoRedo() {
}

void MainWindowUndoRedo::undo(DocumentState* documentState,
                             Canvas* canvas,
                             const CanUndoCallback& canUndo,
                             const UndoActionCallback& undoAction,
                             const UpdateUndoRedoButtonsCallback& updateUndoRedoButtons,
                             const UpdateRegionListBoxCallback& updateRegionListBox,
                             const UpdateGroupListBoxCallback& updateGroupListBox,
                             const UpdateCanvasCallback& /* updateCanvas */,
                             const ClearSelectionCallback& clearSelection,
                             const ShowStatusMessageCallback& showStatusMessage) {
    OCR_ORC_DEBUG("=== UNDO CALLED ===");
    OCR_ORC_DEBUG("  documentState=" << (documentState ? "exists" : "null"));
    
    if (!documentState) {
        OCR_ORC_DEBUG("  ERROR: documentState is null");
        return;
    }
    
    bool canUndoValue = canUndo();
    OCR_ORC_DEBUG("  canUndo=" << canUndoValue);
    
    if (!canUndoValue) {
        OCR_ORC_DEBUG("  Cannot undo - stack is empty");
        return;
    }
    
    OCR_ORC_DEBUG("  Calling undoAction()...");
    if (undoAction) {
        undoAction();
    }
    OCR_ORC_DEBUG("  undoAction() completed");
    
    // Clear selection before refresh (per current policy)
    if (clearSelection) {
        clearSelection();
    }
    
    // Use CanvasUiSync to refresh all UI components in correct order
    CanvasUiSync::refreshAll(
        documentState,
        [canvas]() {
            if (canvas) {
                canvas->invalidateCoordinateCache();
            }
        },
        updateRegionListBox,
        updateGroupListBox,
        [canvas]() {
            if (canvas) {
                canvas->update();
            }
        },
        [updateUndoRedoButtons, documentState]() {
            if (updateUndoRedoButtons && documentState) {
                bool canUndoAfter = documentState->canUndo();
                bool canRedoAfter = documentState->canRedo();
                updateUndoRedoButtons(canUndoAfter, canRedoAfter);
            }
        }
    );
    
    if (showStatusMessage) {
        showStatusMessage("Undo completed", 2000);
    }
    OCR_ORC_DEBUG("=== UNDO COMPLETED ===");
}

void MainWindowUndoRedo::redo(DocumentState* documentState,
                             Canvas* canvas,
                             const CanRedoCallback& canRedo,
                             const RedoActionCallback& redoAction,
                             const UpdateUndoRedoButtonsCallback& updateUndoRedoButtons,
                             const UpdateRegionListBoxCallback& updateRegionListBox,
                             const UpdateGroupListBoxCallback& updateGroupListBox,
                             const UpdateCanvasCallback& /* updateCanvas */,
                             const ClearSelectionCallback& clearSelection,
                             const ShowStatusMessageCallback& showStatusMessage) {
    OCR_ORC_DEBUG("=== REDO CALLED ===");
    OCR_ORC_DEBUG("  documentState=" << (documentState ? "exists" : "null"));
    
    if (!documentState) {
        OCR_ORC_DEBUG("  ERROR: documentState is null");
        return;
    }
    
    bool canRedoValue = canRedo();
    OCR_ORC_DEBUG("  canRedo=" << canRedoValue);
    
    if (!canRedoValue) {
        OCR_ORC_DEBUG("  Cannot redo - stack is empty");
        return;
    }
    
    OCR_ORC_DEBUG("  Calling redoAction()...");
    if (redoAction) {
        redoAction();
    }
    OCR_ORC_DEBUG("  redoAction() completed");
    
    // Clear selection before refresh (per current policy)
    if (clearSelection) {
        clearSelection();
    }
    
    // Use CanvasUiSync to refresh all UI components in correct order
    CanvasUiSync::refreshAll(
        documentState,
        [canvas]() {
            if (canvas) {
                canvas->invalidateCoordinateCache();
            }
        },
        updateRegionListBox,
        updateGroupListBox,
        [canvas]() {
            if (canvas) {
                canvas->update();
            }
        },
        [updateUndoRedoButtons, documentState]() {
            if (updateUndoRedoButtons && documentState) {
                bool canUndoAfter = documentState->canUndo();
                bool canRedoAfter = documentState->canRedo();
                updateUndoRedoButtons(canUndoAfter, canRedoAfter);
            }
        }
    );
    
    if (showStatusMessage) {
        showStatusMessage("Redo completed", 2000);
    }
    OCR_ORC_DEBUG("=== REDO COMPLETED ===");
}

void MainWindowUndoRedo::updateButtons(DocumentState* documentState,
                                      const CanUndoCallback& canUndo,
                                      const CanRedoCallback& canRedo,
                                      const UpdateUndoRedoButtonsCallback& updateUndoRedoButtons) {
    if (!documentState || !updateUndoRedoButtons) {
        return;
    }
    
    bool canUndoValue = canUndo();
    bool canRedoValue = canRedo();
    
    updateUndoRedoButtons(canUndoValue, canRedoValue);
}

} // namespace ocr_orc


#include "MainWindowUndoRedoAdapter.h"
#include "../../../models/DocumentState.h"
#include "../../canvas/Canvas.h"
#include "../../canvas/ui/CanvasUiSync.h"
#if OCR_ORC_DEBUG_ENABLED
#include <QtCore/QDebug>
#endif

namespace ocr_orc {

MainWindowUndoRedoAdapter::MainWindowUndoRedoAdapter() {
}

MainWindowUndoRedoAdapter::~MainWindowUndoRedoAdapter() {
}

bool MainWindowUndoRedoAdapter::performUndo(DocumentState* documentState,
                                            Canvas* canvas,
                                            const CanUndoCallback& canUndo,
                                            const UndoActionCallback& undoAction,
                                            const InvalidateCacheCallback& invalidateCache,
                                            const UpdateCanvasCallback& updateCanvas,
                                            const UpdateRegionListBoxCallback& updateRegionListBox,
                                            const UpdateGroupListBoxCallback& updateGroupListBox,
                                            const UpdateUndoRedoButtonsCallback& updateUndoRedoButtons,
                                            const ClearSelectionCallback& clearSelection,
                                            const ShowStatusMessageCallback& showStatusMessage) {
    OCR_ORC_DEBUG("=== UNDO ADAPTER CALLED ===");
    
    if (!documentState) {
        OCR_ORC_DEBUG("  ERROR: documentState is null");
        return false;
    }
    
    // Check if undo is available
    bool canUndoValue = canUndo ? canUndo() : documentState->canUndo();
    if (!canUndoValue) {
        OCR_ORC_DEBUG("  Cannot undo - stack is empty");
        return false;
    }
    
    // Step 1: Perform the undo action
    OCR_ORC_DEBUG("  Calling undoAction()...");
    if (undoAction) {
        undoAction();
    } else {
        documentState->undoAction();
    }
    OCR_ORC_DEBUG("  undoAction() completed");
    
    // Step 2: Clear selection (per current policy)
    if (clearSelection) {
        clearSelection();
    }
    
    // Step 3: Use CanvasUiSync to refresh all UI components in correct order
    // Order: invalidate cache → update lists → repaint canvas → update buttons
    CanvasUiSync::refreshAll(
        documentState,
        invalidateCache ? invalidateCache : [canvas]() {
            if (canvas) {
                canvas->invalidateCoordinateCache();
            }
        },
        updateRegionListBox,
        updateGroupListBox,
        updateCanvas ? updateCanvas : [canvas]() {
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
    
    // Step 4: Show status message
    if (showStatusMessage) {
        showStatusMessage("Undo completed", 2000);
    }
    
    OCR_ORC_DEBUG("=== UNDO ADAPTER COMPLETED ===");
    return true;
}

bool MainWindowUndoRedoAdapter::performRedo(DocumentState* documentState,
                                            Canvas* canvas,
                                            const CanRedoCallback& canRedo,
                                            const RedoActionCallback& redoAction,
                                            const InvalidateCacheCallback& invalidateCache,
                                            const UpdateCanvasCallback& updateCanvas,
                                            const UpdateRegionListBoxCallback& updateRegionListBox,
                                            const UpdateGroupListBoxCallback& updateGroupListBox,
                                            const UpdateUndoRedoButtonsCallback& updateUndoRedoButtons,
                                            const ClearSelectionCallback& clearSelection,
                                            const ShowStatusMessageCallback& showStatusMessage) {
    OCR_ORC_DEBUG("=== REDO ADAPTER CALLED ===");
    
    if (!documentState) {
        OCR_ORC_DEBUG("  ERROR: documentState is null");
        return false;
    }
    
    // Check if redo is available
    bool canRedoValue = canRedo ? canRedo() : documentState->canRedo();
    if (!canRedoValue) {
        OCR_ORC_DEBUG("  Cannot redo - stack is empty");
        return false;
    }
    
    // Step 1: Perform the redo action
    OCR_ORC_DEBUG("  Calling redoAction()...");
    if (redoAction) {
        redoAction();
    } else {
        documentState->redoAction();
    }
    OCR_ORC_DEBUG("  redoAction() completed");
    
    // Step 2: Clear selection (per current policy)
    if (clearSelection) {
        clearSelection();
    }
    
    // Step 3: Use CanvasUiSync to refresh all UI components in correct order
    // Order: invalidate cache → update lists → repaint canvas → update buttons
    CanvasUiSync::refreshAll(
        documentState,
        invalidateCache ? invalidateCache : [canvas]() {
            if (canvas) {
                canvas->invalidateCoordinateCache();
            }
        },
        updateRegionListBox,
        updateGroupListBox,
        updateCanvas ? updateCanvas : [canvas]() {
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
    
    // Step 4: Show status message
    if (showStatusMessage) {
        showStatusMessage("Redo completed", 2000);
    }
    
    OCR_ORC_DEBUG("=== REDO ADAPTER COMPLETED ===");
    return true;
}

} // namespace ocr_orc


#include "MainWindowKeyboardShortcuts.h"
#include "../../MainWindow.h"
#include <QtWidgets/QWidget>
#include <QtGui/QAction>
#include <QtGui/QKeySequence>
#include <QtCore/Qt>

namespace ocr_orc {

MainWindowKeyboardShortcuts::MainWindowKeyboardShortcuts() {
}

MainWindowKeyboardShortcuts::~MainWindowKeyboardShortcuts() {
}

void MainWindowKeyboardShortcuts::setupShortcuts(MainWindow* mainWindow) {
    if (!mainWindow) {
        return;
    }
    
    // Determine modifier key based on platform
    #ifdef Q_OS_MACOS
    Qt::KeyboardModifier MODIFIER = Qt::MetaModifier;  // Cmd on macOS
    #else
    Qt::KeyboardModifier MODIFIER = Qt::ControlModifier;  // Ctrl on Windows/Linux
    #endif
    
    // File Operations
    QAction* loadPdfAction = new QAction(mainWindow);
    loadPdfAction->setShortcut(QKeySequence(MODIFIER | Qt::Key_O));
    loadPdfAction->setShortcutContext(Qt::WindowShortcut);
    QObject::connect(loadPdfAction, SIGNAL(triggered()), mainWindow, SLOT(onLoadPdf()));
    mainWindow->addAction(loadPdfAction);
    
    QAction* exportAction = new QAction(mainWindow);
    exportAction->setShortcut(QKeySequence(MODIFIER | Qt::Key_S));
    exportAction->setShortcutContext(Qt::WindowShortcut);
    QObject::connect(exportAction, SIGNAL(triggered()), mainWindow, SLOT(onExportCoordinates()));
    mainWindow->addAction(exportAction);
    
    QAction* importAction = new QAction(mainWindow);
    importAction->setShortcut(QKeySequence(MODIFIER | Qt::ShiftModifier | Qt::Key_S));
    importAction->setShortcutContext(Qt::WindowShortcut);
    QObject::connect(importAction, SIGNAL(triggered()), mainWindow, SLOT(onLoadCoordinates()));
    mainWindow->addAction(importAction);
    
    QAction* exportMaskAction = new QAction(mainWindow);
    exportMaskAction->setShortcut(QKeySequence(MODIFIER | Qt::Key_M));
    exportMaskAction->setShortcutContext(Qt::WindowShortcut);
    QObject::connect(exportMaskAction, SIGNAL(triggered()), mainWindow, SLOT(onExportMask()));
    mainWindow->addAction(exportMaskAction);
    
    // Mode Switching (1, 2)
    QAction* createModeAction = new QAction(mainWindow);
    createModeAction->setShortcut(QKeySequence("1"));
    createModeAction->setShortcutContext(Qt::WindowShortcut);
    QObject::connect(createModeAction, SIGNAL(triggered()), mainWindow, SLOT(onCreateModeSelected()));
    mainWindow->addAction(createModeAction);
    
    QAction* selectModeAction = new QAction(mainWindow);
    selectModeAction->setShortcut(QKeySequence("2"));
    selectModeAction->setShortcutContext(Qt::WindowShortcut);
    QObject::connect(selectModeAction, SIGNAL(triggered()), mainWindow, SLOT(onSelectModeSelected()));
    mainWindow->addAction(selectModeAction);
    
    // Zoom Operations (handled in Canvas keyPressEvent, but add shortcuts for consistency)
    QAction* zoomInAction = new QAction(mainWindow);
    zoomInAction->setShortcut(QKeySequence(Qt::Key_Plus));
    zoomInAction->setShortcutContext(Qt::WindowShortcut);
    QObject::connect(zoomInAction, SIGNAL(triggered()), mainWindow, SLOT(onZoomIn()));
    mainWindow->addAction(zoomInAction);
    
    QAction* zoomOutAction = new QAction(mainWindow);
    zoomOutAction->setShortcut(QKeySequence(Qt::Key_Minus));
    zoomOutAction->setShortcutContext(Qt::WindowShortcut);
    QObject::connect(zoomOutAction, SIGNAL(triggered()), mainWindow, SLOT(onZoomOut()));
    mainWindow->addAction(zoomOutAction);
    
    QAction* zoomResetAction = new QAction(mainWindow);
    zoomResetAction->setShortcut(QKeySequence(Qt::Key_0));
    zoomResetAction->setShortcutContext(Qt::WindowShortcut);
    QObject::connect(zoomResetAction, SIGNAL(triggered()), mainWindow, SLOT(onZoomReset()));
    mainWindow->addAction(zoomResetAction);
    
    // Region Operations
    QAction* deleteAction = new QAction(mainWindow);
    deleteAction->setShortcut(QKeySequence::Delete);
    deleteAction->setShortcutContext(Qt::WindowShortcut);
    QObject::connect(deleteAction, SIGNAL(triggered()), mainWindow, SLOT(onDeleteSelected()));
    mainWindow->addAction(deleteAction);
    
    QAction* backspaceAction = new QAction(mainWindow);
    backspaceAction->setShortcut(QKeySequence(Qt::Key_Backspace));
    backspaceAction->setShortcutContext(Qt::WindowShortcut);
    QObject::connect(backspaceAction, SIGNAL(triggered()), mainWindow, SLOT(onDeleteSelected()));
    mainWindow->addAction(backspaceAction);
    
    QAction* editAction = new QAction(mainWindow);
    editAction->setShortcut(QKeySequence("E"));
    editAction->setShortcutContext(Qt::WindowShortcut);
    QObject::connect(editAction, SIGNAL(triggered()), mainWindow, SLOT(onEditSelected()));
    mainWindow->addAction(editAction);
    
    // Selection Operations
    QAction* selectAllAction = new QAction(mainWindow);
    selectAllAction->setShortcut(QKeySequence(MODIFIER | Qt::Key_A));
    selectAllAction->setShortcutContext(Qt::WindowShortcut);
    QObject::connect(selectAllAction, SIGNAL(triggered()), mainWindow, SLOT(onSelectAll()));
    mainWindow->addAction(selectAllAction);
    
    QAction* invertSelectionAction = new QAction(mainWindow);
    invertSelectionAction->setShortcut(QKeySequence(MODIFIER | Qt::Key_I));
    invertSelectionAction->setShortcutContext(Qt::WindowShortcut);
    QObject::connect(invertSelectionAction, SIGNAL(triggered()), mainWindow, SLOT(onInvertSelection()));
    mainWindow->addAction(invertSelectionAction);
    
    // Group Operations
    QAction* groupAction = new QAction(mainWindow);
    groupAction->setShortcut(QKeySequence(MODIFIER | Qt::Key_G));
    groupAction->setShortcutContext(Qt::WindowShortcut);
    QObject::connect(groupAction, SIGNAL(triggered()), mainWindow, SLOT(onCreateGroup()));
    mainWindow->addAction(groupAction);
    
    QAction* ungroupAction = new QAction(mainWindow);
    ungroupAction->setShortcut(QKeySequence(MODIFIER | Qt::ShiftModifier | Qt::Key_G));
    ungroupAction->setShortcutContext(Qt::WindowShortcut);
    QObject::connect(ungroupAction, SIGNAL(triggered()), mainWindow, SLOT(onRemoveFromGroup()));
    mainWindow->addAction(ungroupAction);
    
    // Redo alternative (Cmd+Y)
    #ifdef Q_OS_MACOS
    QAction* redoAltAction = new QAction(mainWindow);
    redoAltAction->setShortcut(QKeySequence(Qt::META | Qt::Key_Y));
    redoAltAction->setShortcutContext(Qt::WindowShortcut);
    QObject::connect(redoAltAction, SIGNAL(triggered()), mainWindow, SLOT(onRedo()));
    mainWindow->addAction(redoAltAction);
    #else
    QAction* redoAltAction = new QAction(mainWindow);
    redoAltAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Y));
    redoAltAction->setShortcutContext(Qt::WindowShortcut);
    QObject::connect(redoAltAction, SIGNAL(triggered()), mainWindow, SLOT(onRedo()));
    mainWindow->addAction(redoAltAction);
    #endif
    
    // F1 key for help
    QAction* helpAction = new QAction(mainWindow);
    helpAction->setShortcut(QKeySequence(Qt::Key_F1));
    helpAction->setShortcutContext(Qt::WindowShortcut);
    QObject::connect(helpAction, SIGNAL(triggered()), mainWindow, SLOT(showHelp()));
    mainWindow->addAction(helpAction);
    
    // Magic Detect shortcut (Cmd+M / Ctrl+M)
    QAction* magicDetectAction = new QAction(mainWindow);
    magicDetectAction->setShortcut(QKeySequence(MODIFIER | Qt::Key_M));
    magicDetectAction->setShortcutContext(Qt::WindowShortcut);
    QObject::connect(magicDetectAction, SIGNAL(triggered()), mainWindow, SLOT(onMagicDetect()));
    mainWindow->addAction(magicDetectAction);
}

} // namespace ocr_orc


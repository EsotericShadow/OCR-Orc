#include "MainWindowEventHandlers.h"
#include "../../MainWindow.h"
#include "../../../models/DocumentState.h"
#include "../../canvas/Canvas.h"
#include "../../components/widgets/ControlPanelWidget.h"
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDragMoveEvent>
#include <QtGui/QDropEvent>
#include <QtGui/QCloseEvent>
#include <QtCore/QMimeData>
#include <QtCore/QUrl>
#include <QtCore/QFileInfo>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QStatusBar>
#include <functional>

namespace ocr_orc {

MainWindowEventHandlers::MainWindowEventHandlers() {
}

MainWindowEventHandlers::~MainWindowEventHandlers() {
}

void MainWindowEventHandlers::handleCloseEvent(MainWindow* mainWindow, QCloseEvent* event) {
    if (!mainWindow || !event) {
        return;
    }
    
    // For now, just accept the close event
    // Later, we can add unsaved work checking
    event->accept();
}

void MainWindowEventHandlers::handleDragEnterEvent(MainWindow* mainWindow, QDragEnterEvent* event) {
    if (!mainWindow || !event) {
        return;
    }
    
    // Check if the drag contains URLs (files)
    if (event->mimeData()->hasUrls()) {
        QList<QUrl> urls = event->mimeData()->urls();
        
        // Check if any URL is a PDF file
        for (const QUrl& url : urls) {
            if (url.isLocalFile()) {
                QString filePath = url.toLocalFile();
                QFileInfo fileInfo(filePath);
                if (fileInfo.suffix().toLower() == "pdf") {
                    // Accept the drag if it's a PDF file
                    event->acceptProposedAction();
                    return;
                }
            }
        }
    }
    
    // Reject if not a PDF file
    event->ignore();
}

void MainWindowEventHandlers::handleDragMoveEvent(MainWindow* mainWindow, QDragMoveEvent* event) {
    if (!mainWindow || !event) {
        return;
    }
    
    // Same logic as dragEnterEvent - accept if PDF file
    if (event->mimeData()->hasUrls()) {
        QList<QUrl> urls = event->mimeData()->urls();
        
        for (const QUrl& url : urls) {
            if (url.isLocalFile()) {
                QString filePath = url.toLocalFile();
                QFileInfo fileInfo(filePath);
                if (fileInfo.suffix().toLower() == "pdf") {
                    event->acceptProposedAction();
                    return;
                }
            }
        }
    }
    
    event->ignore();
}

void MainWindowEventHandlers::handleDropEvent(MainWindow* mainWindow,
                                             QDropEvent* event,
                                             DocumentState* documentState,
                                             Canvas* canvas,
                                             ControlPanelWidget* controlPanelWidget,
                                             const std::function<void()>& updateZoomLabel,
                                             const std::function<void()>& updateUndoRedoButtons,
                                             const std::function<void()>& updateRegionListBox,
                                             const std::function<void()>& updateGroupListBox,
                                             const std::function<void(const QString&)>& setFileLabel,
                                             const std::function<void(const QString&, const QString&)>& showWarning) {
    if (!mainWindow || !event || !documentState || !canvas || !controlPanelWidget) {
        return;
    }
    
    // Get the dropped file URLs
    if (!event->mimeData()->hasUrls()) {
        event->ignore();
        return;
    }
    
    QList<QUrl> urls = event->mimeData()->urls();
    
    // Find the first PDF file
    QString pdfPath;
    for (const QUrl& url : urls) {
        if (url.isLocalFile()) {
            QString filePath = url.toLocalFile();
            QFileInfo fileInfo(filePath);
            if (fileInfo.suffix().toLower() == "pdf") {
                pdfPath = filePath;
                break;
            }
        }
    }
    
    if (pdfPath.isEmpty()) {
        if (showWarning) {
            showWarning("Invalid File", "Please drop a PDF file.");
        }
        event->ignore();
        return;
    }
    
    // Accept the drop
    event->acceptProposedAction();
    
    // Load the PDF using file operations module
    mainWindow->fileOperations->handlePdfDrop(
        pdfPath,
        documentState,
        canvas,
        controlPanelWidget,
        mainWindow->statusBar(),
        updateZoomLabel,
        updateUndoRedoButtons,
        updateRegionListBox,
        updateGroupListBox,
        setFileLabel,
        showWarning
    );
}

} // namespace ocr_orc


#include "MainWindowFileOperations.h"
#include "../../../../models/DocumentState.h"
#include "../../../canvas/Canvas.h"
#include "../../../components/widgets/ControlPanelWidget.h"
#include "../../../../export/JsonExporter.h"
#include "../../../../export/CsvExporter.h"
#include "../../../../export/JsonImporter.h"
#include "../../../../export/MaskGenerator.h"
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QStatusBar>
#include <QtCore/QFileInfo>
#include <QtGui/QImage>
#include <QtCore/QString>

namespace ocr_orc {

MainWindowFileOperations::MainWindowFileOperations() {
}

MainWindowFileOperations::~MainWindowFileOperations() {
}

void MainWindowFileOperations::loadPdf(QWidget* parentWidget,
                                      DocumentState* documentState,
                                      Canvas* canvas,
                                      ControlPanelWidget* controlPanelWidget,
                                      QStatusBar* statusBar,
                                      const UpdateZoomLabelCallback& updateZoomLabel,
                                      const UpdateUndoRedoButtonsCallback& updateUndoRedoButtons,
                                      const UpdateRegionListBoxCallback& updateRegionListBox,
                                      const UpdateGroupListBoxCallback& updateGroupListBox,
                                      const SetFileLabelCallback& setFileLabel) {
    // Open file dialog to select PDF
    QString filePath = QFileDialog::getOpenFileName(
        parentWidget,
        "Load PDF Document",
        "",
        "PDF Files (*.pdf);;All Files (*)"
    );
    
    if (filePath.isEmpty()) {
        return; // User cancelled
    }
    
    // Update status bar
    if (statusBar) {
        statusBar->showMessage("Loading PDF...");
    }
    
    // Load PDF in canvas
    bool success = canvas->loadPdf(filePath);
    
    if (success) {
        // Update file label
        QFileInfo fileInfo(filePath);
        if (setFileLabel) {
            setFileLabel(fileInfo.fileName());
        }
        
        // Update document state
        documentState->pdfPath = filePath;
        documentState->image = canvas->hasDocument() ? 
            QImage(canvas->getDocumentImage()) : QImage();
        
        // Clear undo/redo stacks when loading new PDF
        documentState->clearUndoRedoStacks();
        
        // Synchronize coordinates for any existing regions
        documentState->synchronizeCoordinates();
        
        // Update UI
        if (updateZoomLabel) {
            updateZoomLabel();
        }
        if (updateUndoRedoButtons) {
            updateUndoRedoButtons();
        }
        if (updateRegionListBox) {
            updateRegionListBox();
        }
        if (updateGroupListBox) {
            updateGroupListBox();
        }
        
        // Update status bar
        if (statusBar) {
            statusBar->showMessage("PDF loaded successfully", 3000);
        }
    } else {
        // Show error message
        if (parentWidget) {
            QMessageBox::warning(
                parentWidget,
                "Load PDF Error",
                "Failed to load PDF file:\n" + filePath + "\n\nPlease ensure the file is a valid PDF."
            );
        }
        
        // Update status bar
        if (statusBar) {
            statusBar->showMessage("Failed to load PDF", 3000);
        }
    }
}

void MainWindowFileOperations::exportCoordinates(QWidget* parentWidget,
                                                 DocumentState* documentState,
                                                 QStatusBar* statusBar,
                                                 const ShowInformationCallback& showInformation,
                                                 const ShowWarningCallback& showWarning,
                                                 const ShowCriticalCallback& showCritical) {
    if (!documentState) {
        if (showWarning) {
            showWarning("Export", "No document loaded.");
        }
        return;
    }
    
    if (documentState->regions.isEmpty()) {
        if (showWarning) {
            showWarning("Export", "No regions to export.");
        }
        return;
    }
    
    // Show file save dialog
    QString filePath = QFileDialog::getSaveFileName(
        parentWidget,
        "Export Coordinates",
        "",
        "JSON files (*.json);;CSV files (*.csv);;All files (*.*)"
    );
    
    if (filePath.isEmpty()) {
        return; // User cancelled
    }
    
    try {
        // Determine format from extension
        if (filePath.endsWith(".json", Qt::CaseInsensitive)) {
            JsonExporter::exportToFile(*documentState, filePath);
            if (statusBar) {
                statusBar->showMessage(
                    QString("Exported %1 regions to JSON: %2")
                        .arg(documentState->regions.size())
                        .arg(filePath),
                    3000
                );
            }
        } else if (filePath.endsWith(".csv", Qt::CaseInsensitive)) {
            CsvExporter::exportToFile(*documentState, filePath);
            if (statusBar) {
                statusBar->showMessage(
                    QString("Exported %1 regions to CSV: %2")
                        .arg(documentState->regions.size())
                        .arg(filePath),
                    3000
                );
            }
        } else {
            // Default to JSON if no extension
            if (!filePath.endsWith(".json", Qt::CaseInsensitive)) {
                filePath += ".json";
            }
            JsonExporter::exportToFile(*documentState, filePath);
            if (statusBar) {
                statusBar->showMessage(
                    QString("Exported %1 regions to JSON: %2")
                        .arg(documentState->regions.size())
                        .arg(filePath),
                    3000
                );
            }
        }
        
        if (showInformation) {
            showInformation("Export Successful", 
                QString("Successfully exported %1 regions to:\n%2")
                    .arg(documentState->regions.size())
                    .arg(filePath));
        }
                
    } catch (const std::exception& e) {
        if (showCritical) {
            showCritical("Export Error", 
                QString("Failed to export coordinates:\n%1").arg(e.what()));
        }
    }
}

void MainWindowFileOperations::loadCoordinates(QWidget* parentWidget,
                                              DocumentState* documentState,
                                              Canvas* canvas,
                                              QStatusBar* statusBar,
                                               const UpdateRegionListBoxCallback& updateRegionListBox,
                                               const UpdateGroupListBoxCallback& updateGroupListBox,
                                               const UpdateUndoRedoButtonsCallback& updateUndoRedoButtons,
                                               const ZoomResetCallback& zoomReset,
                                               const ShowInformationCallback& showInformation,
                                               const ShowCriticalCallback& showCritical) {
    if (!documentState) {
        if (showCritical) {
            showCritical("Import", "Document state not initialized.");
        }
        return;
    }
    
    // Show file open dialog
    QString filePath = QFileDialog::getOpenFileName(
        parentWidget,
        "Import Coordinates",
        "",
        "JSON files (*.json);;All files (*.*)"
    );
    
    if (filePath.isEmpty()) {
        return; // User cancelled
    }
    
    try {
        // Import JSON
        JsonImporter::importFromFile(*documentState, filePath);
        
        // Update UI
        if (updateRegionListBox) {
            updateRegionListBox();
        }
        if (updateGroupListBox) {
            updateGroupListBox();
        }
        if (canvas) {
            canvas->update();
        }
        if (updateUndoRedoButtons) {
            updateUndoRedoButtons();
        }
        
        // Clear undo/redo stacks after import (fresh start)
        documentState->clearUndoRedoStacks();
        
        // Reset zoom/pan to fit
        if (zoomReset) {
            zoomReset();
        }
        
        if (statusBar) {
            statusBar->showMessage(
                QString("Imported %1 regions from: %2")
                    .arg(documentState->regions.size())
                    .arg(filePath),
                3000
            );
        }
        
        if (showInformation) {
            showInformation("Import Successful", 
                QString("Successfully imported %1 regions from:\n%2")
                    .arg(documentState->regions.size())
                    .arg(filePath));
        }
                
    } catch (const std::exception& e) {
        if (showCritical) {
            showCritical("Import Error", 
                QString("Failed to import coordinates:\n%1").arg(e.what()));
        }
    }
}

void MainWindowFileOperations::exportMask(QWidget* parentWidget,
                                         DocumentState* documentState,
                                         QStatusBar* statusBar,
                                         const ShowInformationCallback& showInformation,
                                         const ShowWarningCallback& showWarning,
                                         const ShowCriticalCallback& showCritical) {
    if (!documentState) {
        if (showWarning) {
            showWarning("Export Mask", "No document loaded.");
        }
        return;
    }
    
    if (documentState->image.isNull()) {
        if (showWarning) {
            showWarning("Export Mask", 
                "No image loaded. Please load a PDF first.");
        }
        return;
    }
    
    if (documentState->regions.isEmpty()) {
        if (showWarning) {
            showWarning("Export Mask", 
                "No regions defined. Please create regions first.");
        }
        return;
    }
    
    // Show file save dialog
    QString filePath = QFileDialog::getSaveFileName(
        parentWidget,
        "Export Mask Image",
        "",
        "PNG files (*.png);;"
        "JPEG files (*.jpg);;"
        "TIFF files (*.tiff);;"
        "All files (*.*)"
    );
    
    if (filePath.isEmpty()) {
        return; // User cancelled
    }
    
    // Ensure file has extension
    QFileInfo fileInfo(filePath);
    if (fileInfo.suffix().isEmpty()) {
        filePath += ".png"; // Default to PNG
    }
    
    try {
        // Generate mask
        MaskGenerator::generate(*documentState, filePath);
        
        // Get JSON path (saved alongside mask)
        QFileInfo maskInfo(filePath);
        QString jsonPath = maskInfo.path() + "/" + maskInfo.baseName() + ".json";
        
        if (statusBar) {
            statusBar->showMessage(
                QString("Exported mask image and coordinates: %1").arg(filePath),
                3000
            );
        }
        
        if (showInformation) {
            showInformation("Export Successful", 
                QString("Successfully exported mask image:\n%1\n\n"
                       "Coordinate JSON saved to:\n%2")
                    .arg(filePath)
                    .arg(jsonPath));
        }
                
    } catch (const std::exception& e) {
        if (showCritical) {
            showCritical("Export Error", 
                QString("Failed to export mask:\n%1").arg(e.what()));
        }
    }
}

bool MainWindowFileOperations::handlePdfDrop(const QString& filePath,
                                             DocumentState* documentState,
                                             Canvas* canvas,
                                             ControlPanelWidget* controlPanelWidget,
                                             QStatusBar* statusBar,
                                             const UpdateZoomLabelCallback& updateZoomLabel,
                                             const UpdateUndoRedoButtonsCallback& updateUndoRedoButtons,
                                             const UpdateRegionListBoxCallback& updateRegionListBox,
                                             const UpdateGroupListBoxCallback& updateGroupListBox,
                                             const SetFileLabelCallback& setFileLabel,
                                             const ShowWarningCallback& showWarning) {
    // Update status bar
    if (statusBar) {
        statusBar->showMessage("Loading PDF...");
    }
    
    // Load PDF in canvas
    bool success = canvas->loadPdf(filePath);
    
    if (success) {
        // Update file label
        QFileInfo fileInfo(filePath);
        if (setFileLabel) {
            setFileLabel(fileInfo.fileName());
        }
        
        // Update document state
        documentState->pdfPath = filePath;
        documentState->image = canvas->hasDocument() ? 
            QImage(canvas->getDocumentImage()) : QImage();
        
        // Clear undo/redo stacks when loading new PDF
        documentState->clearUndoRedoStacks();
        
        // Synchronize coordinates for any existing regions
        documentState->synchronizeCoordinates();
        
        // Update UI
        if (updateZoomLabel) {
            updateZoomLabel();
        }
        if (updateUndoRedoButtons) {
            updateUndoRedoButtons();
        }
        if (updateRegionListBox) {
            updateRegionListBox();
        }
        if (updateGroupListBox) {
            updateGroupListBox();
        }
        
        // Update status bar
        if (statusBar) {
            statusBar->showMessage("PDF loaded successfully", 3000);
        }
        
        return true;
    } else {
        // Show error message
        if (showWarning) {
            showWarning("Load PDF Error",
                "Failed to load PDF file:\n" + filePath + "\n\nPlease ensure the file is a valid PDF.");
        }
        
        // Update status bar
        if (statusBar) {
            statusBar->showMessage("Failed to load PDF", 3000);
        }
        
        return false;
    }
}

} // namespace ocr_orc


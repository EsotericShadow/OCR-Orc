#include "MainWindowActionHandlers.h"
#include "../../../MainWindow.h"
#include "../../../../models/DocumentState.h"
#include "../../../../models/RegionData.h"
#include "../../../canvas/Canvas.h"
#include "../../../components/widgets/ControlPanelWidget.h"
#include "../../../components/dialogs/ExportDialog.h"
#include "../../../../export/JsonExporter.h"
#include "../../../../export/CsvExporter.h"
#include "../../../../export/MaskGenerator.h"
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QMessageBox>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QSet>
#include <QtCore/QStringList>
#include <functional>
#include <stdexcept>

namespace ocr_orc {

MainWindowActionHandlers::MainWindowActionHandlers() {
}

MainWindowActionHandlers::~MainWindowActionHandlers() {
}

void MainWindowActionHandlers::onLoadPdf(MainWindow* mainWindow) {
    if (!mainWindow || !mainWindow->documentState || !mainWindow->canvas || !mainWindow->controlPanelWidget) {
        return;
    }
    
    mainWindow->fileOperations->loadPdf(
        mainWindow,
        mainWindow->documentState,
        mainWindow->canvas,
        mainWindow->controlPanelWidget,
        mainWindow->statusBar(),
        [mainWindow]() { mainWindow->updateZoomLabel(); },
        [mainWindow]() { mainWindow->updateUndoRedoButtons(); },
        [mainWindow]() { mainWindow->updateRegionListBox(); },
        [mainWindow]() { mainWindow->updateGroupListBox(); },
        [mainWindow](const QString& label) { mainWindow->controlPanelWidget->setFileLabel(label); }
    );
}

void MainWindowActionHandlers::onExportCoordinates(MainWindow* mainWindow) {
    if (!mainWindow || !mainWindow->documentState) {
        return;
    }
    
    // Check if document is loaded
    if (mainWindow->documentState->regions.isEmpty()) {
        QMessageBox::warning(mainWindow, "Export", "No regions to export.");
        return;
    }
    
    // Show export dialog
    ExportDialog dialog(mainWindow);
    if (dialog.exec() != QDialog::Accepted) {
        return; // User cancelled
    }
    
    // Get export configuration
    int exportTypes = dialog.getExportTypes();
    QString destDir = dialog.getDestinationDirectory();
    QString baseName = dialog.getBaseFileName();
    bool selectedOnly = dialog.exportSelectedOnly();
    
    // Get regions to export
    QMap<QString, RegionData> regionsToExport;
    if (selectedOnly && mainWindow->canvas) {
        QSet<QString> selectedRegions = mainWindow->canvas->getSelectedRegions();
        if (selectedRegions.isEmpty()) {
            QMessageBox::warning(mainWindow, "Export", "No regions selected. Please select regions or uncheck 'Export selected regions only'.");
            return;
        }
        for (const QString& name : selectedRegions) {
            if (mainWindow->documentState->hasRegion(name)) {
                regionsToExport[name] = mainWindow->documentState->getRegion(name);
            }
        }
    } else {
        regionsToExport = mainWindow->documentState->regions;
    }
    
    if (regionsToExport.isEmpty()) {
        QMessageBox::warning(mainWindow, "Export", "No regions to export.");
        return;
    }
    
    // Create temporary DocumentState for export (if selected only)
    DocumentState exportState;
    if (selectedOnly) {
        exportState = *mainWindow->documentState;
        exportState.regions = regionsToExport;
        exportState.image = mainWindow->documentState->image;
        exportState.pdfPath = mainWindow->documentState->pdfPath;
    } else {
        exportState = *mainWindow->documentState;
    }
    
    QStringList exportedFiles;
    QStringList errors;
    
    // Export coordinates
    if (exportTypes & 1) {
        QString format = dialog.getCoordinateFormat();
        QString extension = format;
        QString filePath = QDir(destDir).filePath(baseName + "." + extension);
        
        try {
            if (format == "json") {
                JsonExporter::exportToFile(exportState, filePath);
            } else if (format == "csv") {
                CsvExporter::exportToFile(exportState, filePath);
            }
            exportedFiles << filePath;
        } catch (const std::exception& e) {
            errors << QString("Failed to export coordinates: %1").arg(e.what());
        }
    }
    
    // Export mask
    if (exportTypes & 2) {
        QString format = dialog.getMaskFormat();
        QString extension = format;
        if (extension == "jpg") extension = "jpeg";
        QString filePath = QDir(destDir).filePath(baseName + "_mask." + extension);
        
        try {
            MaskGenerator::generate(exportState, filePath);
            exportedFiles << filePath;
            
            // Export coordinates JSON with mask if requested
            if (dialog.exportCoordinatesWithMask()) {
                QString jsonPath = QDir(destDir).filePath(baseName + "_mask.json");
                JsonExporter::exportToFile(exportState, jsonPath);
                exportedFiles << jsonPath;
            }
        } catch (const std::exception& e) {
            errors << QString("Failed to export mask: %1").arg(e.what());
        }
    }
    
    // Show results
    if (!errors.isEmpty()) {
        QMessageBox::critical(mainWindow, "Export Errors", 
            "Some exports failed:\n\n" + errors.join("\n"));
    }
    
    if (!exportedFiles.isEmpty()) {
        QString message = QString("Successfully exported %1 file(s):\n\n")
            .arg(exportedFiles.size());
        message += exportedFiles.join("\n");
        
        if (mainWindow->statusBar()) {
            mainWindow->statusBar()->showMessage(
                QString("Exported %1 file(s)").arg(exportedFiles.size()),
                3000
            );
        }
        
        QMessageBox::information(mainWindow, "Export Successful", message);
    }
}

void MainWindowActionHandlers::onLoadCoordinates(MainWindow* mainWindow) {
    if (!mainWindow || !mainWindow->documentState || !mainWindow->canvas) {
        return;
    }
    
    mainWindow->fileOperations->loadCoordinates(
        mainWindow,
        mainWindow->documentState,
        mainWindow->canvas,
        mainWindow->statusBar(),
        [mainWindow]() { mainWindow->updateRegionListBox(); },
        [mainWindow]() { mainWindow->updateGroupListBox(); },
        [mainWindow]() { mainWindow->updateUndoRedoButtons(); },
        [mainWindow]() { if (mainWindow->canvas) mainWindow->canvas->zoomReset(); },
        [mainWindow](const QString& title, const QString& message) {
            QMessageBox::information(mainWindow, title, message);
        },
        [mainWindow](const QString& title, const QString& message) {
            QMessageBox::critical(mainWindow, title, message);
        }
    );
}

void MainWindowActionHandlers::onExportMask(MainWindow* mainWindow) {
    if (!mainWindow || !mainWindow->documentState) {
        return;
    }
    
    mainWindow->fileOperations->exportMask(
        mainWindow,
        mainWindow->documentState,
        mainWindow->statusBar(),
        [mainWindow](const QString& title, const QString& message) {
            QMessageBox::information(mainWindow, title, message);
        },
        [mainWindow](const QString& title, const QString& message) {
            QMessageBox::warning(mainWindow, title, message);
        },
        [mainWindow](const QString& title, const QString& message) {
            QMessageBox::critical(mainWindow, title, message);
        }
    );
}

} // namespace ocr_orc


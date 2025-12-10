#include "CanvasContextMenuBuilder.h"
#include "../CanvasMouseMode.h"
#include <QtWidgets/QMenu>
#include <QtGui/QKeySequence>

namespace ocr_orc {

CanvasContextMenuBuilder::CanvasContextMenuBuilder() {
}

CanvasContextMenuBuilder::~CanvasContextMenuBuilder() {
}

void CanvasContextMenuBuilder::buildContextMenu(QMenu* menu,
                                               DocumentState* documentState,
                                               const QImage& documentImage,
                                               const QPointF& canvasPos,
                                               const QSet<QString>& selectedRegions,
                                               const QString& regionAtPos,
                                               const DuplicateCallback& duplicateSelectedRegions,
                                               const SelectRegionCallback& selectRegion,
                                               const EmitSignalCallback& emitRegionEditRequested,
                                               const EmitSignalCallback& emitRegionColorChangeRequested,
                                               const EmitRegionsDuplicatedCallback& emitRegionsDuplicated,
                                               MouseMode mouseMode,
                                               const std::function<void(const QString&)>& setShapeType) {
    if (!menu || !documentState || documentImage.isNull()) {
        return;
    }
    
    // If in Create mode, add shape type selection
    if (mouseMode == ocr_orc::Create && setShapeType) {
        QMenu* shapeMenu = menu->addMenu("Shape Type");
        
        QAction* rectAction = shapeMenu->addAction("Rectangle");
        rectAction->setCheckable(true);
        QObject::connect(rectAction, &QAction::triggered, [setShapeType]() {
            setShapeType("rect");
        });
        
        QAction* circleAction = shapeMenu->addAction("Circle/Ellipse");
        circleAction->setCheckable(true);
        QObject::connect(circleAction, &QAction::triggered, [setShapeType]() {
            setShapeType("circle");
        });
        
        QAction* triangleAction = shapeMenu->addAction("Triangle");
        triangleAction->setCheckable(true);
        QObject::connect(triangleAction, &QAction::triggered, [setShapeType]() {
            setShapeType("triangle");
        });
        
        QAction* polyAction = shapeMenu->addAction("Polygon");
        polyAction->setCheckable(true);
        polyAction->setEnabled(false); // Not implemented yet
        QObject::connect(polyAction, &QAction::triggered, [setShapeType]() {
            setShapeType("poly");
        });
        
        menu->addSeparator();
    }
    
    // Add duplicate option if regions are selected
    if (!selectedRegions.isEmpty()) {
        QAction* duplicateAction = menu->addAction("Duplicate Selected");
        duplicateAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_D));
        
        QObject::connect(duplicateAction, &QAction::triggered, [duplicateSelectedRegions, emitRegionsDuplicated]() {
            QList<QString> duplicated = duplicateSelectedRegions();
            if (!duplicated.isEmpty()) {
                emitRegionsDuplicated(duplicated);
            }
        });
        
        // Add edit name option for single selection
        if (selectedRegions.size() == 1) {
            menu->addSeparator();
            QAction* editNameAction = menu->addAction("Edit Name");
            QString regionName = *selectedRegions.begin();
            QObject::connect(editNameAction, &QAction::triggered, [emitRegionEditRequested, regionName]() {
                emitRegionEditRequested(regionName);
            });
            
            // Add change color option
            QAction* changeColorAction = menu->addAction("Change Color");
            QObject::connect(changeColorAction, &QAction::triggered, [emitRegionColorChangeRequested, regionName]() {
                emitRegionColorChangeRequested(regionName);
            });
        }
        
        menu->addSeparator();
    }
    
    // If clicking on a region, add option to select it
    if (!regionAtPos.isEmpty() && !selectedRegions.contains(regionAtPos)) {
        QAction* selectAction = menu->addAction("Select Region");
        QObject::connect(selectAction, &QAction::triggered, [selectRegion, regionAtPos]() {
            selectRegion(regionAtPos);
        });
    }
}

} // namespace ocr_orc


#ifndef CANVAS_CONTEXT_MENU_BUILDER_H
#define CANVAS_CONTEXT_MENU_BUILDER_H

#include <QtWidgets/QMenu>
#include <QtCore/QString>
#include <QtCore/QSet>
#include <QtCore/QPointF>
#include <functional>
#include "../../../models/DocumentState.h"
#include "../CanvasMouseMode.h"

namespace ocr_orc {

/**
 * @brief Builds context menus for Canvas
 */
class CanvasContextMenuBuilder {
public:
    // Callback types
    using DuplicateCallback = std::function<QList<QString>()>;
    using SelectRegionCallback = std::function<void(const QString&)>;
    using EmitSignalCallback = std::function<void(const QString&)>;
    using EmitRegionsDuplicatedCallback = std::function<void(const QList<QString>&)>;
    
    CanvasContextMenuBuilder();
    ~CanvasContextMenuBuilder();
    
    void buildContextMenu(QMenu* menu,
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
                         const std::function<void(const QString&)>& setShapeType = nullptr);
};

} // namespace ocr_orc

#endif // CANVAS_CONTEXT_MENU_BUILDER_H


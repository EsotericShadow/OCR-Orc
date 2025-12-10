#ifndef CANVAS_REGION_MANAGER_H
#define CANVAS_REGION_MANAGER_H

#include <QtCore/QString>
#include <QtCore/QSet>
#include <QtCore/QList>
#include <QtCore/QPointF>
#include "../../../models/DocumentState.h"

namespace ocr_orc {

class CanvasRegionOperations;

/**
 * @brief Manages region operations for Canvas (duplicate, move, etc.)
 */
class CanvasRegionManager {
public:
    CanvasRegionManager();
    ~CanvasRegionManager();
    
    QList<QString> duplicateSelectedRegions(DocumentState* documentState,
                                            const QSet<QString>& selectedRegions,
                                            QSet<QString>& updatedSelectedRegions,
                                            QString& primarySelectedRegion);
    
    void moveRegion(const QString& regionName,
                   const QPointF& delta,
                   DocumentState* documentState,
                   const QImage& documentImage,
                   double scaleFactor,
                   const QPointF& imageOffset,
                   CanvasRegionOperations* regionOperations);
    
    void moveSelectedRegions(const QPointF& delta,
                            DocumentState* documentState,
                            const QImage& documentImage,
                            double scaleFactor,
                            const QPointF& imageOffset,
                            const QSet<QString>& selectedRegions,
                            CanvasRegionOperations* regionOperations);
};

} // namespace ocr_orc

#endif // CANVAS_REGION_MANAGER_H


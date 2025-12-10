#ifndef CANVAS_SELECTION_MANAGER_H
#define CANVAS_SELECTION_MANAGER_H

#include <QtCore/QString>
#include <QtCore/QSet>
#include <QtCore/QList>
#include <QtCore/QRectF>
#include "../../../../models/DocumentState.h"
#include "../../../../core/CoordinateSystem.h"

namespace ocr_orc {

/**
 * @brief Manages region selection operations
 */
class CanvasSelectionManager {
public:
    CanvasSelectionManager();
    ~CanvasSelectionManager();
    
    /**
     * @brief Clear all selections
     * @param selectedRegions Output: cleared selection set
     * @param primarySelectedRegion Output: cleared primary selection
     */
    void clearSelection(QSet<QString>& selectedRegions, QString& primarySelectedRegion);
    
    /**
     * @brief Select a single region (clears other selections)
     * @param regionName Name of region to select
     * @param selectedRegions Output: updated selection set
     * @param primarySelectedRegion Output: updated primary selection
     */
    void selectRegion(const QString& regionName,
                     QSet<QString>& selectedRegions,
                     QString& primarySelectedRegion);
    
    /**
     * @brief Toggle selection of a region (for multi-select)
     * @param regionName Name of region to toggle
     * @param selectedRegions Output: updated selection set
     * @param primarySelectedRegion Output: updated primary selection
     */
    void toggleRegionSelection(const QString& regionName,
                              QSet<QString>& selectedRegions,
                              QString& primarySelectedRegion);
    
    /**
     * @brief Add regions to selection (for box selection)
     * @param regionNames Set of region names to add
     * @param selectedRegions Output: updated selection set
     * @param primarySelectedRegion Output: updated primary selection
     */
    void addToSelection(const QSet<QString>& regionNames,
                       QSet<QString>& selectedRegions,
                       QString& primarySelectedRegion);
    
    /**
     * @brief Find all regions intersecting with the given box
     * @param box Selection box rectangle in canvas coordinates
     * @param documentState Document state
     * @param documentImage Document image (for dimensions)
     * @param scaleFactor Current scale factor
     * @param imageOffset Current image offset
     * @return Set of region names that intersect the box
     */
    QSet<QString> findRegionsInBox(const QRectF& box,
                                   DocumentState* documentState,
                                   const QImage& documentImage,
                                   double scaleFactor,
                                   const QPointF& imageOffset) const;
    
    /**
     * @brief Validate selection state consistency
     * Ensures primarySelectedRegion is in selectedRegions and fixes inconsistencies
     * @param selectedRegions Selection set (may be modified)
     * @param primarySelectedRegion Primary selection (may be modified)
     */
    void validateSelection(QSet<QString>& selectedRegions,
                          QString& primarySelectedRegion);
};

} // namespace ocr_orc

#endif // CANVAS_SELECTION_MANAGER_H


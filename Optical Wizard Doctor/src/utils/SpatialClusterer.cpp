#include "SpatialClusterer.h"
#include "RegionDetector.h"  // Full definition needed here
#include <algorithm>
#include <cmath>
#if OCR_ORC_DEBUG_ENABLED
#include <QtCore/QDebug>
#endif

namespace ocr_orc {

SpatialClusterer::SpatialClusterer() {
}

double SpatialClusterer::getCenterY(const DetectedRegion& region) {
    return (region.coords.y1 + region.coords.y2) / 2.0;
}

double SpatialClusterer::getCenterX(const DetectedRegion& region) {
    return (region.coords.x1 + region.coords.x2) / 2.0;
}

bool SpatialClusterer::isHorizontallyAligned(const DetectedRegion& r1, const DetectedRegion& r2, double tolerance) {
    double y1 = getCenterY(r1);
    double y2 = getCenterY(r2);
    return std::abs(y1 - y2) <= tolerance;
}

bool SpatialClusterer::isVerticallyAligned(const DetectedRegion& r1, const DetectedRegion& r2, double tolerance) {
    double x1 = getCenterX(r1);
    double x2 = getCenterX(r2);
    return std::abs(x1 - x2) <= tolerance;
}

void SpatialClusterer::sortByX(QList<DetectedRegion>& regions) {
    std::sort(regions.begin(), regions.end(), 
              [this](const DetectedRegion& a, const DetectedRegion& b) {
                  return getCenterX(a) < getCenterX(b);
              });
}

void SpatialClusterer::sortByY(QList<DetectedRegion>& regions) {
    std::sort(regions.begin(), regions.end(), 
              [this](const DetectedRegion& a, const DetectedRegion& b) {
                  return getCenterY(a) < getCenterY(b);
              });
}

QList<QList<DetectedRegion>> SpatialClusterer::clusterByHorizontalAlignment(
    const QList<DetectedRegion>& regions,
    double tolerance) {
    
    QList<QList<DetectedRegion>> clusters;
    
    // Make a copy to sort
    QList<DetectedRegion> sortedRegions = regions;
    sortByY(sortedRegions);
    
    for (const DetectedRegion& region : sortedRegions) {
        bool addedToCluster = false;
        
        // Try to add to existing cluster
        for (auto& cluster : clusters) {
            if (!cluster.isEmpty()) {
                // Check if aligned with first region in cluster
                if (isHorizontallyAligned(region, cluster.first(), tolerance)) {
                    cluster.append(region);
                    addedToCluster = true;
                    break;
                }
            }
        }
        
        // Create new cluster if not added
        if (!addedToCluster) {
            clusters.append({region});
        }
    }
    
    // Sort each cluster by X-coordinate
    for (auto& cluster : clusters) {
        sortByX(cluster);
    }
    
    #if OCR_ORC_DEBUG_ENABLED
    qDebug() << "[SpatialClusterer] Horizontal alignment: Found" 
             << clusters.size() << "rows from" << regions.size() << "regions";
    #endif
    
    return clusters;
}

QList<QList<DetectedRegion>> SpatialClusterer::clusterByVerticalAlignment(
    const QList<DetectedRegion>& regions,
    double tolerance) {
    
    QList<QList<DetectedRegion>> clusters;
    
    // Make a copy to sort
    QList<DetectedRegion> sortedRegions = regions;
    sortByX(sortedRegions);
    
    for (const DetectedRegion& region : sortedRegions) {
        bool addedToCluster = false;
        
        // Try to add to existing cluster
        for (auto& cluster : clusters) {
            if (!cluster.isEmpty()) {
                // Check if aligned with first region in cluster
                if (isVerticallyAligned(region, cluster.first(), tolerance)) {
                    cluster.append(region);
                    addedToCluster = true;
                    break;
                }
            }
        }
        
        // Create new cluster if not added
        if (!addedToCluster) {
            clusters.append({region});
        }
    }
    
    // Sort each cluster by Y-coordinate
    for (auto& cluster : clusters) {
        sortByY(cluster);
    }
    
    #if OCR_ORC_DEBUG_ENABLED
    qDebug() << "[SpatialClusterer] Vertical alignment: Found" 
             << clusters.size() << "columns from" << regions.size() << "regions";
    #endif
    
    return clusters;
}

struct GridStructure SpatialClusterer::detectGridStructure(const QList<DetectedRegion>& regions) {
    GridStructure grid;
    
    if (regions.isEmpty()) {
        return grid;
    }
    
    // Cluster by horizontal alignment (rows)
    QList<QList<DetectedRegion>> rows = clusterByHorizontalAlignment(regions, 0.01);
    
    if (rows.isEmpty()) {
        return grid;
    }
    
    // Filter rows with similar number of cells (likely grid rows)
    // Find most common row size
    QMap<int, int> rowSizeCounts;
    for (const auto& row : rows) {
        rowSizeCounts[row.size()]++;
    }
    
    int mostCommonSize = 0;
    int maxCount = 0;
    for (auto it = rowSizeCounts.begin(); it != rowSizeCounts.end(); ++it) {
        if (it.value() > maxCount) {
            maxCount = it.value();
            mostCommonSize = it.key();
        }
    }
    
    // Filter rows that match the most common size (within 1 cell tolerance)
    QList<QList<DetectedRegion>> gridRows;
    for (const auto& row : rows) {
        if (std::abs((int)row.size() - mostCommonSize) <= 1) {
            gridRows.append(row);
        }
    }
    
    if (gridRows.isEmpty()) {
        return grid;
    }
    
    grid.rows = gridRows.size();
    grid.cols = mostCommonSize;
    
    // Calculate average cell dimensions
    double totalWidth = 0.0;
    double totalHeight = 0.0;
    int count = 0;
    
    for (const auto& row : gridRows) {
        for (const auto& region : row) {
            totalWidth += (region.coords.x2 - region.coords.x1);
            totalHeight += (region.coords.y2 - region.coords.y1);
            count++;
        }
    }
    
    if (count > 0) {
        grid.cellWidth = totalWidth / count;
        grid.cellHeight = totalHeight / count;
    }
    
    // Build grid structure
    grid.gridCells = gridRows;
    
    // Calculate confidence based on grid regularity
    // More rows with consistent size = higher confidence
    double regularityScore = (double)gridRows.size() / rows.size();
    double sizeConsistencyScore = (double)maxCount / rows.size();
    grid.confidence = (regularityScore + sizeConsistencyScore) / 2.0;
    
    #if OCR_ORC_DEBUG_ENABLED
    qDebug() << "[SpatialClusterer] Grid detected:" << grid.rows << "rows x" 
             << grid.cols << "cols, confidence:" << grid.confidence;
    #endif
    
    return grid;
}

QList<QList<DetectedRegion>> SpatialClusterer::detectSequentialPattern(
    const QList<DetectedRegion>& regions,
    double maxGap) {
    
    QList<QList<DetectedRegion>> sequences;
    
    if (regions.isEmpty()) {
        return sequences;
    }
    
    // Cluster by horizontal alignment first (rows)
    QList<QList<DetectedRegion>> rows = clusterByHorizontalAlignment(regions, 0.01);
    
    for (auto& row : rows) {
        if (row.size() < 2) {
            continue; // Need at least 2 regions for a sequence
        }
        
        sortByX(row);
        
        // Check if regions form a sequence (gaps are within maxGap)
        QList<DetectedRegion> sequence;
        sequence.append(row[0]);
        
        for (int i = 1; i < row.size(); ++i) {
            double gap = getCenterX(row[i]) - getCenterX(row[i-1]) - 
                        (row[i-1].coords.x2 - row[i-1].coords.x1) / 2.0 -
                        (row[i].coords.x2 - row[i].coords.x1) / 2.0;
            
            if (gap <= maxGap) {
                sequence.append(row[i]);
            } else {
                // Gap too large, start new sequence
                if (sequence.size() >= 2) {
                    sequences.append(sequence);
                }
                sequence.clear();
                sequence.append(row[i]);
            }
        }
        
        // Add final sequence
        if (sequence.size() >= 2) {
            sequences.append(sequence);
        }
    }
    
    #if OCR_ORC_DEBUG_ENABLED
    qDebug() << "[SpatialClusterer] Sequential patterns: Found" 
             << sequences.size() << "sequences";
    #endif
    
    return sequences;
}

} // namespace ocr_orc

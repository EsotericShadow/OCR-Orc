#include "PatternAnalyzer.h"
#include <algorithm>
#include <cmath>
#include <QtCore/QMap>

namespace ocr_orc {

PatternAnalyzer::PatternAnalyzer()
    : patternThreshold(0.7)  // 70% threshold
{
}

QString PatternAnalyzer::analyzeCheckboxPattern(const QList<OCRTextRegion>& regions, 
                                                const QList<CheckboxDetection>& checkboxes)
{
    if (regions.isEmpty()) {
        return "unknown";
    }
    
    // Group regions by horizontal alignment
    // Need page height - estimate from max Y coordinate
    int maxY = 0;
    for (const OCRTextRegion& region : regions) {
        maxY = std::max(maxY, region.boundingBox.y + region.boundingBox.height);
    }
    
    if (maxY == 0) {
        return "unknown";
    }
    
    double tolerance = maxY * 0.02;  // 2% of page height
    QList<QList<OCRTextRegion>> horizontalGroups = groupByHorizontalAlignment(regions, tolerance, maxY);
    
    // Analyze pattern for each row
    QList<QString> rowPatterns;
    for (const QList<OCRTextRegion>& row : horizontalGroups) {
        QString pattern = determineRowPattern(row, checkboxes);
        if (pattern != "unknown" && pattern != "mixed") {
            rowPatterns.append(pattern);
        }
    }
    
    if (rowPatterns.isEmpty()) {
        return "unknown";
    }
    
    // Majority voting across all rows
    QMap<QString, int> patternCounts;
    for (const QString& pattern : rowPatterns) {
        patternCounts[pattern]++;
    }
    
    // Find majority pattern
    QString majorityPattern = "unknown";
    int maxCount = 0;
    int totalCount = rowPatterns.size();
    
    for (auto it = patternCounts.begin(); it != patternCounts.end(); ++it) {
        double percentage = static_cast<double>(it.value()) / totalCount;
        if (percentage >= patternThreshold && it.value() > maxCount) {
            maxCount = it.value();
            majorityPattern = it.key();
        }
    }
    
    return majorityPattern;
}

QList<QList<OCRTextRegion>> PatternAnalyzer::groupByHorizontalAlignment(
    const QList<OCRTextRegion>& regions, 
    double tolerance, 
    int /* pageHeight */)
{
    QList<QList<OCRTextRegion>> groups;
    
    if (regions.isEmpty()) {
        return groups;
    }
    
    // Create a copy and sort by Y-coordinate
    QList<OCRTextRegion> sorted = regions;
    std::sort(sorted.begin(), sorted.end(),
        [](const OCRTextRegion& a, const OCRTextRegion& b) {
            int aY = a.boundingBox.y + a.boundingBox.height / 2;
            int bY = b.boundingBox.y + b.boundingBox.height / 2;
            return aY < bY;
        });
    
    // Cluster by Y-coordinate
    for (const OCRTextRegion& region : sorted) {
        int regionY = region.boundingBox.y + region.boundingBox.height / 2;
        bool added = false;
        
        for (auto& group : groups) {
            if (group.isEmpty()) continue;
            
            int groupY = group[0].boundingBox.y + group[0].boundingBox.height / 2;
            if (std::abs(regionY - groupY) < tolerance) {
                group.append(region);
                added = true;
                break;
            }
        }
        
        if (!added) {
            groups.append({region});
        }
    }
    
    // Sort each group by X-coordinate
    for (auto& group : groups) {
        std::sort(group.begin(), group.end(),
            [](const OCRTextRegion& a, const OCRTextRegion& b) {
                return a.boundingBox.x < b.boundingBox.x;
            });
    }
    
    return groups;
}

QString PatternAnalyzer::determineRowPattern(const QList<OCRTextRegion>& row, 
                                             const QList<CheckboxDetection>& checkboxes)
{
    if (row.isEmpty()) {
        return "unknown";
    }
    
    int leftCount = 0;
    int rightCount = 0;
    int totalChecked = 0;
    
    // Analyze first 3 and last 3 items
    int analyzeCount = (row.size() < 3) ? row.size() : 3;
    
    // First items
    for (int i = 0; i < analyzeCount; i++) {
        CheckboxDetection cb = findCheckboxForRegion(row[i], checkboxes);
        if (cb.detected) {
            totalChecked++;
            if (cb.placement == "left") {
                leftCount++;
            } else if (cb.placement == "right") {
                rightCount++;
            }
        }
    }
    
    // Last items
    for (int i = std::max(0, static_cast<int>(row.size()) - analyzeCount); i < static_cast<int>(row.size()); i++) {
        CheckboxDetection cb = findCheckboxForRegion(row[i], checkboxes);
        if (cb.detected) {
            // Don't double-count if already analyzed
            if (i >= analyzeCount) {
                totalChecked++;
                if (cb.placement == "left") {
                    leftCount++;
                } else if (cb.placement == "right") {
                    rightCount++;
                }
            }
        }
    }
    
    if (totalChecked == 0) {
        return "unknown";
    }
    
    // Calculate percentages
    double leftPercent = static_cast<double>(leftCount) / totalChecked;
    double rightPercent = static_cast<double>(rightCount) / totalChecked;
    
    // Determine pattern (>70% threshold)
    if (leftPercent >= patternThreshold) {
        return "checkbox_left";
    } else if (rightPercent >= patternThreshold) {
        return "checkbox_right";
    } else {
        return "mixed";
    }
}

void PatternAnalyzer::applyPatternToAmbiguous(QList<OCRTextRegion>& regions, const QString& pattern)
{
    // This method can be used to apply the detected pattern to regions
    // where checkbox placement is unclear. For now, it's a placeholder
    // that can be enhanced later if needed.
    Q_UNUSED(regions);
    Q_UNUSED(pattern);
}

QList<QList<OCRTextRegion>> PatternAnalyzer::detectMultiItemLines(const QList<OCRTextRegion>& regions, 
                                                                   int pageWidth)
{
    QList<QList<OCRTextRegion>> sequences;
    
    if (regions.isEmpty()) {
        return sequences;
    }
    
    // Group by horizontal alignment first
    int maxY = 0;
    for (const OCRTextRegion& region : regions) {
        maxY = std::max(maxY, region.boundingBox.y + region.boundingBox.height);
    }
    
    double tolerance = maxY * 0.02;  // 2% of page height
    QList<QList<OCRTextRegion>> horizontalGroups = groupByHorizontalAlignment(regions, tolerance, maxY);
    
    // For each horizontal group, detect sequences
    for (const QList<OCRTextRegion>& row : horizontalGroups) {
        if (row.size() < 2) continue;
        
        // Calculate gaps between items
        QList<double> gaps;
        for (int i = 0; i < row.size() - 1; i++) {
            double gap = row[i+1].boundingBox.x - 
                        (row[i].boundingBox.x + row[i].boundingBox.width);
            gaps.append(gap);
        }
        
        // Check if gaps are small (< 5% of page width)
        double maxGap = pageWidth * 0.05;
        bool isSequence = true;
        for (double gap : gaps) {
            if (gap > maxGap) {
                isSequence = false;
                break;
            }
        }
        
        if (isSequence) {
            sequences.append(row);
        }
    }
    
    return sequences;
}

double PatternAnalyzer::getPatternMatchScore(const OCRTextRegion& region, const QString& pattern)
{
    // This is a placeholder - in practice, this would check if the region
    // matches the detected pattern (e.g., has checkbox on left if pattern is "checkbox_left")
    // For now, return a default score
    Q_UNUSED(region);
    Q_UNUSED(pattern);
    
    // If pattern is detected, return high score
    if (pattern != "unknown" && pattern != "mixed") {
        return 1.0;
    }
    
    return 0.5;  // Ambiguous score
}

CheckboxDetection PatternAnalyzer::findCheckboxForRegion(const OCRTextRegion& region, 
                                                         const QList<CheckboxDetection>& checkboxes)
{
    // Find checkbox that overlaps or is near this region
    // Simple approach: check if checkbox center is near text region
    cv::Rect textBox = region.boundingBox;
    int textCenterX = textBox.x + textBox.width / 2;
    int textCenterY = textBox.y + textBox.height / 2;
    
    CheckboxDetection bestMatch;
    double bestDistance = std::numeric_limits<double>::max();
    
    for (const CheckboxDetection& cb : checkboxes) {
        if (!cb.detected) continue;
        
        int cbCenterX = cb.boundingBox.x + cb.boundingBox.width / 2;
        int cbCenterY = cb.boundingBox.y + cb.boundingBox.height / 2;
        
        // Calculate distance
        double distance = std::sqrt(
            std::pow(cbCenterX - textCenterX, 2) + 
            std::pow(cbCenterY - textCenterY, 2)
        );
        
        // Check if within reasonable distance (2x text height)
        if (distance < textBox.height * 2 && distance < bestDistance) {
            bestDistance = distance;
            bestMatch = cb;
        }
    }
    
    return bestMatch;
}

} // namespace ocr_orc

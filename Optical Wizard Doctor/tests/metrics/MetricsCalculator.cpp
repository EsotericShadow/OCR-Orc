#include "MetricsCalculator.h"
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QDateTime>
#include <algorithm>
#include <cmath>

// #region agent log
static void debugLog(const QString& location, const QString& message, const QVariantMap& data = QVariantMap()) {
    QFile logFile("/Users/main/Desktop/OWD-OCR_Coordinates/.cursor/debug.log");
    if (logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&logFile);
        QJsonObject logObj;
        logObj["timestamp"] = QDateTime::currentMSecsSinceEpoch();
        logObj["location"] = location;
        logObj["message"] = message;
        QJsonObject dataObj;
        for (auto it = data.begin(); it != data.end(); ++it) {
            dataObj[it.key()] = QJsonValue::fromVariant(it.value());
        }
        logObj["data"] = dataObj;
        logObj["sessionId"] = "debug-session";
        out << QJsonDocument(logObj).toJson(QJsonDocument::Compact) << "\n";
        logFile.close();
    }
}
// #endregion

namespace ocr_orc {

MetricsCalculator::MetricsCalculator()
{
}

EvaluationMetrics MetricsCalculator::calculateMetrics(const QList<DetectedRegion>& detectedRegions,
                                                     const GroundTruthAnnotation& groundTruth,
                                                     int imageWidth, int imageHeight) const
{
    EvaluationMetrics metrics;
    
    // Match regions with ground truth
    QList<RegionEvaluation> evaluations = matchRegions(detectedRegions, groundTruth,
                                                       imageWidth, imageHeight);
    metrics.regionEvaluations = evaluations;
    
    // Count true positives, false positives, false negatives
    int truePositives = 0;
    int falsePositives = 0;
    int falseNegatives = 0;
    double totalIoU = 0.0;
    int matchedCount = 0;
    
    for (const RegionEvaluation& eval : evaluations) {
        if (eval.isTruePositive) {
            truePositives++;
            totalIoU += eval.iou;
            matchedCount++;
        } else if (eval.isFalsePositive) {
            falsePositives++;
        } else if (eval.isFalseNegative) {
            falseNegatives++;
        }
    }
    
    // Calculate overall metrics
    metrics.precision = calculatePrecision(truePositives, falsePositives);
    metrics.recall = calculateRecall(truePositives, falseNegatives);
    metrics.f1Score = calculateF1Score(metrics.precision, metrics.recall);
    metrics.meanIoU = (matchedCount > 0) ? totalIoU / matchedCount : 0.0;
    
    return metrics;
}

double MetricsCalculator::calculateIoU(const cv::Rect& rect1, const cv::Rect& rect2)
{
    // Calculate intersection
    int x1 = std::max(rect1.x, rect2.x);
    int y1 = std::max(rect1.y, rect2.y);
    int x2 = std::min(rect1.x + rect1.width, rect2.x + rect2.width);
    int y2 = std::min(rect1.y + rect1.height, rect2.y + rect2.height);
    
    if (x2 <= x1 || y2 <= y1) {
        return 0.0;  // No intersection
    }
    
    int intersectionArea = (x2 - x1) * (y2 - y1);
    int area1 = rect1.width * rect1.height;
    int area2 = rect2.width * rect2.height;
    int unionArea = area1 + area2 - intersectionArea;
    
    if (unionArea == 0) {
        return 0.0;
    }
    
    return static_cast<double>(intersectionArea) / unionArea;
}

double MetricsCalculator::calculateIoU(const NormalizedCoords& coords1, const NormalizedCoords& coords2)
{
    // Calculate intersection in normalized space
    double x1 = std::max(coords1.x1, coords2.x1);
    double y1 = std::max(coords1.y1, coords2.y1);
    double x2 = std::min(coords1.x2, coords2.x2);
    double y2 = std::min(coords1.y2, coords2.y2);
    
    if (x2 <= x1 || y2 <= y1) {
        return 0.0;  // No intersection
    }
    
    double intersectionArea = (x2 - x1) * (y2 - y1);
    double area1 = (coords1.x2 - coords1.x1) * (coords1.y2 - coords1.y1);
    double area2 = (coords2.x2 - coords2.x1) * (coords2.y2 - coords2.y1);
    double unionArea = area1 + area2 - intersectionArea;
    
    if (unionArea == 0) {
        return 0.0;
    }
    
    return intersectionArea / unionArea;
}

QList<RegionEvaluation> MetricsCalculator::matchRegions(const QList<DetectedRegion>& detectedRegions,
                                                        const GroundTruthAnnotation& groundTruth,
                                                        int imageWidth, int imageHeight,
                                                        double iouThreshold) const
{
    // #region agent log
    debugLog("MetricsCalculator.cpp:matchRegions", "Entry", {{"detectedCount", detectedRegions.size()}, {"expectedCount", groundTruth.expectedRegions.size()}, {"imageWidth", imageWidth}, {"imageHeight", imageHeight}});
    // #endregion
    
    QList<RegionEvaluation> evaluations;
    QList<bool> gtMatched(groundTruth.expectedRegions.size(), false);
    
    // #region agent log
    debugLog("MetricsCalculator.cpp:matchRegions", "Starting matching loop", {{"detectedRegions", detectedRegions.size()}, {"gtMatchedSize", gtMatched.size()}});
    // #endregion
    
    // Match each detected region with ground truth
    int detectedIndex = 0;
    for (const DetectedRegion& detected : detectedRegions) {
        detectedIndex++;
        if (detectedIndex % 50 == 0 || detectedIndex == 1) {
            // #region agent log
            debugLog("MetricsCalculator.cpp:matchRegions", "Processing detected region", {{"index", detectedIndex}, {"total", detectedRegions.size()}});
            // #endregion
        }
        RegionEvaluation eval;
        eval.detectedBox = detected.boundingBox;
        eval.confidence = detected.confidence;
        
        double bestIoU = 0.0;
        int bestGtIndex = -1;
        
        // Find best matching ground truth region
        for (int i = 0; i < groundTruth.expectedRegions.size(); ++i) {
            if (gtMatched[i]) {
                continue;  // Already matched
            }
            
            const GroundTruthRegion& gt = groundTruth.expectedRegions[i];
            cv::Rect gtRect = normalizedToImageRect(gt.normalizedCoords, imageWidth, imageHeight);
            
            double iou = calculateIoU(detected.boundingBox, gtRect);
            if (iou > bestIoU) {
                bestIoU = iou;
                bestGtIndex = i;
            }
        }
        
        if (bestGtIndex >= 0 && bestIoU >= iouThreshold) {
            // True positive
            eval.isTruePositive = true;
            eval.iou = bestIoU;
            const GroundTruthRegion& gt = groundTruth.expectedRegions[bestGtIndex];
            eval.regionName = gt.name;
            eval.groundTruthBox = normalizedToImageRect(gt.normalizedCoords, imageWidth, imageHeight);
            eval.positionAccuracy = calculatePositionAccuracy(detected.coords, gt.normalizedCoords);
            eval.sizeAccuracy = calculateSizeAccuracy(detected.coords, gt.normalizedCoords);
            gtMatched[bestGtIndex] = true;
        } else {
            // False positive
            eval.isFalsePositive = true;
            eval.iou = bestIoU;
        }
        
        evaluations.append(eval);
    }
    
    // Add false negatives (unmatched ground truth regions)
    for (int i = 0; i < groundTruth.expectedRegions.size(); ++i) {
        if (!gtMatched[i]) {
            RegionEvaluation eval;
            eval.isFalseNegative = true;
            const GroundTruthRegion& gt = groundTruth.expectedRegions[i];
            eval.regionName = gt.name;
            eval.groundTruthBox = normalizedToImageRect(gt.normalizedCoords, imageWidth, imageHeight);
            eval.iou = 0.0;
            evaluations.append(eval);
        }
    }
    
    return evaluations;
}

double MetricsCalculator::calculatePrecision(int truePositives, int falsePositives)
{
    int total = truePositives + falsePositives;
    if (total == 0) {
        return 0.0;
    }
    return static_cast<double>(truePositives) / total;
}

double MetricsCalculator::calculateRecall(int truePositives, int falseNegatives)
{
    int total = truePositives + falseNegatives;
    if (total == 0) {
        return 0.0;
    }
    return static_cast<double>(truePositives) / total;
}

double MetricsCalculator::calculateF1Score(double precision, double recall)
{
    if (precision + recall == 0.0) {
        return 0.0;
    }
    return 2.0 * (precision * recall) / (precision + recall);
}

double MetricsCalculator::calculatePositionAccuracy(const NormalizedCoords& detectedCoords,
                                                    const NormalizedCoords& groundTruthCoords)
{
    // Calculate center points
    double detectedCenterX = (detectedCoords.x1 + detectedCoords.x2) / 2.0;
    double detectedCenterY = (detectedCoords.y1 + detectedCoords.y2) / 2.0;
    double gtCenterX = (groundTruthCoords.x1 + groundTruthCoords.x2) / 2.0;
    double gtCenterY = (groundTruthCoords.y1 + groundTruthCoords.y2) / 2.0;
    
    // Calculate Euclidean distance
    double dx = detectedCenterX - gtCenterX;
    double dy = detectedCenterY - gtCenterY;
    double distance = std::sqrt(dx * dx + dy * dy);
    
    // Normalize by diagonal of unit square (maximum possible distance)
    double maxDistance = std::sqrt(2.0);
    return std::min(distance / maxDistance, 1.0);
}

double MetricsCalculator::calculateSizeAccuracy(const NormalizedCoords& detectedCoords,
                                                const NormalizedCoords& groundTruthCoords)
{
    // Calculate widths and heights
    double detectedWidth = detectedCoords.x2 - detectedCoords.x1;
    double detectedHeight = detectedCoords.y2 - detectedCoords.y1;
    double gtWidth = groundTruthCoords.x2 - groundTruthCoords.x1;
    double gtHeight = groundTruthCoords.y2 - groundTruthCoords.y1;
    
    if (gtWidth == 0.0 || gtHeight == 0.0) {
        return 0.0;
    }
    
    // Calculate ratios
    double widthRatio = std::min(detectedWidth / gtWidth, gtWidth / detectedWidth);
    double heightRatio = std::min(detectedHeight / gtHeight, gtHeight / detectedHeight);
    
    // Average ratio (1.0 = perfect match)
    return (widthRatio + heightRatio) / 2.0;
}

cv::Rect MetricsCalculator::normalizedToImageRect(const NormalizedCoords& coords, int width, int height) const
{
    ImageCoords imgCoords = CoordinateSystem::normalizedToImage(coords, width, height);
    return cv::Rect(imgCoords.x1, imgCoords.y1,
                    imgCoords.x2 - imgCoords.x1,
                    imgCoords.y2 - imgCoords.y1);
}

NormalizedCoords MetricsCalculator::imageRectToNormalized(const cv::Rect& rect, int width, int height) const
{
    ImageCoords imgCoords(rect.x, rect.y, rect.x + rect.width, rect.y + rect.height);
    return CoordinateSystem::imageToNormalized(imgCoords, width, height);
}

} // namespace ocr_orc

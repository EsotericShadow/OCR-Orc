#ifndef METRICS_CALCULATOR_H
#define METRICS_CALCULATOR_H

#include "../TestDataManager.h"
#include "../../src/utils/RegionDetector.h"
#include "../../src/core/CoordinateSystem.h"
#include "../instrumentation/PipelineInstrumentation.h"
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <opencv2/opencv.hpp>

namespace ocr_orc {

/**
 * @brief Evaluation result for a single region
 */
struct RegionEvaluation {
    QString regionName;
    bool isTruePositive;
    bool isFalsePositive;
    bool isFalseNegative;
    double iou;  // Intersection over Union with ground truth
    double positionAccuracy;  // Distance from ground truth center (normalized)
    double sizeAccuracy;  // Width/height ratio vs ground truth
    double confidence;
    cv::Rect detectedBox;
    cv::Rect groundTruthBox;
    
    RegionEvaluation()
        : isTruePositive(false), isFalsePositive(false), isFalseNegative(false),
          iou(0.0), positionAccuracy(0.0), sizeAccuracy(0.0), confidence(0.0) {}
};

/**
 * @brief Performance metrics
 */
struct PerformanceMetrics {
    double totalTimeMs;
    QMap<QString, double> stageTimings;
    QMap<QString, int> cacheHits;
    QMap<QString, int> cacheMisses;
    double memoryUsageMB;
    
    PerformanceMetrics()
        : totalTimeMs(0.0), memoryUsageMB(0.0) {}
};

/**
 * @brief Threshold effectiveness analysis
 */
struct ThresholdEffectiveness {
    QString thresholdName;
    double value;
    double impactOnPrecision;
    double impactOnRecall;
    int timesUsed;
    
    ThresholdEffectiveness()
        : impactOnPrecision(0.0), impactOnRecall(0.0), timesUsed(0) {}
};

/**
 * @brief Comprehensive evaluation metrics
 */
struct EvaluationMetrics {
    // Overall metrics
    double precision;
    double recall;
    double f1Score;
    double meanIoU;
    
    // Per-stage metrics
    QMap<QString, StageMetrics> stageMetrics;
    
    // Per-region details
    QList<RegionEvaluation> regionEvaluations;
    
    // Performance metrics
    PerformanceMetrics performance;
    
    // Threshold analysis
    QMap<QString, ThresholdEffectiveness> thresholdAnalysis;
    
    EvaluationMetrics()
        : precision(0.0), recall(0.0), f1Score(0.0), meanIoU(0.0) {}
};

/**
 * @brief Metrics calculator for comprehensive evaluation
 */
class MetricsCalculator {
public:
    MetricsCalculator();
    ~MetricsCalculator() = default;
    
    /**
     * @brief Calculate evaluation metrics comparing detected regions with ground truth
     * @param detectedRegions Detected regions from pipeline
     * @param groundTruth Ground truth annotation
     * @param imageWidth Image width in pixels
     * @param imageHeight Image height in pixels
     * @return Evaluation metrics
     */
    EvaluationMetrics calculateMetrics(const QList<DetectedRegion>& detectedRegions,
                                      const GroundTruthAnnotation& groundTruth,
                                      int imageWidth, int imageHeight) const;
    
    /**
     * @brief Calculate IoU between two rectangles
     * @param rect1 First rectangle
     * @param rect2 Second rectangle
     * @return IoU value (0.0-1.0)
     */
    static double calculateIoU(const cv::Rect& rect1, const cv::Rect& rect2);
    
    /**
     * @brief Calculate IoU between normalized coordinates
     * @param coords1 First normalized coordinates
     * @param coords2 Second normalized coordinates
     * @return IoU value (0.0-1.0)
     */
    static double calculateIoU(const NormalizedCoords& coords1, const NormalizedCoords& coords2);
    
    /**
     * @brief Match detected regions with ground truth regions
     * @param detectedRegions Detected regions
     * @param groundTruth Ground truth annotation
     * @param imageWidth Image width
     * @param imageHeight Image height
     * @param iouThreshold IoU threshold for matching (default: 0.5)
     * @return List of region evaluations
     */
    QList<RegionEvaluation> matchRegions(const QList<DetectedRegion>& detectedRegions,
                                         const GroundTruthAnnotation& groundTruth,
                                         int imageWidth, int imageHeight,
                                         double iouThreshold = 0.5) const;
    
    /**
     * @brief Calculate precision
     * @param truePositives Number of true positives
     * @param falsePositives Number of false positives
     * @return Precision value (0.0-1.0)
     */
    static double calculatePrecision(int truePositives, int falsePositives);
    
    /**
     * @brief Calculate recall
     * @param truePositives Number of true positives
     * @param falseNegatives Number of false negatives
     * @return Recall value (0.0-1.0)
     */
    static double calculateRecall(int truePositives, int falseNegatives);
    
    /**
     * @brief Calculate F1 score
     * @param precision Precision value
     * @param recall Recall value
     * @return F1 score (0.0-1.0)
     */
    static double calculateF1Score(double precision, double recall);
    
    /**
     * @brief Calculate position accuracy (distance from ground truth center)
     * @param detectedCoords Detected normalized coordinates
     * @param groundTruthCoords Ground truth normalized coordinates
     * @return Normalized distance (0.0 = perfect match, 1.0 = maximum distance)
     */
    static double calculatePositionAccuracy(const NormalizedCoords& detectedCoords,
                                           const NormalizedCoords& groundTruthCoords);
    
    /**
     * @brief Calculate size accuracy (width/height ratio vs ground truth)
     * @param detectedCoords Detected normalized coordinates
     * @param groundTruthCoords Ground truth normalized coordinates
     * @return Size accuracy (1.0 = perfect match, 0.0 = no match)
     */
    static double calculateSizeAccuracy(const NormalizedCoords& detectedCoords,
                                       const NormalizedCoords& groundTruthCoords);

private:
    cv::Rect normalizedToImageRect(const NormalizedCoords& coords, int width, int height) const;
    NormalizedCoords imageRectToNormalized(const cv::Rect& rect, int width, int height) const;
};

} // namespace ocr_orc

#endif // METRICS_CALCULATOR_H

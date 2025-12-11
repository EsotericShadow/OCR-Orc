#ifndef CONFIDENCE_CALCULATOR_H
#define CONFIDENCE_CALCULATOR_H

#include "RegionDetector.h"
#include <QtCore/QList>

namespace ocr_orc {

/**
 * @brief Multi-factor confidence calculator
 * 
 * Combines multiple confidence sources (OCR, line detection, rectangularity, pattern)
 * into a single confidence score using weighted averaging.
 */
class ConfidenceCalculator {
public:
    ConfidenceCalculator();
    ~ConfidenceCalculator() = default;
    
    /**
     * @brief Calculate combined confidence score
     * @param ocrConf OCR confidence (0.0-100.0, will be normalized to 0.0-1.0)
     * @param lineScore Line detection score (0.0-1.0)
     * @param rectScore Rectangularity score (0.0-1.0)
     * @param patternScore Pattern match score (0.0-1.0)
     * @return Combined confidence (0.0-1.0)
     */
    static double calculate(double ocrConf, double lineScore, double rectScore, double patternScore);
    
    /**
     * @brief Filter regions by confidence threshold
     * @param regions List of detected regions
     * @param minConf Minimum confidence threshold (0.0-1.0)
     * @return Filtered list of regions
     */
    static QList<DetectedRegion> filterRegions(const QList<DetectedRegion>& regions, double minConf);
    
    /**
     * @brief Set confidence weights
     * @param ocrWeight Weight for OCR confidence (default: 0.4)
     * @param lineWeight Weight for line detection (default: 0.3)
     * @param rectWeight Weight for rectangularity (default: 0.2)
     * @param patternWeight Weight for pattern match (default: 0.1)
     */
    static void setWeights(double ocrWeight, double lineWeight, double rectWeight, double patternWeight);
    
    /**
     * @brief Get current weights
     * @param ocrWeight Output: OCR weight
     * @param lineWeight Output: Line detection weight
     * @param rectWeight Output: Rectangularity weight
     * @param patternWeight Output: Pattern match weight
     */
    static void getWeights(double& ocrWeight, double& lineWeight, double& rectWeight, double& patternWeight);
    
    /**
     * @brief Set filtering thresholds
     * @param highThreshold High confidence threshold (default: 0.7)
     * @param mediumThreshold Medium confidence threshold (default: 0.5)
     */
    static void setThresholds(double highThreshold, double mediumThreshold);
    
    /**
     * @brief Get current thresholds
     * @param highThreshold Output: High confidence threshold
     * @param mediumThreshold Output: Medium confidence threshold
     */
    static void getThresholds(double& highThreshold, double& mediumThreshold);

private:
    static double ocrWeight;      // Weight for OCR confidence (default: 0.4)
    static double lineWeight;      // Weight for line detection (default: 0.3)
    static double rectWeight;      // Weight for rectangularity (default: 0.2)
    static double patternWeight;   // Weight for pattern match (default: 0.1)
    
    static double highThreshold;   // High confidence threshold (default: 0.7)
    static double mediumThreshold;  // Medium confidence threshold (default: 0.5)
};

} // namespace ocr_orc

#endif // CONFIDENCE_CALCULATOR_H

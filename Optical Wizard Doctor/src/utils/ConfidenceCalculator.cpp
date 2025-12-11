#include "ConfidenceCalculator.h"
#include <algorithm>

namespace ocr_orc {

// Static member initialization
double ConfidenceCalculator::ocrWeight = 0.4;
double ConfidenceCalculator::lineWeight = 0.3;
double ConfidenceCalculator::rectWeight = 0.2;
double ConfidenceCalculator::patternWeight = 0.1;

double ConfidenceCalculator::highThreshold = 0.7;
double ConfidenceCalculator::mediumThreshold = 0.5;

ConfidenceCalculator::ConfidenceCalculator()
{
}

double ConfidenceCalculator::calculate(double ocrConf, double lineScore, double rectScore, double patternScore)
{
    // Normalize OCR confidence from 0-100 to 0-1
    double normalizedOcrConf = ocrConf / 100.0;
    
    // Clamp all scores to 0-1 range
    normalizedOcrConf = std::clamp(normalizedOcrConf, 0.0, 1.0);
    lineScore = std::clamp(lineScore, 0.0, 1.0);
    rectScore = std::clamp(rectScore, 0.0, 1.0);
    patternScore = std::clamp(patternScore, 0.0, 1.0);
    
    // Weighted average
    double confidence = 
        (normalizedOcrConf * ocrWeight) +
        (lineScore * lineWeight) +
        (rectScore * rectWeight) +
        (patternScore * patternWeight);
    
    // Normalize weights (in case they don't sum to 1.0)
    double totalWeight = ocrWeight + lineWeight + rectWeight + patternWeight;
    if (totalWeight > 0.0) {
        confidence /= totalWeight;
    }
    
    return std::clamp(confidence, 0.0, 1.0);
}

QList<DetectedRegion> ConfidenceCalculator::filterRegions(const QList<DetectedRegion>& regions, double minConf)
{
    QList<DetectedRegion> filtered;
    
    for (const DetectedRegion& region : regions) {
        // High confidence: always include
        if (region.confidence >= highThreshold) {
            filtered.append(region);
        }
        // Medium confidence: include if above minimum threshold
        else if (region.confidence >= mediumThreshold && region.confidence >= minConf) {
            filtered.append(region);
        }
        // Low confidence: only include if explicitly requested (minConf < mediumThreshold)
        else if (region.confidence >= minConf && minConf < mediumThreshold) {
            filtered.append(region);
        }
    }
    
    return filtered;
}

void ConfidenceCalculator::setWeights(double ocrWeight, double lineWeight, double rectWeight, double patternWeight)
{
    ConfidenceCalculator::ocrWeight = ocrWeight;
    ConfidenceCalculator::lineWeight = lineWeight;
    ConfidenceCalculator::rectWeight = rectWeight;
    ConfidenceCalculator::patternWeight = patternWeight;
}

void ConfidenceCalculator::getWeights(double& ocrWeight, double& lineWeight, double& rectWeight, double& patternWeight)
{
    ocrWeight = ConfidenceCalculator::ocrWeight;
    lineWeight = ConfidenceCalculator::lineWeight;
    rectWeight = ConfidenceCalculator::rectWeight;
    patternWeight = ConfidenceCalculator::patternWeight;
}

void ConfidenceCalculator::setThresholds(double highThreshold, double mediumThreshold)
{
    ConfidenceCalculator::highThreshold = highThreshold;
    ConfidenceCalculator::mediumThreshold = mediumThreshold;
}

void ConfidenceCalculator::getThresholds(double& highThreshold, double& mediumThreshold)
{
    highThreshold = ConfidenceCalculator::highThreshold;
    mediumThreshold = ConfidenceCalculator::mediumThreshold;
}

} // namespace ocr_orc

#include "TypeInferencer.h"
#include <opencv2/imgproc.hpp>
#include <cmath>
#if OCR_ORC_DEBUG_ENABLED
#include <QtCore/QDebug>
#endif

namespace ocr_orc {

TypeInferencer::TypeInferencer() {
}

double TypeInferencer::analyzeEdgeDensity(const cv::Mat& regionImage) {
    if (regionImage.empty()) {
        return 0.0;
    }
    
    cv::Mat gray;
    if (regionImage.channels() == 3) {
        cv::cvtColor(regionImage, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = regionImage.clone();
    }
    
    // Detect edges using Canny
    cv::Mat edges;
    cv::Canny(gray, edges, 50, 150);
    
    // Calculate edge density (percentage of edge pixels)
    int edgePixels = cv::countNonZero(edges);
    int totalPixels = edges.rows * edges.cols;
    
    return totalPixels > 0 ? (double)edgePixels / totalPixels : 0.0;
}

double TypeInferencer::analyzeTexture(const cv::Mat& regionImage) {
    if (regionImage.empty()) {
        return 0.0;
    }
    
    cv::Mat gray;
    if (regionImage.channels() == 3) {
        cv::cvtColor(regionImage, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = regionImage.clone();
    }
    
    // Calculate standard deviation (texture measure)
    cv::Scalar mean, stddev;
    cv::meanStdDev(gray, mean, stddev);
    
    return stddev[0]; // Return standard deviation as texture measure
}

bool TypeInferencer::appearsAsLetterField(const cv::Mat& regionImage, double aspectRatio) {
    // Letter fields often have:
    // - Wider aspect ratio (typically > 1.0 for horizontal text)
    // - Higher edge density (letters have more edges)
    // - More texture variation
    
    double edgeDensity = analyzeEdgeDensity(regionImage);
    double texture = analyzeTexture(regionImage);
    
    // Heuristics for letter fields
    bool wideAspect = aspectRatio > 0.8; // Not too narrow
    bool highEdgeDensity = edgeDensity > 0.1; // Some edges present
    bool hasTexture = texture > 10.0; // Some texture variation
    
    return wideAspect && (highEdgeDensity || hasTexture);
}

bool TypeInferencer::appearsAsNumberField(const cv::Mat& regionImage, double aspectRatio) {
    // Number fields often have:
    // - More compact aspect ratio (can be square-ish)
    // - Different edge patterns (numbers are simpler)
    // - Less texture variation
    
    double edgeDensity = analyzeEdgeDensity(regionImage);
    double texture = analyzeTexture(regionImage);
    
    // Heuristics for number fields
    bool compactAspect = aspectRatio > 0.5 && aspectRatio < 2.0; // Reasonable range
    bool moderateEdgeDensity = edgeDensity > 0.05 && edgeDensity < 0.2; // Some edges but not too many
    bool lessTexture = texture < 30.0; // Less texture variation
    
    return compactAspect && moderateEdgeDensity && lessTexture;
}

QString TypeInferencer::inferTypeFromVisual(const cv::Mat& regionImage, double aspectRatio) {
    if (regionImage.empty()) {
        return "unknown";
    }
    
    bool looksLikeLetters = appearsAsLetterField(regionImage, aspectRatio);
    bool looksLikeNumbers = appearsAsNumberField(regionImage, aspectRatio);
    
    if (looksLikeLetters && !looksLikeNumbers) {
        return "letters";
    } else if (looksLikeNumbers && !looksLikeLetters) {
        return "numbers";
    } else if (looksLikeLetters && looksLikeNumbers) {
        return "mixed";
    } else {
        return "unknown";
    }
}

QString TypeInferencer::inferTypeFromPattern(int regionIndex, const QString& patternType) {
    if (patternType == "postal_code") {
        // Canadian postal code pattern: A#A-#A#
        // Indices: 0=L, 1=N, 2=L, 3=N, 4=L, 5=N
        if (regionIndex == 0 || regionIndex == 2 || regionIndex == 4) {
            return "letters";
        } else if (regionIndex == 1 || regionIndex == 3 || regionIndex == 5) {
            return "numbers";
        }
    } else if (patternType == "name_field") {
        return "letters"; // Name fields are typically letters
    } else if (patternType == "number_sequence") {
        return "numbers"; // Number sequences are numbers
    }
    
    return "unknown";
}

QString TypeInferencer::combineTypeInferences(const QString& visualType, const QString& patternType) {
    // If pattern type is known, prefer it (more reliable)
    if (patternType != "unknown") {
        return patternType;
    }
    
    // Otherwise use visual type
    return visualType;
}

} // namespace ocr_orc

#ifndef TYPE_INFERENCER_H
#define TYPE_INFERENCER_H

#include <QtCore/QString>
#include <opencv2/opencv.hpp>

namespace ocr_orc {

/**
 * @brief Type inference for detected regions
 * 
 * Infers region type (letters/numbers/mixed) from visual characteristics
 * and pattern analysis.
 */
class TypeInferencer {
public:
    TypeInferencer();
    ~TypeInferencer() = default;
    
    /**
     * @brief Infer type from visual characteristics
     * @param regionImage ROI of the region (cv::Mat)
     * @param aspectRatio Aspect ratio of the region (width/height)
     * @return Inferred type: "letters", "numbers", "mixed", "unknown"
     */
    QString inferTypeFromVisual(const cv::Mat& regionImage, double aspectRatio);
    
    /**
     * @brief Infer type from pattern context
     * @param regionIndex Index in a sequence
     * @param patternType Known pattern type (e.g., "postal_code", "name_field")
     * @return Inferred type based on pattern
     */
    QString inferTypeFromPattern(int regionIndex, const QString& patternType);
    
    /**
     * @brief Combine multiple type inferences
     * @param visualType Type from visual analysis
     * @param patternType Type from pattern analysis
     * @return Combined type inference
     */
    QString combineTypeInferences(const QString& visualType, const QString& patternType);

private:
    /**
     * @brief Analyze edge density in region
     */
    double analyzeEdgeDensity(const cv::Mat& regionImage);
    
    /**
     * @brief Analyze texture characteristics
     */
    double analyzeTexture(const cv::Mat& regionImage);
    
    /**
     * @brief Check if region appears to be a letter field
     */
    bool appearsAsLetterField(const cv::Mat& regionImage, double aspectRatio);
    
    /**
     * @brief Check if region appears to be a number field
     */
    bool appearsAsNumberField(const cv::Mat& regionImage, double aspectRatio);
};

} // namespace ocr_orc

#endif // TYPE_INFERENCER_H

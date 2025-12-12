#ifndef DOCUMENT_PREPROCESSOR_H
#define DOCUMENT_PREPROCESSOR_H

#include <opencv2/opencv.hpp>

namespace ocr_orc {

/**
 * @brief Document preprocessor
 * 
 * Handles scanned document quality issues:
 * - Skew detection and correction
 * - Denoising
 * - Shadow removal
 * - Contrast enhancement
 */
class DocumentPreprocessor {
public:
    DocumentPreprocessor();
    ~DocumentPreprocessor() = default;
    
    /**
     * @brief Preprocess document (full pipeline)
     * @param image Source image
     * @return Preprocessed image
     */
    cv::Mat preprocess(const cv::Mat& image);
    
    /**
     * @brief Correct skew (rotation) in document
     * @param image Source image
     * @return Deskewed image
     */
    cv::Mat correctSkew(const cv::Mat& image);
    
    /**
     * @brief Denoise image
     * @param image Source image
     * @return Denoised image
     */
    cv::Mat denoise(const cv::Mat& image);
    
    /**
     * @brief Remove shadows from image
     * @param image Source image
     * @return Image with shadows removed
     */
    cv::Mat removeShadows(const cv::Mat& image);
    
    /**
     * @brief Enhance contrast
     * @param image Source image
     * @return Contrast-enhanced image
     */
    cv::Mat enhanceContrast(const cv::Mat& image);
    
    /**
     * @brief Detect skew angle
     * @param image Source image
     * @return Skew angle in degrees
     */
    double detectSkewAngle(const cv::Mat& image);
    
    /**
     * @brief Enable/disable preprocessing steps
     * @param enableSkew Enable skew correction
     * @param enableDenoise Enable denoising
     * @param enableShadowRemoval Enable shadow removal
     * @param enableContrast Enable contrast enhancement
     */
    void setPreprocessingOptions(bool enableSkew = true, bool enableDenoise = true,
                                bool enableShadowRemoval = true, bool enableContrast = true);

private:
    bool enableSkewCorrection;
    bool enableDenoising;
    bool enableShadowRemoval;
    bool enableContrastEnhancement;
    
    /**
     * @brief Rotate image by angle
     * @param image Source image
     * @param angle Rotation angle in degrees
     * @return Rotated image
     */
    cv::Mat rotateImage(const cv::Mat& image, double angle);
};

} // namespace ocr_orc

#endif // DOCUMENT_PREPROCESSOR_H

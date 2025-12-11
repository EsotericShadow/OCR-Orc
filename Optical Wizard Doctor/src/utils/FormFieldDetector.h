#ifndef FORM_FIELD_DETECTOR_H
#define FORM_FIELD_DETECTOR_H

#include "OcrTextExtractor.h"
#include "CheckboxDetector.h"
#include <opencv2/opencv.hpp>
#include <QtCore/QString>
#include <QtCore/QList>

namespace ocr_orc {

/**
 * @brief Form field type enumeration
 */
enum class FormFieldType {
    TextInput,      // Single-line text input (underline or box)
    TextLine,       // Single-line text input with underline (handwriting line)
    TextBlock,      // Multi-line text area for handwriting (government forms)
    MultiLineText,  // Multi-line text area
    CheckboxField,  // Checkbox + label
    GridCell,       // Part of grid/table
    Cell,           // Individual input cell
    Title,          // Title/heading (should be filtered out)
    Label,          // Just a label (no input field)
    Unknown
};

/**
 * @brief Form field detector
 * 
 * Classifies OCR text regions into form field types by analyzing
 * visual characteristics and spatial relationships.
 */
class FormFieldDetector {
public:
    FormFieldDetector();
    ~FormFieldDetector() = default;
    
    /**
     * @brief Detect form field type for a text region
     * @param textRegion OCR text region
     * @param image Source image
     * @param checkbox Optional checkbox detection for this region
     * @return Detected form field type
     */
    FormFieldType detectFormField(const OCRTextRegion& textRegion, 
                                  const cv::Mat& image,
                                  const CheckboxDetection& checkbox = CheckboxDetection());
    
    /**
     * @brief Detect multi-line text areas from multiple regions
     * @param regions List of OCR regions
     * @param pageWidth Page width in pixels
     * @return List of multi-line text area regions
     */
    QList<OCRTextRegion> detectMultiLineTextArea(const QList<OCRTextRegion>& regions, 
                                                  int pageWidth);
    
    /**
     * @brief Detect grid cells from OCR regions
     * @param regions List of OCR regions
     * @return List of regions that are part of a grid
     */
    QList<OCRTextRegion> detectGridCells(const QList<OCRTextRegion>& regions);
    
    /**
     * @brief Classify and refine detected regions (overfit then classify)
     * @param regions List of detected regions (cv::Rect)
     * @param image Source image
     * @param ocrRegions All OCR text regions for context
     * @return List of classified and validated form fields
     */
    QList<cv::Rect> classifyAndRefineRegions(const QList<cv::Rect>& regions,
                                             const cv::Mat& image,
                                             const QList<OCRTextRegion>& ocrRegions);
    
    /**
     * @brief Drastically overfit a region (expand by percentage)
     * @param region Original region
     * @param image Source image (for bounds)
     * @param expandPercent Percentage to expand (default: 30% for drastic overfitting)
     * @return Overfitted region
     */
    cv::Rect overfitRegion(const cv::Rect& region, const cv::Mat& image, int expandPercent = 30);
    
    /**
     * @brief Extremely drastic overfitting with asymmetric expansion
     * @param region Original region
     * @param image Source image (for bounds)
     * @param expandPercentX Horizontal expansion percentage
     * @param expandPercentY Vertical expansion percentage (typically larger for multi-line fields)
     * @return Overfitted region with asymmetric expansion
     */
    cv::Rect overfitRegionAsymmetric(const cv::Rect& region, const cv::Mat& image, 
                                     int expandPercentX, int expandPercentY);
    
    /**
     * @brief Refine overfitted regions using smart boundary detection
     * Finds actual form field boundaries within overfitted regions using edge detection
     * Looks for hard edges ABOVE regions to use as anchor points
     * @param overfittedRegions List of overfitted regions
     * @param image Source image
     * @return Refined regions with accurate boundaries and increased vertical height
     */
    QList<cv::Rect> refineOverfittedRegions(const QList<cv::Rect>& overfittedRegions, const cv::Mat& image);
    
    /**
     * @brief Find hard edge (horizontal line) above a region to use as anchor
     * @param region Region to find edge above
     * @param image Source image
     * @param searchHeight How far above to search (default: 50px)
     * @return Y coordinate of edge, or -1 if not found
     */
    int findHardEdgeAbove(const cv::Rect& region, const cv::Mat& image, int searchHeight = 50);
    
    /**
     * @brief Find hard edge (vertical line) to the left of a region to use as anchor
     * @param region Region to find edge left of
     * @param image Source image
     * @param searchWidth How far left to search (default: 50px)
     * @return X coordinate of edge, or -1 if not found
     */
    int findHardEdgeLeft(const cv::Rect& region, const cv::Mat& image, int searchWidth = 50);
    
    /**
     * @brief Find hard edge (vertical line) to the right of a region to use as anchor
     * @param region Region to find edge right of
     * @param image Source image
     * @param searchWidth How far right to search (default: 50px)
     * @return X coordinate of edge, or -1 if not found
     */
    int findHardEdgeRight(const cv::Rect& region, const cv::Mat& image, int searchWidth = 50);
    
    /**
     * @brief Detect groups of cells that share vertical walls (grid pattern)
     * @param regions List of detected regions
     * @param image Source image
     * @return List of cell groups (each group contains cells sharing walls)
     */
    QList<QList<cv::Rect>> detectCellGroupsWithSharedWalls(const QList<cv::Rect>& regions, const cv::Mat& image);
    
    /**
     * @brief Find vertical walls/lines between cells (more sensitive detection)
     * @param region Region to analyze
     * @param image Source image
     * @return List of vertical line X coordinates (walls)
     */
    QList<int> findVerticalWalls(const cv::Rect& region, const cv::Mat& image);
    
    /**
     * @brief Enhanced horizontal edge detection (more sensitive)
     * @param region Region to analyze
     * @param image Source image
     * @return List of horizontal edge Y coordinates
     */
    QList<int> findHorizontalEdges(const cv::Rect& region, const cv::Mat& image);
    
    /**
     * @brief Classify a region type (TextLine, TextBlock, Cell, Title, etc.)
     * @param region Region to classify
     * @param image Source image
     * @param ocrRegions All OCR regions for context
     * @return Classified form field type
     */
    FormFieldType classifyRegionType(const cv::Rect& region, 
                                     const cv::Mat& image,
                                     const QList<OCRTextRegion>& ocrRegions);
    
    /**
     * @brief Check if region is a title/heading (should be filtered out)
     * @param region Region to check
     * @param image Source image
     * @param ocrRegions All OCR regions
     * @return True if region appears to be a title/heading
     */
    bool isTitleOrHeading(const cv::Rect& region, 
                         const cv::Mat& image,
                         const QList<OCRTextRegion>& ocrRegions);
    
    /**
     * @brief Distinguish text line (single-line) from text block (multi-line)
     * @param region Region to analyze
     * @param image Source image
     * @return True if text block (multi-line), false if text line (single-line)
     */
    bool isTextBlock(const cv::Rect& region, const cv::Mat& image);

private:
    /**
     * @brief Detect single-line text input field
     * @param textRegion OCR text region
     * @param image Source image
     * @return True if text input field detected
     */
    bool detectTextInputField(const OCRTextRegion& textRegion, const cv::Mat& image);
    
    /**
     * @brief Check for horizontal line below text (underline)
     * @param textBox Text bounding box
     * @param image Source image
     * @return True if underline detected
     */
    bool detectUnderline(const cv::Rect& textBox, const cv::Mat& image);
    
    /**
     * @brief Check for rectangle around text (form field box)
     * @param textBox Text bounding box
     * @param image Source image
     * @return True if bounding rectangle detected
     */
    bool detectBoundingRectangle(const cv::Rect& textBox, const cv::Mat& image);
};

} // namespace ocr_orc

#endif // FORM_FIELD_DETECTOR_H

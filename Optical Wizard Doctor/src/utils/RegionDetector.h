#ifndef REGION_DETECTOR_H
#define REGION_DETECTOR_H

#include <QtGui/QImage>
#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QPair>
#include <opencv2/opencv.hpp>
#include "../core/CoordinateSystem.h"
#include "SpatialClusterer.h"
#include "DocumentTypeClassifier.h"

namespace ocr_orc {

// Forward declarations
struct DetectedRectangle;  // Defined in RectangleDetector.h

// Forward declare OCRTextRegion - we'll include the header in .cpp
struct OCRTextRegion;

// Forward declare AdaptiveThresholdManager (class, not enum, so forward declaration is OK)
class AdaptiveThresholdManager;

// Forward declare DetectionParameters - defined in MagicDetectParamsDialog.h
struct DetectionParameters;

/**
 * @brief Detected region from automatic detection
 */
struct DetectedRegion {
    NormalizedCoords coords;      // Detected region coordinates (normalized 0.0-1.0)
    double confidence;            // Confidence score (0.0-1.0)
    QString method;               // Detection method used ("grid", "contour", "hybrid")
    cv::Rect boundingBox;         // Original OpenCV bounding box (for reference)
    QString inferredType;         // Inferred type: "letters", "numbers", "mixed", "unknown"
    QString suggestedGroup;        // Suggested group name (e.g., "Postalcode", "Firstname")
    QString suggestedColor;        // Suggested color: "red", "blue", "green", "yellow", etc.
    
    DetectedRegion() : confidence(0.0), inferredType("unknown") {}
    DetectedRegion(const NormalizedCoords& c, double conf, const QString& m, const cv::Rect& bb)
        : coords(c), confidence(conf), method(m), boundingBox(bb), inferredType("unknown") {}
};

/**
 * @brief Detected group structure
 */
struct DetectedGroup {
    QString name;                   // Group name (e.g., "Postalcode", "Firstname")
    QList<QString> regionNames;     // Names of regions in this group
    QString suggestedColor;          // Suggested color for the group
    double confidence;              // Confidence in group detection
    
    DetectedGroup() : confidence(0.0) {}
    DetectedGroup(const QString& n, const QList<QString>& rn, const QString& c, double conf)
        : name(n), regionNames(rn), suggestedColor(c), confidence(conf) {}
};

/**
 * @brief Grid structure detected from regions
 */
struct GridStructure {
    int rows;
    int cols;
    QList<QList<DetectedRegion>> gridCells;  // gridCells[row][col]
    double cellWidth;   // Average cell width (normalized)
    double cellHeight;  // Average cell height (normalized)
    double confidence; // Confidence in grid detection
    
    GridStructure() : rows(0), cols(0), cellWidth(0.0), cellHeight(0.0), confidence(0.0) {}
};

/**
 * @brief Result of region detection operation
 */
struct DetectionResult {
    QList<DetectedRegion> regions;  // All detected regions
    int totalDetected;              // Total count
    int highConfidence;             // High confidence count (>= 0.8)
    int mediumConfidence;           // Medium confidence count (0.5-0.8)
    int lowConfidence;              // Low confidence count (< 0.5)
    QString methodUsed;             // Primary method that succeeded
    
    // Enhanced fields for smart detection
    QList<DetectedGroup> inferredGroups;  // Groups inferred from patterns
    QMap<QString, QString> regionTypes;    // Region name → type mapping
    QMap<QString, QString> suggestedColors; // Group name → color mapping
    GridStructure detectedGrid;            // Detected grid structure (if any)
    
    DetectionResult() : totalDetected(0), highConfidence(0), mediumConfidence(0), lowConfidence(0) {}
};

/**
 * @brief Automatic region detection using computer vision
 * 
 * Detects regions (cells, form fields) in scanned documents using:
 * - Grid detection (Hough transform for visible grid lines)
 * - Contour detection (rectangular shape detection)
 * - Hybrid method (combines both approaches)
 */
class RegionDetector {
public:
    RegionDetector();
    ~RegionDetector() = default;
    
    /**
     * @brief Detect regions in an image
     * @param image Source image (QImage)
     * @param method Detection method: "auto", "grid", "contour", "hybrid", "ocr-first"
     * @param params Optional detection parameters (if not provided, uses defaults)
     * @return DetectionResult with detected regions and statistics
     */
    DetectionResult detectRegions(const QImage& image, const QString& method, 
                                  const DetectionParameters& params);
    
    /**
     * @brief Detect regions using OCR-first approach
     * @param image Source image (QImage)
     * @param method Detection method (default: "ocr-first")
     * @param params Detection parameters
     * @return DetectionResult with detected regions and statistics
     */
    DetectionResult detectRegionsOCRFirst(const QImage& image, const QString& method,
                                          const DetectionParameters& params);
    
    /**
     * @brief Enable/disable document preprocessing
     * @param enable True to enable preprocessing (deskew, denoise, shadow removal, contrast)
     */
    void setPreprocessingEnabled(bool enable) { enablePreprocessing = enable; }
    
    /**
     * @brief Get preprocessing enabled state
     * @return True if preprocessing is enabled
     */
    bool isPreprocessingEnabled() const { return enablePreprocessing; }
    
    /**
     * @brief Match and merge results from OCR-first and rectangle detection pipelines
     * @param ocrRegions Results from OCR-first pipeline (cv::Rect)
     * @param rectangleRegions Results from rectangle detection pipeline (DetectedRectangle)
     * @param image Source image
     * @return Merged DetectionResult with matched regions (consensus-based)
     */
    DetectionResult matchAndMergePipelines(const QList<cv::Rect>& ocrRegions,
                                          const QList<DetectedRectangle>& rectangleRegions,
                                          const QImage& image,
                                          const QList<OCRTextRegion>& ocrTextRegions,
                                          const DetectionParameters& params);
    
    /**
     * @brief Handle multi-label fields (one rectangle matching multiple OCR regions)
     * @param rectangleToOcrMatches Map of rectangle to matching OCR regions
     * @param ocrTextRegions All OCR text regions for semantic validation
     * @return List of detected regions for multi-label fields
     */
    QList<DetectedRegion> handleMultiLabelFields(
        const QMap<cv::Rect, QList<cv::Rect>>& rectangleToOcrMatches,
        const QList<OCRTextRegion>& ocrTextRegions);
    
    // Parameter setters
    void setMinCellSize(int width, int height);
    void setMaxCellSize(int width, int height);
    void setLineDetectionThreshold(int threshold);
    void setContourMinArea(int area);
    
    /**
     * @brief Consensus matching mode
     */
    enum ConsensusMode {
        STRICT_CONSENSUS,   // Only include consensus matches (IoU >0.6)
        LENIENT_CONSENSUS   // Include high-confidence OCR-only if no rectangles nearby
    };
    
    /**
     * @brief Set consensus matching mode
     * @param mode Consensus mode
     */
    void setConsensusMode(ConsensusMode mode) { consensusMode = mode; }
    
    /**
     * @brief Get current consensus mode
     * @return Consensus mode
     */
    ConsensusMode getConsensusMode() const { return consensusMode; }
    
    /**
     * @brief Set instrumentation for tracking pipeline execution
     * @param instrumentation Instrumentation instance (can be nullptr to disable)
     */
    void setInstrumentation(void* instrumentation) {
        this->instrumentation = instrumentation;
    }
    
private:
    // Detection methods
    DetectionResult detectGrid(const QImage& image);
    DetectionResult detectContours(const QImage& image);
    DetectionResult detectHybrid(const QImage& image);
    
    // Helper methods
    QList<DetectedRegion> filterRegions(const QList<DetectedRegion>& regions);
    double validateSizeConsistency(const cv::Rect& rect, const QList<DetectedRegion>& allRegions, int imgWidth, int imgHeight, double distanceThreshold = 0.1);
    double calculateConfidence(const cv::Rect& rect, const cv::Mat& image, const QString& method);
    double calculateConfidenceWithContext(const DetectedRegion& region, const cv::Mat& image, 
                                          const QList<DetectedRegion>& allRegions, 
                                          const GridStructure& gridStructure);
    NormalizedCoords convertToNormalized(const cv::Rect& rect, int imgWidth, int imgHeight);
    double calculateOverlap(const NormalizedCoords& coords1, const NormalizedCoords& coords2);
    DetectionResult mergeDetectionResults(const DetectionResult& result1, const DetectionResult& result2);
    cv::Point2f findLineIntersection(const cv::Vec4i& line1, const cv::Vec4i& line2);
    bool isValidIntersection(const cv::Point2f& point, const cv::Size& imageSize);
    std::vector<cv::Rect> formGridCells(const std::vector<cv::Point2f>& intersections);
    
    // Multi-scale detection methods
    DetectionResult detectAtScale(const QImage& image, const QString& method, double scale);
    DetectionResult mergeScaleResults(const QList<DetectionResult>& scaleResults, const QSize& originalSize);
    
    // Detection parameters
    int minCellWidth;
    int minCellHeight;
    int maxCellWidth;
    int maxCellHeight;
    int lineThreshold;        // Hough transform threshold
    int contourMinArea;      // Minimum contour area
    ConsensusMode consensusMode;  // Consensus matching mode (default: LENIENT_CONSENSUS)
    bool enablePreprocessing;    // Enable document preprocessing (default: false)
    
    // Instrumentation (optional, for testing and analysis)
    // Using void* to avoid including test headers in production code
    void* instrumentation;
};

} // namespace ocr_orc

#endif // REGION_DETECTOR_H

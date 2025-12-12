#ifndef DETECTION_CACHE_H
#define DETECTION_CACHE_H

#include <opencv2/opencv.hpp>
#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtCore/QRecursiveMutex>

namespace ocr_orc {

/**
 * @brief Cache for expensive detection calculations (Canny edges, brightness, edge density)
 * 
 * This cache stores precomputed values to avoid redundant calculations during detection.
 * Cache is keyed by region coordinates and image hash to ensure correctness.
 */
class DetectionCache {
public:
    DetectionCache();
    ~DetectionCache() = default;
    
    /**
     * @brief Get or compute Canny edges for a region
     * @param image Full image
     * @param region Region of interest
     * @param lowThreshold Canny low threshold
     * @param highThreshold Canny high threshold
     * @return Canny edge image for the region
     */
    cv::Mat getCannyEdges(const cv::Mat& image, const cv::Rect& region,
                          int lowThreshold = 50, int highThreshold = 150);
    
    /**
     * @brief Get or compute brightness for a region
     * @param image Full image
     * @param region Region of interest
     * @return Average brightness (0.0-1.0)
     */
    double getBrightness(const cv::Mat& image, const cv::Rect& region);
    
    /**
     * @brief Get or compute edge density for a region
     * @param image Full image
     * @param region Region of interest
     * @param lowThreshold Canny low threshold
     * @param highThreshold Canny high threshold
     * @return Edge density (0.0-1.0)
     */
    double getEdgeDensity(const cv::Mat& image, const cv::Rect& region,
                         int lowThreshold = 50, int highThreshold = 150);
    
    /**
     * @brief Get or compute horizontal edge density for a region
     * @param image Full image
     * @param region Region of interest
     * @return Horizontal edge density (0.0-1.0)
     */
    double getHorizontalEdgeDensity(const cv::Mat& image, const cv::Rect& region);
    
    /**
     * @brief Get or compute vertical edge density for a region
     * @param image Full image
     * @param region Region of interest
     * @return Vertical edge density (0.0-1.0)
     */
    double getVerticalEdgeDensity(const cv::Mat& image, const cv::Rect& region);
    
    /**
     * @brief Clear all cached values
     */
    void clear();
    
    /**
     * @brief Clear cache for a specific region
     * @param region Region to clear
     */
    void clearRegion(const cv::Rect& region);
    
    /**
     * @brief Get cache statistics
     * @return Number of cached entries
     */
    int getCacheSize() const;

private:
    /**
     * @brief Generate cache key from region and thresholds
     */
    QString generateKey(const cv::Rect& region, int lowThreshold = 0, int highThreshold = 0) const;
    
    /**
     * @brief Generate image hash for cache validation
     */
    QString generateImageHash(const cv::Mat& image) const;
    
    // Cache storage
    QHash<QString, cv::Mat> cannyCache;              // Key: "region_hash_thresholds" -> Canny edges
    QHash<QString, double> brightnessCache;          // Key: "region_hash" -> brightness
    QHash<QString, double> edgeDensityCache;         // Key: "region_hash_thresholds" -> edge density
    QHash<QString, double> horizontalEdgeDensityCache; // Key: "region_hash" -> horizontal edge density
    QHash<QString, double> verticalEdgeDensityCache;   // Key: "region_hash" -> vertical edge density
    
    // Image hash for cache validation (ensures cache is valid for current image)
    QString currentImageHash;
    
    // Mutex for thread safety (recursive to allow nested locking from clear() calls)
    mutable QRecursiveMutex cacheMutex;
    
    // Maximum cache size (to prevent memory issues)
    static const int MAX_CACHE_SIZE = 1000;
};

} // namespace ocr_orc

#endif // DETECTION_CACHE_H

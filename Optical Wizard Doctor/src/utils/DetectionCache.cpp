#include "DetectionCache.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <QtCore/QCryptographicHash>
#include <QtCore/QByteArray>
#include <QtCore/QMutex>
#include <QtCore/QMutableHashIterator>
#include <algorithm>
#include <cmath>
#include <cstdio>

namespace ocr_orc {

DetectionCache::DetectionCache()
    : currentImageHash("")
{
}

QString DetectionCache::generateKey(const cv::Rect& region, int lowThreshold, int highThreshold) const
{
    QString key = QString("%1_%2_%3_%4")
                  .arg(region.x)
                  .arg(region.y)
                  .arg(region.width)
                  .arg(region.height);
    
    if (lowThreshold > 0 || highThreshold > 0) {
        key += QString("_%1_%2").arg(lowThreshold).arg(highThreshold);
    }
    
    return key;
}

QString DetectionCache::generateImageHash(const cv::Mat& image) const
{
    if (image.empty()) {
        return "";
    }
    
    // Generate a simple hash from image dimensions and a sample of pixels
    // This is a lightweight hash for cache validation
    QCryptographicHash hash(QCryptographicHash::Md5);
    
    // Add dimensions
    QByteArray dims;
    dims.append(reinterpret_cast<const char*>(&image.cols), sizeof(int));
    dims.append(reinterpret_cast<const char*>(&image.rows), sizeof(int));
    hash.addData(dims);
    
    // Add sample pixels (corners and center) for quick validation
    if (image.cols > 0 && image.rows > 0) {
        // Sample 9 points: corners, edges, center
        int samplePoints[9][2] = {
            {0, 0}, {image.cols/2, 0}, {image.cols-1, 0},
            {0, image.rows/2}, {image.cols/2, image.rows/2}, {image.cols-1, image.rows/2},
            {0, image.rows-1}, {image.cols/2, image.rows-1}, {image.cols-1, image.rows-1}
        };
        
        for (int i = 0; i < 9; i++) {
            int x = samplePoints[i][0];
            int y = samplePoints[i][1];
            if (x < image.cols && y < image.rows) {
                cv::Vec3b pixel = image.at<cv::Vec3b>(y, x);
                QByteArray pixelData(reinterpret_cast<const char*>(&pixel), sizeof(cv::Vec3b));
                hash.addData(pixelData);
            }
        }
    }
    
    return hash.result().toHex();
}

cv::Mat DetectionCache::getCannyEdges(const cv::Mat& image, const cv::Rect& region,
                                      int lowThreshold, int highThreshold)
{
    fprintf(stderr, "[DetectionCache::getCannyEdges] ENTERED - region(x=%d,y=%d,w=%d,h=%d), imageSize=%dx%d, thresholds=%d/%d\n",
            region.x, region.y, region.width, region.height, image.cols, image.rows, lowThreshold, highThreshold);
    fflush(stderr);
    
    fprintf(stderr, "[DetectionCache::getCannyEdges] Step 1: Acquiring mutex lock...\n");
    fflush(stderr);
    QMutexLocker locker(&cacheMutex);
    fprintf(stderr, "[DetectionCache::getCannyEdges] Step 1: ✓ Mutex locked\n");
    fflush(stderr);
    
    // Validate region
    fprintf(stderr, "[DetectionCache::getCannyEdges] Step 2: Validating region...\n");
    fflush(stderr);
    if (region.width <= 0 || region.height <= 0 ||
        region.x < 0 || region.y < 0 ||
        region.x + region.width > image.cols ||
        region.y + region.height > image.rows) {
        fprintf(stderr, "[DetectionCache::getCannyEdges] Step 2: ✗ Region invalid - returning empty Mat\n");
        fflush(stderr);
        return cv::Mat();
    }
    fprintf(stderr, "[DetectionCache::getCannyEdges] Step 2: ✓ Region valid\n");
    fflush(stderr);
    
    // Check if image changed (invalidate cache)
    fprintf(stderr, "[DetectionCache::getCannyEdges] Step 3: Generating image hash (this may access image pixels)...\n");
    fflush(stderr);
    QString imageHash = generateImageHash(image);
    fprintf(stderr, "[DetectionCache::getCannyEdges] Step 3: ✓ Image hash generated: %s (current: %s)\n", 
            imageHash.toLocal8Bit().constData(), currentImageHash.toLocal8Bit().constData());
    fflush(stderr);
    if (imageHash != currentImageHash) {
        fprintf(stderr, "[DetectionCache::getCannyEdges] Step 3.1: Image changed - clearing cache...\n");
        fflush(stderr);
        clear();
        currentImageHash = imageHash;
        fprintf(stderr, "[DetectionCache::getCannyEdges] Step 3.1: ✓ Cache cleared\n");
        fflush(stderr);
    }
    
    // Generate cache key
    fprintf(stderr, "[DetectionCache::getCannyEdges] Step 4: Generating cache key...\n");
    fflush(stderr);
    QString key = generateKey(region, lowThreshold, highThreshold);
    fprintf(stderr, "[DetectionCache::getCannyEdges] Step 4: ✓ Cache key: %s\n", key.toLocal8Bit().constData());
    fflush(stderr);
    
    // Check cache
    fprintf(stderr, "[DetectionCache::getCannyEdges] Step 5: Checking cache (size=%d)...\n", cannyCache.size());
    fflush(stderr);
    if (cannyCache.contains(key)) {
        fprintf(stderr, "[DetectionCache::getCannyEdges] Step 5: ✓ Cache HIT - cloning and returning...\n");
        fflush(stderr);
        return cannyCache[key].clone();  // Return copy to avoid cache corruption
    }
    fprintf(stderr, "[DetectionCache::getCannyEdges] Step 5: ✗ Cache MISS - need to compute\n");
    fflush(stderr);
    
    // Compute Canny edges
    fprintf(stderr, "[DetectionCache::getCannyEdges] Step 6: Extracting ROI...\n");
    fflush(stderr);
    cv::Mat roi = image(region);
    fprintf(stderr, "[DetectionCache::getCannyEdges] Step 6: ✓ ROI extracted (size: %dx%d, channels=%d)\n", 
            roi.cols, roi.rows, roi.channels());
    fflush(stderr);
    
    fprintf(stderr, "[DetectionCache::getCannyEdges] Step 7: Converting to grayscale if needed...\n");
    fflush(stderr);
    cv::Mat gray;
    if (roi.channels() == 3) {
        fprintf(stderr, "[DetectionCache::getCannyEdges] Step 7: Calling cv::cvtColor() on ROI...\n");
        fflush(stderr);
        cv::cvtColor(roi, gray, cv::COLOR_BGR2GRAY);
        fprintf(stderr, "[DetectionCache::getCannyEdges] Step 7: ✓ cv::cvtColor() returned\n");
        fflush(stderr);
    } else {
        fprintf(stderr, "[DetectionCache::getCannyEdges] Step 7: Cloning ROI (already grayscale)...\n");
        fflush(stderr);
        gray = roi.clone();
        fprintf(stderr, "[DetectionCache::getCannyEdges] Step 7: ✓ ROI cloned\n");
        fflush(stderr);
    }
    
    fprintf(stderr, "[DetectionCache::getCannyEdges] Step 8: Calling cv::Canny() - THIS IS EXPENSIVE...\n");
    fflush(stderr);
    cv::Mat edges;
    cv::Canny(gray, edges, lowThreshold, highThreshold);
    fprintf(stderr, "[DetectionCache::getCannyEdges] Step 8: ✓ cv::Canny() returned (edges size: %dx%d)\n", 
            edges.cols, edges.rows);
    fflush(stderr);
    
    // Store in cache (if not too large)
    fprintf(stderr, "[DetectionCache::getCannyEdges] Step 9: Storing in cache (current size=%d, MAX=%d)...\n", 
            cannyCache.size(), MAX_CACHE_SIZE);
    fflush(stderr);
    if (cannyCache.size() < MAX_CACHE_SIZE) {
        fprintf(stderr, "[DetectionCache::getCannyEdges] Step 9: Cloning edges for cache storage...\n");
        fflush(stderr);
        cannyCache[key] = edges.clone();
        fprintf(stderr, "[DetectionCache::getCannyEdges] Step 9: ✓ Edges stored in cache\n");
        fflush(stderr);
    } else {
        fprintf(stderr, "[DetectionCache::getCannyEdges] Step 9: ✗ Cache full - not storing\n");
        fflush(stderr);
    }
    
    fprintf(stderr, "[DetectionCache::getCannyEdges] Step 10: Returning edges...\n");
    fflush(stderr);
    return edges;
}

double DetectionCache::getBrightness(const cv::Mat& image, const cv::Rect& region)
{
    QMutexLocker locker(&cacheMutex);
    
    // Validate region
    if (region.width <= 0 || region.height <= 0 ||
        region.x < 0 || region.y < 0 ||
        region.x + region.width > image.cols ||
        region.y + region.height > image.rows) {
        return 0.0;
    }
    
    // Check if image changed
    QString imageHash = generateImageHash(image);
    if (imageHash != currentImageHash) {
        clear();
        currentImageHash = imageHash;
    }
    
    // Generate cache key
    QString key = generateKey(region);
    
    // Check cache
    if (brightnessCache.contains(key)) {
        return brightnessCache[key];
    }
    
    // Compute brightness
    cv::Mat roi = image(region);
    cv::Mat gray;
    if (roi.channels() == 3) {
        cv::cvtColor(roi, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = roi.clone();
    }
    
    cv::Scalar meanScalar = cv::mean(gray);
    double brightness = meanScalar[0] / 255.0;  // Normalize to 0.0-1.0
    
    // Store in cache
    if (brightnessCache.size() < MAX_CACHE_SIZE) {
        brightnessCache[key] = brightness;
    }
    
    return brightness;
}

double DetectionCache::getEdgeDensity(const cv::Mat& image, const cv::Rect& region,
                                     int lowThreshold, int highThreshold)
{
    QMutexLocker locker(&cacheMutex);
    
    // Validate region
    if (region.width <= 0 || region.height <= 0 ||
        region.x < 0 || region.y < 0 ||
        region.x + region.width > image.cols ||
        region.y + region.height > image.rows) {
        return 0.0;
    }
    
    // Check if image changed
    QString imageHash = generateImageHash(image);
    if (imageHash != currentImageHash) {
        clear();
        currentImageHash = imageHash;
    }
    
    // Generate cache key
    QString key = generateKey(region, lowThreshold, highThreshold);
    
    // Check cache
    if (edgeDensityCache.contains(key)) {
        return edgeDensityCache[key];
    }
    
    // Get Canny edges (may use cache)
    cv::Mat edges = getCannyEdges(image, region, lowThreshold, highThreshold);
    
    if (edges.empty()) {
        return 0.0;
    }
    
    // Compute edge density
    int edgePixels = cv::countNonZero(edges);
    double density = static_cast<double>(edgePixels) / (region.width * region.height);
    
    // Store in cache
    if (edgeDensityCache.size() < MAX_CACHE_SIZE) {
        edgeDensityCache[key] = density;
    }
    
    return density;
}

double DetectionCache::getHorizontalEdgeDensity(const cv::Mat& image, const cv::Rect& region)
{
    QMutexLocker locker(&cacheMutex);
    
    // Validate region
    if (region.width <= 0 || region.height <= 0 ||
        region.x < 0 || region.y < 0 ||
        region.x + region.width > image.cols ||
        region.y + region.height > image.rows) {
        return 0.0;
    }
    
    // Check if image changed
    QString imageHash = generateImageHash(image);
    if (imageHash != currentImageHash) {
        clear();
        currentImageHash = imageHash;
    }
    
    // Generate cache key
    QString key = generateKey(region);
    
    // Check cache
    if (horizontalEdgeDensityCache.contains(key)) {
        return horizontalEdgeDensityCache[key];
    }
    
    // Get Canny edges
    cv::Mat edges = getCannyEdges(image, region, 50, 150);
    
    if (edges.empty()) {
        return 0.0;
    }
    
    // Create horizontal kernel to emphasize horizontal edges
    cv::Mat horizontalKernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 1));
    cv::Mat horizontalEdges;
    cv::morphologyEx(edges, horizontalEdges, cv::MORPH_DILATE, horizontalKernel);
    
    // Count horizontal edge pixels
    int horizontalEdgePixels = cv::countNonZero(horizontalEdges);
    double density = static_cast<double>(horizontalEdgePixels) / (region.width * region.height);
    
    // Store in cache
    if (horizontalEdgeDensityCache.size() < MAX_CACHE_SIZE) {
        horizontalEdgeDensityCache[key] = density;
    }
    
    return density;
}

double DetectionCache::getVerticalEdgeDensity(const cv::Mat& image, const cv::Rect& region)
{
    QMutexLocker locker(&cacheMutex);
    
    // Validate region
    if (region.width <= 0 || region.height <= 0 ||
        region.x < 0 || region.y < 0 ||
        region.x + region.width > image.cols ||
        region.y + region.height > image.rows) {
        return 0.0;
    }
    
    // Check if image changed
    QString imageHash = generateImageHash(image);
    if (imageHash != currentImageHash) {
        clear();
        currentImageHash = imageHash;
    }
    
    // Generate cache key
    QString key = generateKey(region);
    
    // Check cache
    if (verticalEdgeDensityCache.contains(key)) {
        return verticalEdgeDensityCache[key];
    }
    
    // Get Canny edges
    cv::Mat edges = getCannyEdges(image, region, 50, 150);
    
    if (edges.empty()) {
        return 0.0;
    }
    
    // Create vertical kernel to emphasize vertical edges
    cv::Mat verticalKernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(1, 5));
    cv::Mat verticalEdges;
    cv::morphologyEx(edges, verticalEdges, cv::MORPH_DILATE, verticalKernel);
    
    // Count vertical edge pixels
    int verticalEdgePixels = cv::countNonZero(verticalEdges);
    double density = static_cast<double>(verticalEdgePixels) / (region.width * region.height);
    
    // Store in cache
    if (verticalEdgeDensityCache.size() < MAX_CACHE_SIZE) {
        verticalEdgeDensityCache[key] = density;
    }
    
    return density;
}

void DetectionCache::clear()
{
    QMutexLocker locker(&cacheMutex);
    cannyCache.clear();
    brightnessCache.clear();
    edgeDensityCache.clear();
    horizontalEdgeDensityCache.clear();
    verticalEdgeDensityCache.clear();
    currentImageHash = "";
}

void DetectionCache::clearRegion(const cv::Rect& region)
{
    QMutexLocker locker(&cacheMutex);
    
    // Remove all cache entries for this region
    QString baseKey = generateKey(region);
    
    // Remove from all caches (keys may have threshold suffixes)
    // Use QMutableHashIterator for safe removal during iteration
    QMutableHashIterator<QString, cv::Mat> cannyIt(cannyCache);
    while (cannyIt.hasNext()) {
        cannyIt.next();
        if (cannyIt.key().startsWith(baseKey)) {
            cannyIt.remove();
        }
    }
    
    brightnessCache.remove(baseKey);
    
    QMutableHashIterator<QString, double> edgeIt(edgeDensityCache);
    while (edgeIt.hasNext()) {
        edgeIt.next();
        if (edgeIt.key().startsWith(baseKey)) {
            edgeIt.remove();
        }
    }
    
    horizontalEdgeDensityCache.remove(baseKey);
    verticalEdgeDensityCache.remove(baseKey);
}

int DetectionCache::getCacheSize() const
{
    QMutexLocker locker(&cacheMutex);
    return cannyCache.size() + brightnessCache.size() + edgeDensityCache.size() +
           horizontalEdgeDensityCache.size() + verticalEdgeDensityCache.size();
}

} // namespace ocr_orc

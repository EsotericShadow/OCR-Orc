#include "DetectionWorker.h"
#include "../../utils/RegionDetector.h"
#include <QtCore/QDebug>

namespace ocr_orc {

DetectionWorker::DetectionWorker(QObject* parent)
    : QObject(parent)
    , detector(nullptr)
{
    // Create detector in the worker thread, not in constructor
    // This will be done when first detection is requested
}

void DetectionWorker::detectRegions(const QImage& image, const QString& method) {
    if (image.isNull()) {
        emit detectionError("Invalid image provided");
        return;
    }
    
    // Create detector if not already created (lazy initialization in worker thread)
    if (!detector) {
        detector = new RegionDetector();
    }
    
    try {
        emit detectionProgress(10, "Preprocessing image...");
        
        // Run detection
        DetectionResult result = detector->detectRegions(image, method);
        
        emit detectionProgress(90, "Finalizing results...");
        
        emit detectionProgress(100, "Detection complete");
        emit detectionComplete(result);
        
    } catch (const std::exception& e) {
        emit detectionError(QString("Detection failed: %1").arg(e.what()));
    } catch (...) {
        emit detectionError("Detection failed: Unknown error");
    }
}

} // namespace ocr_orc

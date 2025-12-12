#ifndef DETECTION_WORKER_H
#define DETECTION_WORKER_H

#include <QtCore/QObject>
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <QtCore/QElapsedTimer>
#include <QtGui/QImage>
#include "../../utils/RegionDetector.h"
#include "../components/dialogs/MagicDetectParamsDialog.h"

namespace ocr_orc {

/**
 * @brief Worker thread for running region detection in background
 * 
 * Prevents UI freezing during detection operations by running
 * detection algorithms in a separate thread.
 */
class DetectionWorker : public QObject {
    Q_OBJECT

public:
    explicit DetectionWorker(QObject* parent = nullptr);
    ~DetectionWorker() = default;

public slots:
    /**
     * @brief Set detection parameters (called before detectRegions)
     * @param params Detection parameters to use
     */
    void setDetectionParameters(const DetectionParameters& params);
    
    /**
     * @brief Detect regions in an image (runs in worker thread)
     * @param image Source image to analyze
     * @param method Detection method: "auto", "grid", "contour", "hybrid"
     */
    void detectRegions(const QImage& image, const QString& method = "auto");

signals:
    /**
     * @brief Emitted during detection to report progress
     * @param percent Progress percentage (0-100)
     * @param message Status message
     */
    void detectionProgress(int percent, const QString& message);
    
    /**
     * @brief Emitted when detection completes successfully
     * @param result DetectionResult with all detected regions
     */
    void detectionComplete(const DetectionResult& result);
    
    /**
     * @brief Emitted when detection fails
     * @param error Error message describing the failure
     */
    void detectionError(const QString& error);

private slots:
    /**
     * @brief Periodic update during long OCR operations
     */
    void onProgressUpdate();

private:
    RegionDetector* detector; // Created lazily in worker thread
    QTimer* progressTimer;    // Timer for periodic progress updates during OCR
    QElapsedTimer* ocrTimer;  // Timer to track OCR elapsed time
    int progressCounter;      // Counter for progress updates (20-80%)
    DetectionParameters detectionParams; // Parameters for current detection
};

} // namespace ocr_orc

#endif // DETECTION_WORKER_H

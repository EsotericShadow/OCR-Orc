#include "DetectionWorker.h"
#include "../../utils/RegionDetector.h"
#include <QtCore/QDebug>
#include <QtCore/QElapsedTimer>
#include <QtCore/QCoreApplication>
#include <QtCore/QEventLoop>
#include <QtCore/QThread>
#include <cstdio>

namespace ocr_orc {

DetectionWorker::DetectionWorker(QObject* parent)
    : QObject(parent)
    , detector(nullptr)
    , progressTimer(nullptr)
    , ocrTimer(nullptr)
    , progressCounter(20)
    , detectionParams()
{
    fprintf(stderr, "[DetectionWorker::DetectionWorker] Constructor called\n");
    fflush(stderr);
    qDebug() << "[DetectionWorker] Constructor - detector will be created lazily";
    // Create detector in the worker thread, not in constructor
    // This will be done when first detection is requested
    
    // Create progress timer for periodic updates during OCR
    progressTimer = new QTimer(this);
    progressTimer->setSingleShot(false);
    progressTimer->setInterval(5000); // Update every 5 seconds
    connect(progressTimer, &QTimer::timeout, this, &DetectionWorker::onProgressUpdate);
}

void DetectionWorker::setDetectionParameters(const DetectionParameters& params) {
    detectionParams = params;
}

void DetectionWorker::detectRegions(const QImage& image, const QString& method) {
    fprintf(stderr, "[DetectionWorker::detectRegions] ========== DETECTION WORKER START ==========\n");
    fprintf(stderr, "[DetectionWorker::detectRegions] Entry - method: %s\n", method.toLocal8Bit().constData());
    fflush(stderr);
    qDebug() << "[DetectionWorker::detectRegions] Starting detection with method:" << method;
    
    try {
        fprintf(stderr, "[DetectionWorker::detectRegions] Step 1: Validating image...\n");
        fflush(stderr);
        if (image.isNull()) {
            fprintf(stderr, "[DetectionWorker::detectRegions] ERROR: Image is null!\n");
            fflush(stderr);
            qWarning() << "[DetectionWorker::detectRegions] Invalid image provided";
            emit detectionError("Invalid image provided");
            return;
        }
        
        fprintf(stderr, "[DetectionWorker::detectRegions] Step 1: ✓ Image valid - Size: %dx%d, Format: %d\n", 
                image.width(), image.height(), image.format());
        fflush(stderr);
        qDebug() << "[DetectionWorker::detectRegions] Image dimensions:" << image.width() << "x" << image.height();
        
        fprintf(stderr, "[DetectionWorker::detectRegions] Step 2: Creating/checking RegionDetector...\n");
        fflush(stderr);
        
        // THREAD SAFETY: Verify we're in the worker thread (Tesseract API must be created/used in same thread)
        QThread* currentThread = QThread::currentThread();
        QThread* workerThread = this->thread();
        fprintf(stderr, "[DetectionWorker::detectRegions] Thread check - current: %p, worker: %p\n", 
                (void*)currentThread, (void*)workerThread);
        fflush(stderr);
        Q_ASSERT_X(currentThread == workerThread, "DetectionWorker::detectRegions", 
                   "Tesseract API must be created/used in worker thread only!");
        
        // Create detector if not already created (lazy initialization in worker thread)
        if (!detector) {
            fprintf(stderr, "[DetectionWorker::detectRegions] Step 2.1: Creating new RegionDetector...\n");
            fflush(stderr);
            try {
                detector = new RegionDetector();
                fprintf(stderr, "[DetectionWorker::detectRegions] Step 2.1: ✓ RegionDetector created\n");
                fflush(stderr);
                qDebug() << "[DetectionWorker::detectRegions] RegionDetector created in thread:" << QThread::currentThread();
            } catch (const std::exception& e) {
                fprintf(stderr, "[DetectionWorker::detectRegions] EXCEPTION creating RegionDetector: %s\n", e.what());
                fflush(stderr);
                qCritical() << "[DetectionWorker::detectRegions] Exception creating detector:" << e.what();
                emit detectionError(QString("Failed to create detector: %1").arg(e.what()));
                return;
            } catch (...) {
                fprintf(stderr, "[DetectionWorker::detectRegions] UNKNOWN EXCEPTION creating RegionDetector\n");
                fflush(stderr);
                qCritical() << "[DetectionWorker::detectRegions] Unknown exception creating detector";
                emit detectionError("Failed to create detector: Unknown error");
                return;
            }
        } else {
            fprintf(stderr, "[DetectionWorker::detectRegions] Step 2: ✓ RegionDetector already exists\n");
            fflush(stderr);
        }
        
        fprintf(stderr, "[DetectionWorker::detectRegions] Step 3: Emitting progress (10%%)...\n");
        fflush(stderr);
        emit detectionProgress(10, "Preprocessing image...");
        
        fprintf(stderr, "[DetectionWorker::detectRegions] Step 4: Calling detector->detectRegions()...\n");
        fprintf(stderr, "[DetectionWorker::detectRegions] This may take 30-120 seconds for OCR processing\n");
        fflush(stderr);
        qDebug() << "[DetectionWorker::detectRegions] About to call detector->detectRegions()";
        
        // Emit progress update before OCR (which is the slow part)
        emit detectionProgress(20, "Starting OCR processing... This may take 1-2 minutes. Please wait...");
        fprintf(stderr, "[DetectionWorker::detectRegions] Emitted 20%% progress\n");
        fflush(stderr);
        
        // Start periodic progress updates during OCR
        progressCounter = 20;
        if (!ocrTimer) {
            ocrTimer = new QElapsedTimer();
            fprintf(stderr, "[DetectionWorker::detectRegions] Created new QElapsedTimer\n");
            fflush(stderr);
        }
        ocrTimer->restart(); // Restart timer for this detection
        fprintf(stderr, "[DetectionWorker::detectRegions] Started ocrTimer, elapsed: %lld ms\n", ocrTimer->elapsed());
        fflush(stderr);
        
        fprintf(stderr, "[DetectionWorker::detectRegions] About to start progressTimer...\n");
        fflush(stderr);
        progressTimer->start(); // Start periodic updates every 5 seconds
        fprintf(stderr, "[DetectionWorker::detectRegions] ✓ progressTimer started (interval: %d ms, isActive: %s)\n", 
                progressTimer->interval(), progressTimer->isActive() ? "true" : "false");
        fprintf(stderr, "[DetectionWorker::detectRegions] Current thread: %p\n", (void*)QThread::currentThread());
        fprintf(stderr, "[DetectionWorker::detectRegions] Processing events to ensure timer can fire...\n");
        fflush(stderr);
        
        // Process events to ensure timer can start
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
        
        DetectionResult result;
        QElapsedTimer detectionTimer;
        detectionTimer.start();
        
        // Since OCR blocks the thread, we'll use a different approach:
        // Emit progress updates manually at key points in the OCR pipeline
        // The timer approach won't work because the event loop is blocked
        
        try {
            fprintf(stderr, "[DetectionWorker::detectRegions] About to call detector->detectRegions() - this will block...\n");
            fprintf(stderr, "[DetectionWorker::detectRegions] Using custom parameters:\n");
            fprintf(stderr, "  - OCR overlap: %.2f, Edge density: %.3f\n",
                    detectionParams.ocrOverlapThreshold, detectionParams.edgeDensityThreshold);
            fprintf(stderr, "  - Checkbox size: %d-%dpx, Aspect ratio: %.1f-%.1f, Rectangularity: %.2f\n",
                    detectionParams.minCheckboxSize, detectionParams.maxCheckboxSize,
                    detectionParams.checkboxAspectRatioMin, detectionParams.checkboxAspectRatioMax,
                    detectionParams.checkboxRectangularity);
            fprintf(stderr, "  - Standalone checkbox detection: %s\n",
                    detectionParams.enableStandaloneCheckboxDetection ? "ENABLED" : "DISABLED");
            fflush(stderr);
            result = detector->detectRegions(image, method, detectionParams);
            fprintf(stderr, "[DetectionWorker::detectRegions] ✓ detector->detectRegions() returned\n");
            fflush(stderr);
            
            // Stop progress timer now that OCR is done
            progressTimer->stop();
            fprintf(stderr, "[DetectionWorker::detectRegions] Stopped progress timer\n");
            fflush(stderr);
            qint64 elapsedMs = detectionTimer.elapsed();
            fprintf(stderr, "[DetectionWorker::detectRegions] Step 4: ✓ detectRegions() returned (took %lld ms)\n", elapsedMs);
            fprintf(stderr, "[DetectionWorker::detectRegions] Result: %d regions detected\n", result.totalDetected);
            fflush(stderr);
            qDebug() << "[DetectionWorker::detectRegions] Detection complete - regions:" << result.totalDetected << "in" << (elapsedMs / 1000.0) << "seconds";
        } catch (const std::exception& e) {
            // Stop progress timer on error
            progressTimer->stop();
            fprintf(stderr, "[DetectionWorker::detectRegions] EXCEPTION in detectRegions(): %s\n", e.what());
            fflush(stderr);
            qCritical() << "[DetectionWorker::detectRegions] Exception in detectRegions:" << e.what();
            emit detectionError(QString("Detection failed: %1").arg(e.what()));
            return;
        } catch (...) {
            // Stop progress timer on error
            progressTimer->stop();
            fprintf(stderr, "[DetectionWorker::detectRegions] UNKNOWN EXCEPTION in detectRegions()\n");
            fflush(stderr);
            qCritical() << "[DetectionWorker::detectRegions] Unknown exception in detectRegions";
            emit detectionError("Detection failed: Unknown error in detection algorithm");
            return;
        }
        
        fprintf(stderr, "[DetectionWorker::detectRegions] Step 5: Emitting progress (90%%)...\n");
        fflush(stderr);
        emit detectionProgress(90, "Finalizing results...");
        
        fprintf(stderr, "[DetectionWorker::detectRegions] Step 6: Emitting progress (100%%)...\n");
        fflush(stderr);
        emit detectionProgress(100, "Detection complete");
        
        fprintf(stderr, "[DetectionWorker::detectRegions] Step 7: Emitting detectionComplete signal...\n");
        fprintf(stderr, "[DetectionWorker::detectRegions] Result summary: total=%d, high=%d, medium=%d, low=%d\n",
                result.totalDetected, result.highConfidence, result.mediumConfidence, result.lowConfidence);
        fflush(stderr);
        qDebug() << "[DetectionWorker::detectRegions] Emitting detectionComplete with" << result.totalDetected << "regions";
        
        emit detectionComplete(result);
        
        fprintf(stderr, "[DetectionWorker::detectRegions] Step 7: ✓ Signal emitted\n");
        fprintf(stderr, "[DetectionWorker::detectRegions] ========== DETECTION WORKER SUCCESS ==========\n");
        fflush(stderr);
        qDebug() << "[DetectionWorker::detectRegions] Detection completed successfully";
        
    } catch (const std::exception& e) {
        // Stop progress timer on error
        if (progressTimer) {
            progressTimer->stop();
        }
        fprintf(stderr, "[DetectionWorker::detectRegions] CRITICAL EXCEPTION: %s\n", e.what());
        fflush(stderr);
        qCritical() << "[DetectionWorker::detectRegions] Critical exception:" << e.what();
        emit detectionError(QString("Detection failed: %1").arg(e.what()));
    } catch (...) {
        // Stop progress timer on error
        if (progressTimer) {
            progressTimer->stop();
        }
        fprintf(stderr, "[DetectionWorker::detectRegions] CRITICAL UNKNOWN EXCEPTION\n");
        fflush(stderr);
        qCritical() << "[DetectionWorker::detectRegions] Critical unknown exception";
        emit detectionError("Detection failed: Unknown error");
    }
}

void DetectionWorker::onProgressUpdate() {
    fprintf(stderr, "[DetectionWorker::onProgressUpdate] ========== TIMER FIRED ==========\n");
    fflush(stderr);
    
    if (!ocrTimer) {
        fprintf(stderr, "[DetectionWorker::onProgressUpdate] ERROR: ocrTimer is null!\n");
        fflush(stderr);
        return;
    }
    
    if (!ocrTimer->isValid()) {
        fprintf(stderr, "[DetectionWorker::onProgressUpdate] ERROR: ocrTimer is not valid!\n");
        fflush(stderr);
        return;
    }
    
    qint64 elapsedSeconds = ocrTimer->elapsed() / 1000;
    fprintf(stderr, "[DetectionWorker::onProgressUpdate] Elapsed: %lld seconds\n", elapsedSeconds);
    fflush(stderr);
    
    // Increment progress counter gradually (20% -> 30% -> 40% -> ... -> 80%)
    // This gives visual feedback that something is happening
    if (progressCounter < 80) {
        progressCounter += 5; // Increment by 5% each update (every 5 seconds)
    }
    
    // Create status message with elapsed time
    QString message;
    if (elapsedSeconds < 30) {
        message = QString("OCR processing... (%1 seconds elapsed)").arg(elapsedSeconds);
    } else if (elapsedSeconds < 60) {
        message = QString("OCR processing... (%1 seconds - still working, please wait)").arg(elapsedSeconds);
    } else {
        int minutes = elapsedSeconds / 60;
        int seconds = elapsedSeconds % 60;
        message = QString("OCR processing... (%1m %2s - this is normal for large documents)").arg(minutes).arg(seconds);
    }
    
    fprintf(stderr, "[DetectionWorker::onProgressUpdate] Progress: %d%% - %s\n", progressCounter, message.toLocal8Bit().constData());
    fflush(stderr);
    
    emit detectionProgress(progressCounter, message);
}

} // namespace ocr_orc

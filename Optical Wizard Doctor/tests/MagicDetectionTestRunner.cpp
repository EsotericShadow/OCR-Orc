#include "MagicDetectionTestRunner.h"
#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QDateTime>
#include <QtCore/QElapsedTimer>
#include <QtGui/QImage>
#include <cstdio>

// #region agent log
static void debugLog(const QString& location, const QString& message, const QVariantMap& data = QVariantMap()) {
    QFile logFile("/Users/main/Desktop/OWD-OCR_Coordinates/.cursor/debug.log");
    if (logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&logFile);
        QJsonObject logObj;
        logObj["timestamp"] = QDateTime::currentMSecsSinceEpoch();
        logObj["location"] = location;
        logObj["message"] = message;
        QJsonObject dataObj;
        for (auto it = data.begin(); it != data.end(); ++it) {
            dataObj[it.key()] = QJsonValue::fromVariant(it.value());
        }
        logObj["data"] = dataObj;
        logObj["sessionId"] = "debug-session";
        out << QJsonDocument(logObj).toJson(QJsonDocument::Compact) << "\n";
        logFile.close();
    }
}
// #endregion

namespace ocr_orc {

MagicDetectionTestRunner::MagicDetectionTestRunner()
    : detectionMethod("ocr-first")  // Default to OCR-first
{
    // #region agent log
    debugLog("MagicDetectionTestRunner.cpp:constructor", "Entry");
    // #endregion
    
    dataManager.setBaseDirectory("tests/data");
    
    // #region agent log
    debugLog("MagicDetectionTestRunner.cpp:constructor", "After setBaseDirectory");
    // #endregion
    
    instrumentation.setEnabled(true);
    
    // #region agent log
    debugLog("MagicDetectionTestRunner.cpp:constructor", "After instrumentation setup");
    // #endregion
    
    // Set instrumentation pointer - RegionDetector will cast it back to PipelineInstrumentation*
    detector.setInstrumentation(static_cast<void*>(&instrumentation));
    
    // #region agent log
    debugLog("MagicDetectionTestRunner.cpp:constructor", "After setInstrumentation");
    // #endregion
}

QMap<QString, EvaluationMetrics> MagicDetectionTestRunner::runTestSuite()
{
    QMap<QString, EvaluationMetrics> results;
    
    qDebug() << "Discovering available test forms...";
    fflush(stdout);
    
    QList<QString> forms = dataManager.getAvailableForms();
    
    if (forms.isEmpty()) {
        qWarning() << "WARNING: No test forms found!";
        qWarning() << "Forms directory:" << dataManager.getFormsDirectory();
        qWarning() << "Ground truth directory:" << dataManager.getGroundTruthDirectory();
        return results;
    }
    
    qDebug() << "Found" << forms.size() << "test form(s):" << forms;
    qDebug() << "";
    fflush(stdout);
    
    int formIndex = 0;
    for (const QString& formId : forms) {
        formIndex++;
        qDebug() << "========================================";
        qDebug() << "[" << formIndex << "/" << forms.size() << "] Testing form:" << formId;
        qDebug() << "========================================";
        fflush(stdout);
        
        // #region agent log
        debugLog("MagicDetectionTestRunner.cpp:runTestSuite", "Starting test", {{"formId", formId}, {"formIndex", formIndex}, {"totalForms", forms.size()}});
        // #endregion
        
        EvaluationMetrics metrics = runTest(formId);
        results[formId] = metrics;
        
        qDebug() << "";
        qDebug() << "✓ Test completed for" << formId;
        qDebug() << "  Precision:" << QString::number(metrics.precision, 'f', 4);
        qDebug() << "  Recall:" << QString::number(metrics.recall, 'f', 4);
        qDebug() << "  F1 Score:" << QString::number(metrics.f1Score, 'f', 4);
        qDebug() << "  Mean IoU:" << QString::number(metrics.meanIoU, 'f', 4);
        qDebug() << "";
        fflush(stdout);
        
        // #region agent log
        debugLog("MagicDetectionTestRunner.cpp:runTestSuite", "Completed test", {{"formId", formId}, {"precision", metrics.precision}, {"recall", metrics.recall}});
        // #endregion
    }
    
    qDebug() << "========================================";
    qDebug() << "✓ All tests completed!";
    qDebug() << "========================================";
    fflush(stdout);
    
    return results;
}

EvaluationMetrics MagicDetectionTestRunner::runTest(const QString& formId)
{
    GroundTruthAnnotation groundTruth = dataManager.getGroundTruth(formId);
    if (groundTruth.formId.isEmpty()) {
        qWarning() << "Failed to load ground truth for form:" << formId;
        return EvaluationMetrics();
    }
    
    return runSingleTest(groundTruth);
}

GroundTruthAnnotation MagicDetectionTestRunner::getGroundTruth(const QString& formId) const
{
    return dataManager.getGroundTruth(formId);
}

void MagicDetectionTestRunner::setBaseDirectory(const QString& baseDir)
{
    dataManager.setBaseDirectory(baseDir);
}

void MagicDetectionTestRunner::setInstrumentationEnabled(bool enabled)
{
    instrumentation.setEnabled(enabled);
}

void MagicDetectionTestRunner::setDetectionMethod(const QString& method)
{
    detectionMethod = method;
}

EvaluationMetrics MagicDetectionTestRunner::runSingleTest(const GroundTruthAnnotation& groundTruth)
{
    // Clear previous instrumentation data
    instrumentation.clear();
    
    // Load image
    QString imagePath = dataManager.getFormsDirectory() + "/" + groundTruth.imagePath;
    qDebug() << "Step 1/4: Loading image from:" << imagePath;
    fflush(stdout);
    
    // #region agent log
    debugLog("MagicDetectionTestRunner.cpp:runSingleTest", "Loading image", {{"imagePath", imagePath}});
    // #endregion
    
    QImage image = dataManager.loadImage(imagePath);
    if (image.isNull()) {
        qWarning() << "✗ Failed to load image:" << imagePath;
        return EvaluationMetrics();
    }
    
    qDebug() << "✓ Image loaded:" << image.width() << "x" << image.height() << "pixels";
    fflush(stdout);
    
    qDebug() << "Step 2/4: Starting OCR-first detection pipeline...";
    qDebug() << "  (This may take 30-90 seconds depending on image complexity)";
    qDebug() << "  Please wait - processing in progress...";
    fflush(stdout);
    fflush(stdout);
    
    // #region agent log
    debugLog("MagicDetectionTestRunner.cpp:runSingleTest", "Before detectRegionsOCRFirst", {{"imageWidth", image.width()}, {"imageHeight", image.height()}});
    // #endregion
    
    // Run detection pipeline with progress indication
    QElapsedTimer timer;
    timer.start();
    
    // Run detection pipeline based on selected method
    // #region agent log
    debugLog("MagicDetectionTestRunner.cpp:runSingleTest", "Before detection call", {{"method", detectionMethod}});
    // #endregion
    
    qDebug() << "  Detection method:" << detectionMethod;
    fflush(stdout);
    
    DetectionResult detectionResult;
    if (detectionMethod == "ocr-first") {
        qDebug() << "  → Calling detectRegionsOCRFirst()...";
        fflush(stdout);
        
        // #region agent log
        debugLog("MagicDetectionTestRunner.cpp:runSingleTest", "Calling detectRegionsOCRFirst");
        // #endregion
        
        detectionResult = detector.detectRegionsOCRFirst(image, "ocr-first");
        
        // #region agent log
        debugLog("MagicDetectionTestRunner.cpp:runSingleTest", "detectRegionsOCRFirst returned", {{"regions", detectionResult.regions.size()}});
        // #endregion
    } else {
        qDebug() << "  → Calling detectRegions() with method:" << detectionMethod;
        fflush(stdout);
        
        // #region agent log
        debugLog("MagicDetectionTestRunner.cpp:runSingleTest", "Calling detectRegions", {{"method", detectionMethod}});
        // #endregion
        
        detectionResult = detector.detectRegions(image, detectionMethod);
        
        // #region agent log
        debugLog("MagicDetectionTestRunner.cpp:runSingleTest", "detectRegions returned", {{"regions", detectionResult.regions.size()}});
        // #endregion
    }
    
    // #region agent log
    debugLog("MagicDetectionTestRunner.cpp:runSingleTest", "After detection call, before timer check");
    // #endregion
    
    qint64 elapsed = timer.elapsed();
    
    // #region agent log
    debugLog("MagicDetectionTestRunner.cpp:runSingleTest", "Timer elapsed calculated", {{"elapsedMs", elapsed}});
    // #endregion
    
    qDebug() << "  ✓ Processing completed in" << QString::number(elapsed / 1000.0, 'f', 2) << "seconds";
    fflush(stdout);
    
    // #region agent log
    debugLog("MagicDetectionTestRunner.cpp:runSingleTest", "Before detection complete message");
    // #endregion
    
    qDebug() << "✓ Detection complete! Found" << detectionResult.regions.size() << "region(s)";
    fflush(stdout);
    
    // #region agent log
    debugLog("MagicDetectionTestRunner.cpp:runSingleTest", "After detection", {{"regionsFound", detectionResult.regions.size()}, {"method", detectionMethod}});
    // #endregion
    
    qDebug() << "Step 3/4: Calculating metrics...";
    qDebug() << "  Detected regions:" << detectionResult.regions.size();
    qDebug() << "  Expected regions:" << groundTruth.expectedRegions.size();
    fflush(stdout);
    
    // #region agent log
    debugLog("MagicDetectionTestRunner.cpp:runSingleTest", "Before calculateMetrics", {{"detectedCount", detectionResult.regions.size()}, {"expectedCount", groundTruth.expectedRegions.size()}});
    // #endregion
    
    // Calculate metrics
    EvaluationMetrics metrics = metricsCalculator.calculateMetrics(
        detectionResult.regions, groundTruth, image.width(), image.height());
    
    // #region agent log
    debugLog("MagicDetectionTestRunner.cpp:runSingleTest", "After calculateMetrics", {{"precision", metrics.precision}, {"recall", metrics.recall}});
    // #endregion
    
    qDebug() << "✓ Metrics calculated - Precision:" << QString::number(metrics.precision, 'f', 4) 
             << "Recall:" << QString::number(metrics.recall, 'f', 4);
    fflush(stdout);
    
    qDebug() << "Step 4/4: Gathering performance data...";
    fflush(stdout);
    
    // #region agent log
    debugLog("MagicDetectionTestRunner.cpp:runSingleTest", "Before performance data gathering");
    // #endregion
    
    // Add performance metrics from instrumentation
    metrics.performance.totalTimeMs = instrumentation.getTotalExecutionTime();
    QMap<QString, StageMetrics> stageMetrics = instrumentation.getAllStageMetrics();
    for (auto it = stageMetrics.begin(); it != stageMetrics.end(); ++it) {
        metrics.performance.stageTimings[it.key()] = it.value().totalTimeMs;
    }
    
    // Add cache statistics
    QMap<QString, QPair<int, int>> cacheStats = instrumentation.getCacheStatistics();
    for (auto it = cacheStats.begin(); it != cacheStats.end(); ++it) {
        metrics.performance.cacheHits[it.key()] = it.value().first;
        metrics.performance.cacheMisses[it.key()] = it.value().second;
    }
    
    // Add stage metrics
    metrics.stageMetrics = stageMetrics;
    
    return metrics;
}

} // namespace ocr_orc

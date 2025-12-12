#ifndef MAGIC_DETECTION_TEST_RUNNER_H
#define MAGIC_DETECTION_TEST_RUNNER_H

#include "TestDataManager.h"
#include "instrumentation/PipelineInstrumentation.h"
#include "metrics/MetricsCalculator.h"
#include "reporting/TestReporter.h"
#include "../../src/utils/RegionDetector.h"
#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QMap>

namespace ocr_orc {

/**
 * @brief Test runner for magic detection pipeline
 * 
 * Orchestrates full test suite execution with instrumentation,
 * metrics calculation, and report generation.
 */
class MagicDetectionTestRunner {
public:
    MagicDetectionTestRunner();
    ~MagicDetectionTestRunner() = default;
    
    /**
     * @brief Run test suite on all available forms
     * @return Map of form ID to evaluation metrics
     */
    QMap<QString, EvaluationMetrics> runTestSuite();
    
    /**
     * @brief Run test on a single form
     * @param formId Form ID to test
     * @return Evaluation metrics
     */
    EvaluationMetrics runTest(const QString& formId);
    
    /**
     * @brief Set base directory for test data
     * @param baseDir Base directory
     */
    void setBaseDirectory(const QString& baseDir);
    
    /**
     * @brief Enable or disable instrumentation
     * @param enabled True to enable instrumentation
     */
    void setInstrumentationEnabled(bool enabled);
    
    /**
     * @brief Set detection method (for testing without OCR)
     * @param method Detection method: "ocr-first", "hybrid", "grid", "contour"
     */
    void setDetectionMethod(const QString& method);
    
    /**
     * @brief Get instrumentation instance
     * @return Instrumentation instance
     */
    PipelineInstrumentation* getInstrumentation() { return &instrumentation; }
    
    /**
     * @brief Get test reporter instance
     * @return Test reporter instance
     */
    TestReporter* getReporter() { return &reporter; }
    
    /**
     * @brief Get ground truth for a form (for reporting)
     * @param formId Form ID
     * @return Ground truth annotation
     */
    GroundTruthAnnotation getGroundTruth(const QString& formId) const;

private:
    TestDataManager dataManager;
    PipelineInstrumentation instrumentation;
    MetricsCalculator metricsCalculator;
    TestReporter reporter;
    RegionDetector detector;
    QString detectionMethod;  // Detection method to use
    
    EvaluationMetrics runSingleTest(const GroundTruthAnnotation& groundTruth);
};

} // namespace ocr_orc

#endif // MAGIC_DETECTION_TEST_RUNNER_H

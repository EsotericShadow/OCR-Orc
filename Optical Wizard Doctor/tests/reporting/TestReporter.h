#ifndef TEST_REPORTER_H
#define TEST_REPORTER_H

#include "../metrics/MetricsCalculator.h"
#include "../TestDataManager.h"
#include <QtCore/QString>
#include <QtCore/QMap>
#include <QtCore/QDateTime>

namespace ocr_orc {

/**
 * @brief Test result reporter for saving and displaying test results
 * 
 * Handles persistence of test results to JSON/CSV files and provides
 * detailed console output with per-region analysis.
 */
class TestReporter {
public:
    TestReporter();
    ~TestReporter() = default;
    
    /**
     * @brief Save test results to JSON file
     * @param results Map of form ID to evaluation metrics
     * @param outputPath Path to output JSON file
     * @return True if successful
     */
    bool saveResultsToJson(const QMap<QString, EvaluationMetrics>& results, 
                          const QString& outputPath) const;
    
    /**
     * @brief Save test results to CSV file
     * @param results Map of form ID to evaluation metrics
     * @param outputPath Path to output CSV file
     * @return True if successful
     */
    bool saveResultsToCsv(const QMap<QString, EvaluationMetrics>& results,
                         const QString& outputPath) const;
    
    /**
     * @brief Save single test result to JSON file
     * @param formId Form ID
     * @param metrics Evaluation metrics
     * @param outputPath Path to output JSON file
     * @return True if successful
     */
    bool saveSingleResultToJson(const QString& formId, const EvaluationMetrics& metrics,
                                const QString& outputPath) const;
    
    /**
     * @brief Print detailed test results to console
     * @param formId Form ID
     * @param metrics Evaluation metrics
     * @param groundTruth Ground truth annotation (for region names)
     */
    void printDetailedResults(const QString& formId, const EvaluationMetrics& metrics,
                             const GroundTruthAnnotation& groundTruth) const;
    
    /**
     * @brief Print summary of test suite results
     * @param results Map of form ID to evaluation metrics
     */
    void printSummary(const QMap<QString, EvaluationMetrics>& results) const;
    
    /**
     * @brief Calculate overall test suite pass/fail status
     * @param results Map of form ID to evaluation metrics
     * @param minPrecision Minimum precision threshold (default: 0.7)
     * @param minRecall Minimum recall threshold (default: 0.7)
     * @param minF1 Minimum F1 threshold (default: 0.7)
     * @return True if all tests pass thresholds
     */
    bool calculatePassStatus(const QMap<QString, EvaluationMetrics>& results,
                            double minPrecision = 0.7,
                            double minRecall = 0.7,
                            double minF1 = 0.7) const;
    
    /**
     * @brief Generate HTML report with charts and visualizations
     * @param results Map of form ID to evaluation metrics
     * @param outputPath Path to output HTML file
     * @param minPrecision Minimum precision threshold for pass/fail
     * @param minRecall Minimum recall threshold for pass/fail
     * @param minF1 Minimum F1 threshold for pass/fail
     * @return True if successful
     */
    bool generateHtmlReport(const QMap<QString, EvaluationMetrics>& results,
                           const QString& outputPath,
                           double minPrecision = 0.7,
                           double minRecall = 0.7,
                           double minF1 = 0.7) const;

private:
    QString formatRegionEvaluation(const RegionEvaluation& eval, const QString& regionName) const;
    QString formatStageMetrics(const QMap<QString, StageMetrics>& stageMetrics) const;
    QString formatPerformanceMetrics(const PerformanceMetrics& perf) const;
    QString generateJsArray(const QList<QString>& values) const;
    QString generateJsArray(const QList<double>& values) const;
};

} // namespace ocr_orc

#endif // TEST_REPORTER_H

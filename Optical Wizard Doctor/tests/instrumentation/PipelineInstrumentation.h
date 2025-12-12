#ifndef PIPELINE_INSTRUMENTATION_H
#define PIPELINE_INSTRUMENTATION_H

#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QVariantMap>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QMutex>
#include <QtCore/QElapsedTimer>
#include <QtCore/QDateTime>

namespace ocr_orc {

/**
 * @brief Pipeline event for tracking function calls and intermediate results
 */
struct PipelineEvent {
    QString stageName;
    QString functionName;
    qint64 timestamp;              // Microseconds since epoch
    QVariantMap inputs;            // Input parameters and data
    QVariantMap outputs;           // Output results
    QVariantMap metadata;          // Thresholds, parameters, decisions
    double executionTimeMs;        // Execution time in milliseconds
    
    PipelineEvent()
        : timestamp(0), executionTimeMs(0.0) {}
};

/**
 * @brief Metrics for a pipeline stage
 */
struct StageMetrics {
    QString stageName;
    int inputCount;
    int outputCount;
    double totalTimeMs;
    double avgTimeMs;
    double minTimeMs;
    double maxTimeMs;
    QMap<QString, QVariant> stageSpecificMetrics;  // Stage-specific metrics
    
    StageMetrics()
        : inputCount(0), outputCount(0), totalTimeMs(0.0), avgTimeMs(0.0),
          minTimeMs(std::numeric_limits<double>::max()),
          maxTimeMs(0.0) {}
};

/**
 * @brief Comprehensive pipeline instrumentation system
 * 
 * Tracks every function call, threshold decision, and intermediate result
 * throughout the magic detection pipeline. Thread-safe for parallel processing.
 */
class PipelineInstrumentation {
public:
    PipelineInstrumentation();
    ~PipelineInstrumentation() = default;
    
    /**
     * @brief Enable or disable instrumentation
     */
    void setEnabled(bool enabled) { this->enabled = enabled; }
    bool isEnabled() const { return enabled; }
    
    /**
     * @brief Start tracking a pipeline stage
     * @param stageName Name of the stage (e.g., "Stage 1: OCR Extraction")
     */
    void startStage(const QString& stageName);
    
    /**
     * @brief End tracking a pipeline stage
     * @param stageName Name of the stage
     */
    void endStage(const QString& stageName);
    
    /**
     * @brief Log an event within a stage
     * @param functionName Name of the function being called
     * @param inputs Input parameters and data
     * @param outputs Output results
     * @param metadata Additional metadata (thresholds, decisions, etc.)
     */
    void logEvent(const QString& functionName,
                  const QVariantMap& inputs = QVariantMap(),
                  const QVariantMap& outputs = QVariantMap(),
                  const QVariantMap& metadata = QVariantMap());
    
    /**
     * @brief Log a threshold decision
     * @param thresholdName Name of the threshold (e.g., "brightness_threshold")
     * @param value Threshold value used
     * @param context Context information (document type, region, etc.)
     */
    void logThresholdDecision(const QString& thresholdName, double value,
                              const QVariantMap& context = QVariantMap());
    
    /**
     * @brief Log cache hit/miss
     * @param cacheType Type of cache (e.g., "canny_edges", "brightness")
     * @param hit True if cache hit, false if miss
     * @param key Cache key used
     */
    void logCacheEvent(const QString& cacheType, bool hit, const QString& key = "");
    
    /**
     * @brief Get all events for a specific stage
     * @param stageName Stage name
     * @return List of events
     */
    QList<PipelineEvent> getStageEvents(const QString& stageName) const;
    
    /**
     * @brief Get metrics for a specific stage
     * @param stageName Stage name
     * @return Stage metrics
     */
    StageMetrics getStageMetrics(const QString& stageName) const;
    
    /**
     * @brief Get all stage metrics
     * @return Map of stage name to metrics
     */
    QMap<QString, StageMetrics> getAllStageMetrics() const;
    
    /**
     * @brief Get all events
     * @return List of all events
     */
    QList<PipelineEvent> getAllEvents() const;
    
    /**
     * @brief Get threshold decisions
     * @return Map of threshold name to list of decisions
     */
    QMap<QString, QList<QVariantMap>> getThresholdDecisions() const;
    
    /**
     * @brief Get cache statistics
     * @return Map of cache type to hit/miss counts
     */
    QMap<QString, QPair<int, int>> getCacheStatistics() const;  // hits, misses
    
    /**
     * @brief Clear all instrumentation data
     */
    void clear();
    
    /**
     * @brief Export events to JSON
     * @param filePath Path to output JSON file
     */
    void exportToJson(const QString& filePath) const;
    
    /**
     * @brief Get total execution time
     * @return Total time in milliseconds
     */
    double getTotalExecutionTime() const;

private:
    bool enabled;
    QList<PipelineEvent> events;
    QMap<QString, StageMetrics> stageMetricsMap;
    QMap<QString, QList<QVariantMap>> thresholdDecisions;
    QMap<QString, QPair<int, int>> cacheStats;  // cache type -> (hits, misses)
    
    mutable QMutex mutex;
    QElapsedTimer totalTimer;
    QMap<QString, QElapsedTimer> stageTimers;
    QMap<QString, qint64> stageStartTimes;
    
    QString currentStage;
    
    void updateStageMetrics(const QString& stageName, const PipelineEvent& event);
    qint64 getCurrentTimestamp() const;
};

} // namespace ocr_orc

#endif // PIPELINE_INSTRUMENTATION_H

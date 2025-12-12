#include "PipelineInstrumentation.h"
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtCore/QFile>
#include <QtCore/QDebug>
#include <algorithm>
#include <limits>

namespace ocr_orc {

PipelineInstrumentation::PipelineInstrumentation()
    : enabled(true)
    , currentStage("")
{
    totalTimer.start();
}

void PipelineInstrumentation::startStage(const QString& stageName)
{
    if (!enabled) return;
    
    QMutexLocker locker(&mutex);
    
    currentStage = stageName;
    stageStartTimes[stageName] = getCurrentTimestamp();
    
    if (!stageTimers.contains(stageName)) {
        stageTimers[stageName] = QElapsedTimer();
    }
    stageTimers[stageName].start();
    
    // Initialize stage metrics if not exists
    if (!stageMetricsMap.contains(stageName)) {
        stageMetricsMap[stageName] = StageMetrics();
        stageMetricsMap[stageName].stageName = stageName;
    }
    
    // Log stage start event
    PipelineEvent event;
    event.stageName = stageName;
    event.functionName = "startStage";
    event.timestamp = getCurrentTimestamp();
    event.metadata["action"] = "stage_start";
    events.append(event);
}

void PipelineInstrumentation::endStage(const QString& stageName)
{
    if (!enabled) return;
    
    QMutexLocker locker(&mutex);
    
    if (!stageTimers.contains(stageName)) {
        return;  // Stage was never started
    }
    
    qint64 elapsedMs = stageTimers[stageName].elapsed();
    
    // Update stage metrics
    StageMetrics& metrics = stageMetricsMap[stageName];
    metrics.totalTimeMs += elapsedMs;
    metrics.avgTimeMs = metrics.totalTimeMs / (metrics.inputCount > 0 ? metrics.inputCount : 1);
    metrics.minTimeMs = std::min(metrics.minTimeMs, static_cast<double>(elapsedMs));
    metrics.maxTimeMs = std::max(metrics.maxTimeMs, static_cast<double>(elapsedMs));
    
    // Log stage end event
    PipelineEvent event;
    event.stageName = stageName;
    event.functionName = "endStage";
    event.timestamp = getCurrentTimestamp();
    event.executionTimeMs = elapsedMs;
    event.metadata["action"] = "stage_end";
    event.metadata["stage_duration_ms"] = elapsedMs;
    events.append(event);
    
    if (currentStage == stageName) {
        currentStage = "";
    }
}

void PipelineInstrumentation::logEvent(const QString& functionName,
                                       const QVariantMap& inputs,
                                       const QVariantMap& outputs,
                                       const QVariantMap& metadata)
{
    if (!enabled) return;
    
    QMutexLocker locker(&mutex);
    
    PipelineEvent event;
    event.stageName = currentStage.isEmpty() ? "Unknown" : currentStage;
    event.functionName = functionName;
    event.timestamp = getCurrentTimestamp();
    event.inputs = inputs;
    event.outputs = outputs;
    event.metadata = metadata;
    
    // Calculate execution time if timer exists for this function
    static QMap<QString, QElapsedTimer> functionTimers;
    if (functionTimers.contains(functionName)) {
        event.executionTimeMs = functionTimers[functionName].elapsed();
        functionTimers.remove(functionName);
    }
    
    events.append(event);
    updateStageMetrics(event.stageName, event);
}

void PipelineInstrumentation::logThresholdDecision(const QString& thresholdName, double value,
                                                   const QVariantMap& context)
{
    if (!enabled) return;
    
    QMutexLocker locker(&mutex);
    
    QVariantMap decision;
    decision["threshold_name"] = thresholdName;
    decision["value"] = value;
    decision["timestamp"] = getCurrentTimestamp();
    decision["stage"] = currentStage.isEmpty() ? "Unknown" : currentStage;
    decision["context"] = context;
    
    if (!thresholdDecisions.contains(thresholdName)) {
        thresholdDecisions[thresholdName] = QList<QVariantMap>();
    }
    thresholdDecisions[thresholdName].append(decision);
    
    // Also log as regular event
    QVariantMap metadata;
    metadata["threshold_name"] = thresholdName;
    metadata["threshold_value"] = value;
    // Merge context into metadata
    for (auto it = context.begin(); it != context.end(); ++it) {
        metadata[it.key()] = it.value();
    }
    
    logEvent("threshold_decision", QVariantMap(), QVariantMap(), metadata);
}

void PipelineInstrumentation::logCacheEvent(const QString& cacheType, bool hit, const QString& key)
{
    if (!enabled) return;
    
    QMutexLocker locker(&mutex);
    
    if (!cacheStats.contains(cacheType)) {
        cacheStats[cacheType] = QPair<int, int>(0, 0);
    }
    
    QPair<int, int>& stats = cacheStats[cacheType];
    if (hit) {
        stats.first++;  // hits
    } else {
        stats.second++;  // misses
    }
    
    // Also log as regular event
    QVariantMap metadata;
    metadata["cache_type"] = cacheType;
    metadata["cache_hit"] = hit;
    if (!key.isEmpty()) {
        metadata["cache_key"] = key;
    }
    
    logEvent("cache_access", QVariantMap(), QVariantMap(), metadata);
}

QList<PipelineEvent> PipelineInstrumentation::getStageEvents(const QString& stageName) const
{
    QMutexLocker locker(&mutex);
    
    QList<PipelineEvent> stageEvents;
    for (const PipelineEvent& event : events) {
        if (event.stageName == stageName) {
            stageEvents.append(event);
        }
    }
    return stageEvents;
}

StageMetrics PipelineInstrumentation::getStageMetrics(const QString& stageName) const
{
    QMutexLocker locker(&mutex);
    
    if (stageMetricsMap.contains(stageName)) {
        return stageMetricsMap[stageName];
    }
    return StageMetrics();
}

QMap<QString, StageMetrics> PipelineInstrumentation::getAllStageMetrics() const
{
    QMutexLocker locker(&mutex);
    return stageMetricsMap;
}

QList<PipelineEvent> PipelineInstrumentation::getAllEvents() const
{
    QMutexLocker locker(&mutex);
    return events;
}

QMap<QString, QList<QVariantMap>> PipelineInstrumentation::getThresholdDecisions() const
{
    QMutexLocker locker(&mutex);
    return thresholdDecisions;
}

QMap<QString, QPair<int, int>> PipelineInstrumentation::getCacheStatistics() const
{
    QMutexLocker locker(&mutex);
    return cacheStats;
}

void PipelineInstrumentation::clear()
{
    QMutexLocker locker(&mutex);
    events.clear();
    stageMetricsMap.clear();
    thresholdDecisions.clear();
    cacheStats.clear();
    stageTimers.clear();
    stageStartTimes.clear();
    currentStage = "";
    totalTimer.restart();
}

void PipelineInstrumentation::exportToJson(const QString& filePath) const
{
    QMutexLocker locker(&mutex);
    
    QJsonObject root;
    root["total_execution_time_ms"] = totalTimer.elapsed();
    root["total_events"] = events.size();
    root["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    // Export events
    QJsonArray eventsArray;
    for (const PipelineEvent& event : events) {
        QJsonObject eventObj;
        eventObj["stage_name"] = event.stageName;
        eventObj["function_name"] = event.functionName;
        eventObj["timestamp"] = event.timestamp;
        eventObj["execution_time_ms"] = event.executionTimeMs;
        
        QJsonObject inputsObj;
        for (auto it = event.inputs.begin(); it != event.inputs.end(); ++it) {
            inputsObj[it.key()] = QJsonValue::fromVariant(it.value());
        }
        eventObj["inputs"] = inputsObj;
        
        QJsonObject outputsObj;
        for (auto it = event.outputs.begin(); it != event.outputs.end(); ++it) {
            outputsObj[it.key()] = QJsonValue::fromVariant(it.value());
        }
        eventObj["outputs"] = outputsObj;
        
        QJsonObject metadataObj;
        for (auto it = event.metadata.begin(); it != event.metadata.end(); ++it) {
            metadataObj[it.key()] = QJsonValue::fromVariant(it.value());
        }
        eventObj["metadata"] = metadataObj;
        
        eventsArray.append(eventObj);
    }
    root["events"] = eventsArray;
    
    // Export stage metrics
    QJsonObject metricsObj;
    for (auto it = stageMetricsMap.begin(); it != stageMetricsMap.end(); ++it) {
        const StageMetrics& metrics = it.value();
        QJsonObject stageObj;
        stageObj["input_count"] = metrics.inputCount;
        stageObj["output_count"] = metrics.outputCount;
        stageObj["total_time_ms"] = metrics.totalTimeMs;
        stageObj["avg_time_ms"] = metrics.avgTimeMs;
        stageObj["min_time_ms"] = metrics.minTimeMs;
        stageObj["max_time_ms"] = metrics.maxTimeMs;
        
        QJsonObject specificObj;
        for (auto mit = metrics.stageSpecificMetrics.begin(); mit != metrics.stageSpecificMetrics.end(); ++mit) {
            specificObj[mit.key()] = QJsonValue::fromVariant(mit.value());
        }
        stageObj["specific_metrics"] = specificObj;
        
        metricsObj[it.key()] = stageObj;
    }
    root["stage_metrics"] = metricsObj;
    
    // Export threshold decisions
    QJsonObject thresholdsObj;
    for (auto it = thresholdDecisions.begin(); it != thresholdDecisions.end(); ++it) {
        QJsonArray decisionsArray;
        for (const QVariantMap& decision : it.value()) {
            QJsonObject decisionObj;
            for (auto dit = decision.begin(); dit != decision.end(); ++dit) {
                decisionObj[dit.key()] = QJsonValue::fromVariant(dit.value());
            }
            decisionsArray.append(decisionObj);
        }
        thresholdsObj[it.key()] = decisionsArray;
    }
    root["threshold_decisions"] = thresholdsObj;
    
    // Export cache statistics
    QJsonObject cacheObj;
    for (auto it = cacheStats.begin(); it != cacheStats.end(); ++it) {
        QJsonObject statsObj;
        statsObj["hits"] = it.value().first;
        statsObj["misses"] = it.value().second;
        double hitRate = (it.value().first + it.value().second > 0) ?
            static_cast<double>(it.value().first) / (it.value().first + it.value().second) : 0.0;
        statsObj["hit_rate"] = hitRate;
        cacheObj[it.key()] = statsObj;
    }
    root["cache_statistics"] = cacheObj;
    
    // Write to file
    QJsonDocument doc(root);
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    } else {
        qWarning() << "Failed to write instrumentation data to" << filePath;
    }
}

double PipelineInstrumentation::getTotalExecutionTime() const
{
    QMutexLocker locker(&mutex);
    return totalTimer.elapsed();
}

void PipelineInstrumentation::updateStageMetrics(const QString& stageName, const PipelineEvent& event)
{
    if (!stageMetricsMap.contains(stageName)) {
        stageMetricsMap[stageName] = StageMetrics();
        stageMetricsMap[stageName].stageName = stageName;
    }
    
    StageMetrics& metrics = stageMetricsMap[stageName];
    
    // Count inputs/outputs
    if (!event.inputs.isEmpty()) {
        metrics.inputCount++;
    }
    if (!event.outputs.isEmpty()) {
        metrics.outputCount++;
    }
    
    // Update timing
    if (event.executionTimeMs > 0) {
        metrics.totalTimeMs += event.executionTimeMs;
        metrics.avgTimeMs = metrics.totalTimeMs / (metrics.inputCount > 0 ? metrics.inputCount : 1);
        metrics.minTimeMs = std::min(metrics.minTimeMs, event.executionTimeMs);
        metrics.maxTimeMs = std::max(metrics.maxTimeMs, event.executionTimeMs);
    }
    
    // Merge stage-specific metrics from metadata
    for (auto it = event.metadata.begin(); it != event.metadata.end(); ++it) {
        if (it.key().startsWith("stage_")) {
            metrics.stageSpecificMetrics[it.key()] = it.value();
        }
    }
}

qint64 PipelineInstrumentation::getCurrentTimestamp() const
{
    return QDateTime::currentMSecsSinceEpoch() * 1000;  // Convert to microseconds
}

} // namespace ocr_orc

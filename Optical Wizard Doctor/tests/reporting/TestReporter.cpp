#include "TestReporter.h"
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QDebug>
#include <QtCore/QDateTime>
#include <QtCore/QStringConverter>
#include <algorithm>
#include <cmath>

namespace ocr_orc {

TestReporter::TestReporter()
{
}

bool TestReporter::saveResultsToJson(const QMap<QString, EvaluationMetrics>& results,
                                    const QString& outputPath) const
{
    QJsonObject root;
    root["test_suite_version"] = "1.0";
    root["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    root["total_forms"] = results.size();
    
    // Calculate aggregate metrics
    double totalPrecision = 0.0;
    double totalRecall = 0.0;
    double totalF1 = 0.0;
    double totalIoU = 0.0;
    int count = 0;
    
    for (auto it = results.begin(); it != results.end(); ++it) {
        const EvaluationMetrics& metrics = it.value();
        totalPrecision += metrics.precision;
        totalRecall += metrics.recall;
        totalF1 += metrics.f1Score;
        totalIoU += metrics.meanIoU;
        count++;
    }
    
    if (count > 0) {
        QJsonObject aggregate;
        aggregate["average_precision"] = totalPrecision / count;
        aggregate["average_recall"] = totalRecall / count;
        aggregate["average_f1"] = totalF1 / count;
        aggregate["average_iou"] = totalIoU / count;
        root["aggregate_metrics"] = aggregate;
    }
    
    // Serialize individual test results
    QJsonObject testResults;
    for (auto it = results.begin(); it != results.end(); ++it) {
        const QString& formId = it.key();
        const EvaluationMetrics& metrics = it.value();
        
        QJsonObject formResult;
        formResult["form_id"] = formId;
        formResult["precision"] = metrics.precision;
        formResult["recall"] = metrics.recall;
        formResult["f1_score"] = metrics.f1Score;
        formResult["mean_iou"] = metrics.meanIoU;
        
        // Performance metrics
        QJsonObject perf;
        perf["total_time_ms"] = metrics.performance.totalTimeMs;
        perf["memory_usage_mb"] = metrics.performance.memoryUsageMB;
        
        QJsonObject stageTimings;
        for (auto stIt = metrics.performance.stageTimings.begin(); 
             stIt != metrics.performance.stageTimings.end(); ++stIt) {
            stageTimings[stIt.key()] = stIt.value();
        }
        perf["stage_timings"] = stageTimings;
        formResult["performance"] = perf;
        
        // Per-region evaluations
        QJsonArray regionEvals;
        for (const RegionEvaluation& eval : metrics.regionEvaluations) {
            QJsonObject regionObj;
            regionObj["region_name"] = eval.regionName;
            regionObj["is_true_positive"] = eval.isTruePositive;
            regionObj["is_false_positive"] = eval.isFalsePositive;
            regionObj["is_false_negative"] = eval.isFalseNegative;
            regionObj["iou"] = eval.iou;
            regionObj["position_accuracy"] = eval.positionAccuracy;
            regionObj["size_accuracy"] = eval.sizeAccuracy;
            regionObj["confidence"] = eval.confidence;
            regionEvals.append(regionObj);
        }
        formResult["region_evaluations"] = regionEvals;
        
        testResults[formId] = formResult;
    }
    root["test_results"] = testResults;
    
    // Write to file
    QJsonDocument doc(root);
    QFile file(outputPath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
        return true;
    }
    
    qWarning() << "Failed to write test results to" << outputPath;
    return false;
}

bool TestReporter::saveResultsToCsv(const QMap<QString, EvaluationMetrics>& results,
                                   const QString& outputPath) const
{
    QFile file(outputPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to open CSV file for writing:" << outputPath;
        return false;
    }
    
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    
    // Write header
    out << "form_id,precision,recall,f1_score,mean_iou,total_time_ms,"
        << "true_positives,false_positives,false_negatives\n";
    
    // Write data rows
    for (auto it = results.begin(); it != results.end(); ++it) {
        const QString& formId = it.key();
        const EvaluationMetrics& metrics = it.value();
        
        // Count TP/FP/FN
        int tp = 0, fp = 0, fn = 0;
        for (const RegionEvaluation& eval : metrics.regionEvaluations) {
            if (eval.isTruePositive) tp++;
            else if (eval.isFalsePositive) fp++;
            else if (eval.isFalseNegative) fn++;
        }
        
        out << formId << ","
            << metrics.precision << ","
            << metrics.recall << ","
            << metrics.f1Score << ","
            << metrics.meanIoU << ","
            << metrics.performance.totalTimeMs << ","
            << tp << ","
            << fp << ","
            << fn << "\n";
    }
    
    file.close();
    return true;
}

bool TestReporter::saveSingleResultToJson(const QString& formId, const EvaluationMetrics& metrics,
                                          const QString& outputPath) const
{
    QMap<QString, EvaluationMetrics> singleResult;
    singleResult[formId] = metrics;
    return saveResultsToJson(singleResult, outputPath);
}

void TestReporter::printDetailedResults(const QString& formId, const EvaluationMetrics& metrics,
                                       const GroundTruthAnnotation& groundTruth) const
{
    qDebug() << "\n" << QString("=").repeated(70);
    qDebug() << "Detailed Results for Form:" << formId;
    qDebug() << QString("=").repeated(70);
    
    // Overall metrics
    qDebug() << "\nOverall Metrics:";
    qDebug() << "  Precision:" << QString::number(metrics.precision, 'f', 4);
    qDebug() << "  Recall:" << QString::number(metrics.recall, 'f', 4);
    qDebug() << "  F1 Score:" << QString::number(metrics.f1Score, 'f', 4);
    qDebug() << "  Mean IoU:" << QString::number(metrics.meanIoU, 'f', 4);
    
    // Performance
    qDebug() << "\nPerformance:";
    qDebug() << "  Total Time:" << QString::number(metrics.performance.totalTimeMs, 'f', 2) << "ms";
    if (!metrics.performance.stageTimings.isEmpty()) {
        qDebug() << "  Stage Timings:";
        for (auto it = metrics.performance.stageTimings.begin(); 
             it != metrics.performance.stageTimings.end(); ++it) {
            qDebug() << "    " << it.key() << ":" << QString::number(it.value(), 'f', 2) << "ms";
        }
    }
    
    // Per-region details
    qDebug() << "\nPer-Region Analysis:";
    
    // Group by status
    QList<RegionEvaluation> truePositives;
    QList<RegionEvaluation> falsePositives;
    QList<RegionEvaluation> falseNegatives;
    
    for (const RegionEvaluation& eval : metrics.regionEvaluations) {
        if (eval.isTruePositive) {
            truePositives.append(eval);
        } else if (eval.isFalsePositive) {
            falsePositives.append(eval);
        } else if (eval.isFalseNegative) {
            falseNegatives.append(eval);
        }
    }
    
    qDebug() << "\n  True Positives (" << truePositives.size() << "):";
    for (const RegionEvaluation& eval : truePositives) {
        QString name = eval.regionName.isEmpty() ? "Unknown" : eval.regionName;
        qDebug() << "    ✓" << name
                 << "- IoU:" << QString::number(eval.iou, 'f', 3)
                 << "Confidence:" << QString::number(eval.confidence, 'f', 3)
                 << "Pos Acc:" << QString::number(eval.positionAccuracy, 'f', 3)
                 << "Size Acc:" << QString::number(eval.sizeAccuracy, 'f', 3);
    }
    
    if (!falsePositives.isEmpty()) {
        qDebug() << "\n  False Positives (" << falsePositives.size() << "):";
        for (const RegionEvaluation& eval : falsePositives) {
            qDebug() << "    ✗ Extra region detected"
                     << "- IoU:" << QString::number(eval.iou, 'f', 3)
                     << "Confidence:" << QString::number(eval.confidence, 'f', 3);
        }
    }
    
    if (!falseNegatives.isEmpty()) {
        qDebug() << "\n  False Negatives (" << falseNegatives.size() << "):";
        for (const RegionEvaluation& eval : falseNegatives) {
            QString name = eval.regionName.isEmpty() ? "Unknown" : eval.regionName;
            qDebug() << "    ✗ Missing:" << name;
        }
    }
    
    // Summary counts
    qDebug() << "\nSummary:";
    qDebug() << "  Expected Regions:" << groundTruth.expectedRegions.size();
    qDebug() << "  Detected Regions:" << (truePositives.size() + falsePositives.size());
    qDebug() << "  Correctly Detected:" << truePositives.size();
    qDebug() << "  Missed:" << falseNegatives.size();
    qDebug() << "  Extra Detections:" << falsePositives.size();
    
    qDebug() << QString("=").repeated(70) << "\n";
}

void TestReporter::printSummary(const QMap<QString, EvaluationMetrics>& results) const
{
    qDebug() << "\n" << QString("=").repeated(70);
    qDebug() << "Test Suite Summary";
    qDebug() << QString("=").repeated(70);
    
    if (results.isEmpty()) {
        qDebug() << "No test results available.";
        return;
    }
    
    // Calculate aggregates
    double totalPrecision = 0.0;
    double totalRecall = 0.0;
    double totalF1 = 0.0;
    double totalIoU = 0.0;
    double totalTime = 0.0;
    int totalTP = 0, totalFP = 0, totalFN = 0;
    
    for (auto it = results.begin(); it != results.end(); ++it) {
        const EvaluationMetrics& metrics = it.value();
        totalPrecision += metrics.precision;
        totalRecall += metrics.recall;
        totalF1 += metrics.f1Score;
        totalIoU += metrics.meanIoU;
        totalTime += metrics.performance.totalTimeMs;
        
        for (const RegionEvaluation& eval : metrics.regionEvaluations) {
            if (eval.isTruePositive) totalTP++;
            else if (eval.isFalsePositive) totalFP++;
            else if (eval.isFalseNegative) totalFN++;
        }
    }
    
    int count = results.size();
    
    qDebug() << "\nAggregate Metrics (across" << count << "forms):";
    qDebug() << "  Average Precision:" << QString::number(totalPrecision / count, 'f', 4);
    qDebug() << "  Average Recall:" << QString::number(totalRecall / count, 'f', 4);
    qDebug() << "  Average F1 Score:" << QString::number(totalF1 / count, 'f', 4);
    qDebug() << "  Average Mean IoU:" << QString::number(totalIoU / count, 'f', 4);
    qDebug() << "  Total Execution Time:" << QString::number(totalTime, 'f', 2) << "ms";
    qDebug() << "  Average Time per Form:" << QString::number(totalTime / count, 'f', 2) << "ms";
    
    qDebug() << "\nDetection Statistics:";
    qDebug() << "  Total True Positives:" << totalTP;
    qDebug() << "  Total False Positives:" << totalFP;
    qDebug() << "  Total False Negatives:" << totalFN;
    
    qDebug() << "\nPer-Form Results:";
    for (auto it = results.begin(); it != results.end(); ++it) {
        const QString& formId = it.key();
        const EvaluationMetrics& metrics = it.value();
        
        int tp = 0, fp = 0, fn = 0;
        for (const RegionEvaluation& eval : metrics.regionEvaluations) {
            if (eval.isTruePositive) tp++;
            else if (eval.isFalsePositive) fp++;
            else if (eval.isFalseNegative) fn++;
        }
        
        QString status = (metrics.precision >= 0.7 && metrics.recall >= 0.7 && metrics.f1Score >= 0.7) 
                        ? "✓ PASS" : "✗ FAIL";
        
        qDebug() << "  " << status << formId
                 << "- P:" << QString::number(metrics.precision, 'f', 3)
                 << "R:" << QString::number(metrics.recall, 'f', 3)
                 << "F1:" << QString::number(metrics.f1Score, 'f', 3)
                 << "IoU:" << QString::number(metrics.meanIoU, 'f', 3)
                 << "TP:" << tp << "FP:" << fp << "FN:" << fn;
    }
    
    qDebug() << QString("=").repeated(70) << "\n";
}

bool TestReporter::calculatePassStatus(const QMap<QString, EvaluationMetrics>& results,
                                      double minPrecision, double minRecall, double minF1) const
{
    if (results.isEmpty()) {
        return false;
    }
    
    for (auto it = results.begin(); it != results.end(); ++it) {
        const EvaluationMetrics& metrics = it.value();
        if (metrics.precision < minPrecision || 
            metrics.recall < minRecall || 
            metrics.f1Score < minF1) {
            return false;
        }
    }
    
    return true;
}

QString TestReporter::formatRegionEvaluation(const RegionEvaluation& eval, const QString& regionName) const
{
    QString status;
    if (eval.isTruePositive) status = "TP";
    else if (eval.isFalsePositive) status = "FP";
    else if (eval.isFalseNegative) status = "FN";
    else status = "UNKNOWN";
    
    return QString("%1: %2 (IoU: %3, Conf: %4)")
           .arg(status)
           .arg(regionName)
           .arg(eval.iou, 0, 'f', 3)
           .arg(eval.confidence, 0, 'f', 3);
}

QString TestReporter::formatStageMetrics(const QMap<QString, StageMetrics>& stageMetrics) const
{
    QString result;
    QTextStream stream(&result);
    for (auto it = stageMetrics.begin(); it != stageMetrics.end(); ++it) {
        stream << "  " << it.key() << ": " << it.value().totalTimeMs << "ms\n";
    }
    return result;
}

QString TestReporter::formatPerformanceMetrics(const PerformanceMetrics& perf) const
{
    return QString("Total: %1ms").arg(perf.totalTimeMs, 0, 'f', 2);
}

bool TestReporter::generateHtmlReport(const QMap<QString, EvaluationMetrics>& results,
                                      const QString& outputPath,
                                      double minPrecision, double minRecall, double minF1) const
{
    if (results.isEmpty()) {
        qWarning() << "Cannot generate HTML report: No test results";
        return false;
    }
    
    // Calculate aggregate metrics
    double totalPrecision = 0.0;
    double totalRecall = 0.0;
    double totalF1 = 0.0;
    double totalIoU = 0.0;
    double totalTime = 0.0;
    int totalTP = 0, totalFP = 0, totalFN = 0;
    int passCount = 0;
    int failCount = 0;
    
    QList<QString> formIds;
    QList<double> precisionValues;
    QList<double> recallValues;
    QList<double> f1Values;
    QList<double> iouValues;
    QList<bool> passStatus;
    
    for (auto it = results.begin(); it != results.end(); ++it) {
        const QString& formId = it.key();
        const EvaluationMetrics& metrics = it.value();
        
        formIds.append(formId);
        precisionValues.append(metrics.precision);
        recallValues.append(metrics.recall);
        f1Values.append(metrics.f1Score);
        iouValues.append(metrics.meanIoU);
        
        totalPrecision += metrics.precision;
        totalRecall += metrics.recall;
        totalF1 += metrics.f1Score;
        totalIoU += metrics.meanIoU;
        totalTime += metrics.performance.totalTimeMs;
        
        bool passed = (metrics.precision >= minPrecision && 
                       metrics.recall >= minRecall && 
                       metrics.f1Score >= minF1);
        passStatus.append(passed);
        if (passed) passCount++;
        else failCount++;
        
        for (const RegionEvaluation& eval : metrics.regionEvaluations) {
            if (eval.isTruePositive) totalTP++;
            else if (eval.isFalsePositive) totalFP++;
            else if (eval.isFalseNegative) totalFN++;
        }
    }
    
    int count = results.size();
    double avgPrecision = totalPrecision / count;
    double avgRecall = totalRecall / count;
    double avgF1 = totalF1 / count;
    double avgIoU = totalIoU / count;
    
    // Generate HTML
    QFile file(outputPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to open HTML file for writing:" << outputPath;
        return false;
    }
    
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    
    // HTML header with embedded CSS and Chart.js
    out << "<!DOCTYPE html>\n"
        << "<html lang=\"en\">\n"
        << "<head>\n"
        << "  <meta charset=\"UTF-8\">\n"
        << "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
        << "  <title>Magic Detection Test Suite Report</title>\n"
        << "  <script src=\"https://cdn.jsdelivr.net/npm/chart.js@4.4.0/dist/chart.umd.min.js\"></script>\n"
        << "  <style>\n"
        << "    * { margin: 0; padding: 0; box-sizing: border-box; }\n"
        << "    body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif; "
        << "background: #f5f5f5; color: #333; line-height: 1.6; padding: 20px; }\n"
        << "    .container { max-width: 1400px; margin: 0 auto; background: white; "
        << "border-radius: 8px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); padding: 30px; }\n"
        << "    h1 { color: #2c3e50; border-bottom: 3px solid #3498db; padding-bottom: 10px; "
        << "margin-bottom: 30px; }\n"
        << "    h2 { color: #34495e; margin-top: 30px; margin-bottom: 15px; "
        << "border-left: 4px solid #3498db; padding-left: 10px; }\n"
        << "    .summary-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); "
        << "gap: 20px; margin: 20px 0; }\n"
        << "    .metric-card { background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); "
        << "color: white; padding: 20px; border-radius: 8px; text-align: center; }\n"
        << "    .metric-card.pass { background: linear-gradient(135deg, #11998e 0%, #38ef7d 100%); }\n"
        << "    .metric-card.fail { background: linear-gradient(135deg, #eb3349 0%, #f45c43 100%); }\n"
        << "    .metric-value { font-size: 2.5em; font-weight: bold; margin: 10px 0; }\n"
        << "    .metric-label { font-size: 0.9em; opacity: 0.9; text-transform: uppercase; }\n"
        << "    .chart-container { margin: 30px 0; background: #fafafa; padding: 20px; "
        << "border-radius: 8px; }\n"
        << "    .chart-wrapper { position: relative; height: 400px; margin: 20px 0; }\n"
        << "    table { width: 100%; border-collapse: collapse; margin: 20px 0; "
        << "background: white; box-shadow: 0 1px 3px rgba(0,0,0,0.1); }\n"
        << "    th { background: #34495e; color: white; padding: 12px; text-align: left; "
        << "font-weight: 600; }\n"
        << "    td { padding: 12px; border-bottom: 1px solid #eee; }\n"
        << "    tr:hover { background: #f8f9fa; }\n"
        << "    .status-pass { color: #27ae60; font-weight: bold; }\n"
        << "    .status-fail { color: #e74c3c; font-weight: bold; }\n"
        << "    .timestamp { color: #7f8c8d; font-size: 0.9em; margin-bottom: 20px; }\n"
        << "    .footer { margin-top: 40px; padding-top: 20px; border-top: 2px solid #eee; "
        << "text-align: center; color: #7f8c8d; font-size: 0.9em; }\n"
        << "  </style>\n"
        << "</head>\n"
        << "<body>\n"
        << "  <div class=\"container\">\n"
        << "    <h1>🎯 Magic Detection Test Suite Report</h1>\n"
        << "    <div class=\"timestamp\">Generated: " 
        << QDateTime::currentDateTime().toString(Qt::ISODate) << "</div>\n";
    
    // Summary cards
    out << "    <div class=\"summary-grid\">\n"
        << "      <div class=\"metric-card\">\n"
        << "        <div class=\"metric-label\">Total Forms</div>\n"
        << "        <div class=\"metric-value\">" << count << "</div>\n"
        << "      </div>\n"
        << "      <div class=\"metric-card " << (passCount > failCount ? "pass" : "fail") << "\">\n"
        << "        <div class=\"metric-label\">Passed</div>\n"
        << "        <div class=\"metric-value\">" << passCount << "</div>\n"
        << "      </div>\n"
        << "      <div class=\"metric-card " << (failCount > 0 ? "fail" : "pass") << "\">\n"
        << "        <div class=\"metric-label\">Failed</div>\n"
        << "        <div class=\"metric-value\">" << failCount << "</div>\n"
        << "      </div>\n"
        << "      <div class=\"metric-card\">\n"
        << "        <div class=\"metric-label\">Avg Precision</div>\n"
        << "        <div class=\"metric-value\">" << QString::number(avgPrecision, 'f', 3) << "</div>\n"
        << "      </div>\n"
        << "      <div class=\"metric-card\">\n"
        << "        <div class=\"metric-label\">Avg Recall</div>\n"
        << "        <div class=\"metric-value\">" << QString::number(avgRecall, 'f', 3) << "</div>\n"
        << "      </div>\n"
        << "      <div class=\"metric-card\">\n"
        << "        <div class=\"metric-label\">Avg F1 Score</div>\n"
        << "        <div class=\"metric-value\">" << QString::number(avgF1, 'f', 3) << "</div>\n"
        << "      </div>\n"
        << "      <div class=\"metric-card\">\n"
        << "        <div class=\"metric-label\">Avg IoU</div>\n"
        << "        <div class=\"metric-value\">" << QString::number(avgIoU, 'f', 3) << "</div>\n"
        << "      </div>\n"
        << "      <div class=\"metric-card\">\n"
        << "        <div class=\"metric-label\">Total Time</div>\n"
        << "        <div class=\"metric-value\">" << QString::number(totalTime / 1000.0, 'f', 2) << "s</div>\n"
        << "      </div>\n"
        << "    </div>\n";
    
    // Charts section
    out << "    <h2>📊 Performance Charts</h2>\n"
        << "    <div class=\"chart-container\">\n"
        << "      <div class=\"chart-wrapper\">\n"
        << "        <canvas id=\"metricsChart\"></canvas>\n"
        << "      </div>\n"
        << "      <div class=\"chart-wrapper\">\n"
        << "        <canvas id=\"detectionChart\"></canvas>\n"
        << "      </div>\n"
        << "    </div>\n";
    
    // Results table
    out << "    <h2>📋 Detailed Results</h2>\n"
        << "    <table>\n"
        << "      <thead>\n"
        << "        <tr>\n"
        << "          <th>Form ID</th>\n"
        << "          <th>Precision</th>\n"
        << "          <th>Recall</th>\n"
        << "          <th>F1 Score</th>\n"
        << "          <th>Mean IoU</th>\n"
        << "          <th>TP</th>\n"
        << "          <th>FP</th>\n"
        << "          <th>FN</th>\n"
        << "          <th>Time (ms)</th>\n"
        << "          <th>Status</th>\n"
        << "        </tr>\n"
        << "      </thead>\n"
        << "      <tbody>\n";
    
    int idx = 0;
    for (auto it = results.begin(); it != results.end(); ++it, ++idx) {
        const QString& formId = it.key();
        const EvaluationMetrics& metrics = it.value();
        
        int tp = 0, fp = 0, fn = 0;
        for (const RegionEvaluation& eval : metrics.regionEvaluations) {
            if (eval.isTruePositive) tp++;
            else if (eval.isFalsePositive) fp++;
            else if (eval.isFalseNegative) fn++;
        }
        
        bool passed = passStatus[idx];
        QString statusClass = passed ? "status-pass" : "status-fail";
        QString statusText = passed ? "✓ PASS" : "✗ FAIL";
        
        out << "        <tr>\n"
            << "          <td><strong>" << formId << "</strong></td>\n"
            << "          <td>" << QString::number(metrics.precision, 'f', 4) << "</td>\n"
            << "          <td>" << QString::number(metrics.recall, 'f', 4) << "</td>\n"
            << "          <td>" << QString::number(metrics.f1Score, 'f', 4) << "</td>\n"
            << "          <td>" << QString::number(metrics.meanIoU, 'f', 4) << "</td>\n"
            << "          <td>" << tp << "</td>\n"
            << "          <td>" << fp << "</td>\n"
            << "          <td>" << fn << "</td>\n"
            << "          <td>" << QString::number(metrics.performance.totalTimeMs, 'f', 2) << "</td>\n"
            << "          <td class=\"" << statusClass << "\">" << statusText << "</td>\n"
            << "        </tr>\n";
    }
    
    out << "      </tbody>\n"
        << "    </table>\n";
    
    // JavaScript for charts
    out << "    <script>\n"
        << "      // Metrics comparison chart\n"
        << "      const metricsCtx = document.getElementById('metricsChart').getContext('2d');\n"
        << "      new Chart(metricsCtx, {\n"
        << "        type: 'bar',\n"
        << "        data: {\n"
        << "          labels: " << generateJsArray(formIds) << ",\n"
        << "          datasets: [\n"
        << "            { label: 'Precision', data: " << generateJsArray(precisionValues) 
        << ", backgroundColor: 'rgba(54, 162, 235, 0.6)' },\n"
        << "            { label: 'Recall', data: " << generateJsArray(recallValues) 
        << ", backgroundColor: 'rgba(75, 192, 192, 0.6)' },\n"
        << "            { label: 'F1 Score', data: " << generateJsArray(f1Values) 
        << ", backgroundColor: 'rgba(153, 102, 255, 0.6)' },\n"
        << "            { label: 'Mean IoU', data: " << generateJsArray(iouValues) 
        << ", backgroundColor: 'rgba(255, 159, 64, 0.6)' }\n"
        << "          ]\n"
        << "        },\n"
        << "        options: {\n"
        << "          responsive: true,\n"
        << "          maintainAspectRatio: false,\n"
        << "          scales: { y: { beginAtZero: true, max: 1.0 } },\n"
        << "          plugins: { legend: { position: 'top' } }\n"
        << "        }\n"
        << "      });\n"
        << "      \n"
        << "      // Detection statistics chart\n"
        << "      const detectionCtx = document.getElementById('detectionChart').getContext('2d');\n"
        << "      new Chart(detectionCtx, {\n"
        << "        type: 'doughnut',\n"
        << "        data: {\n"
        << "          labels: ['True Positives', 'False Positives', 'False Negatives'],\n"
        << "          datasets: [{\n"
        << "            data: [" << totalTP << ", " << totalFP << ", " << totalFN << "],\n"
        << "            backgroundColor: ['#27ae60', '#e74c3c', '#f39c12']\n"
        << "          }]\n"
        << "        },\n"
        << "        options: {\n"
        << "          responsive: true,\n"
        << "          maintainAspectRatio: false,\n"
        << "          plugins: { legend: { position: 'right' } }\n"
        << "        }\n"
        << "      });\n"
        << "    </script>\n"
        << "    <div class=\"footer\">\n"
        << "      Magic Detection Test Suite v1.0 | "
        << "Thresholds: Precision &ge; " << minPrecision 
        << ", Recall &ge; " << minRecall 
        << ", F1 &ge; " << minF1 << "\n"
        << "    </div>\n"
        << "  </div>\n"
        << "</body>\n"
        << "</html>\n";
    
    file.close();
    return true;
}

QString TestReporter::generateJsArray(const QList<QString>& values) const
{
    QString result = "[";
    for (int i = 0; i < values.size(); ++i) {
        if (i > 0) result += ", ";
        QString escaped = values[i];
        escaped.replace("\\", "\\\\").replace("\"", "\\\"").replace("\n", "\\n");
        result += "\"" + escaped + "\"";
    }
    result += "]";
    return result;
}

QString TestReporter::generateJsArray(const QList<double>& values) const
{
    QString result = "[";
    for (int i = 0; i < values.size(); ++i) {
        if (i > 0) result += ", ";
        result += QString::number(values[i], 'f', 4);
    }
    result += "]";
    return result;
}

} // namespace ocr_orc

#include "MagicDetectionTestRunner.h"
#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QCommandLineParser>
#include <QtCore/QDir>
#include <QtCore/QStandardPaths>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QDateTime>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

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

int main(int argc, char *argv[])
{
    // Force immediate output
    fprintf(stderr, "=== Magic Detection Test Suite Starting ===\n");
    fflush(stderr);
    fprintf(stdout, "=== Magic Detection Test Suite Starting ===\n");
    fflush(stdout);
    
    // #region agent log
    debugLog("test_magic_detection_suite.cpp:main", "Entry", {{"argc", argc}});
    // #endregion
    
    fprintf(stderr, "Initializing QCoreApplication...\n");
    fflush(stderr);
    QCoreApplication app(argc, argv);
    
    fprintf(stderr, "QCoreApplication initialized\n");
    fflush(stderr);
    
    // #region agent log
    debugLog("test_magic_detection_suite.cpp:main", "After QApplication creation");
    // #endregion
    
    QCommandLineParser parser;
    parser.setApplicationDescription("Magic Detection Test Suite");
    parser.addHelpOption();
    parser.addPositionalArgument("form", "Form ID to test (optional, tests all if not specified)");
    
    QCommandLineOption baseDirOption("base-dir", "Base directory for test data", "directory", "tests/data");
    parser.addOption(baseDirOption);
    
    QCommandLineOption noInstrumentationOption("no-instrumentation", "Disable instrumentation");
    parser.addOption(noInstrumentationOption);
    
    QCommandLineOption outputDirOption("output-dir", "Directory for output files", "directory", "test_results");
    parser.addOption(outputDirOption);
    
    QCommandLineOption noReportOption("no-report", "Don't generate report files");
    parser.addOption(noReportOption);
    
    QCommandLineOption minPrecisionOption("min-precision", "Minimum precision threshold", "value", "0.7");
    parser.addOption(minPrecisionOption);
    
    QCommandLineOption minRecallOption("min-recall", "Minimum recall threshold", "value", "0.7");
    parser.addOption(minRecallOption);
    
    QCommandLineOption minF1Option("min-f1", "Minimum F1 threshold", "value", "0.7");
    parser.addOption(minF1Option);
    
    QCommandLineOption skipOcrOption("skip-ocr", "Skip OCR and use hybrid detection method (faster, avoids Tesseract hangs)");
    parser.addOption(skipOcrOption);
    
    fprintf(stderr, "Processing command line arguments...\n");
    fflush(stderr);
    parser.process(app);
    
    fprintf(stderr, "Command line arguments processed\n");
    fflush(stderr);
    
    // #region agent log
    debugLog("test_magic_detection_suite.cpp:main", "After parser.process");
    // #endregion
    
    QString baseDir = parser.value(baseDirOption);
    QString outputDir = parser.value(outputDirOption);
    bool instrumentationEnabled = !parser.isSet(noInstrumentationOption);
    bool generateReports = !parser.isSet(noReportOption);
    bool skipOcr = parser.isSet(skipOcrOption);
    
    fprintf(stderr, "skipOcr flag: %s\n", skipOcr ? "TRUE" : "FALSE");
    fflush(stderr);
    
    // #region agent log
    debugLog("test_magic_detection_suite.cpp:main", "Parsed skipOcr", {{"skipOcr", skipOcr}, {"isSet", parser.isSet(skipOcrOption)}});
    // #endregion
    
    double minPrecision = parser.value(minPrecisionOption).toDouble();
    double minRecall = parser.value(minRecallOption).toDouble();
    double minF1 = parser.value(minF1Option).toDouble();
    
    // Resolve relative paths relative to executable location
    QDir exeDir = QDir(QCoreApplication::applicationDirPath());
    if (baseDir.startsWith("tests/")) {
        // If relative path, resolve from executable location
        // Executable is in build/tests/, so go up two levels
        exeDir.cdUp(); // build/
        exeDir.cdUp(); // project root
        baseDir = exeDir.absoluteFilePath(baseDir);
    }
    if (outputDir.startsWith("test_results") && !QDir(outputDir).isAbsolute()) {
        exeDir.cdUp(); // build/
        exeDir.cdUp(); // project root
        outputDir = exeDir.absoluteFilePath(outputDir);
    }
    
    // #region agent log
    debugLog("test_magic_detection_suite.cpp:main", "Parsed options", {
        {"baseDir", baseDir}, {"outputDir", outputDir}, 
        {"instrumentationEnabled", instrumentationEnabled}, {"generateReports", generateReports}
    });
    // #endregion
    
    // Create output directory if it doesn't exist
    if (generateReports) {
        QDir dir;
        if (!dir.exists(outputDir)) {
            dir.mkpath(outputDir);
        }
    }
    
    fprintf(stderr, "Creating MagicDetectionTestRunner...\n");
    fflush(stderr);
    
    // #region agent log
    debugLog("test_magic_detection_suite.cpp:main", "Before runner creation");
    // #endregion
    
    ocr_orc::MagicDetectionTestRunner runner;
    
    fprintf(stderr, "MagicDetectionTestRunner created\n");
    fflush(stderr);
    
    // #region agent log
    debugLog("test_magic_detection_suite.cpp:main", "After runner creation");
    // #endregion
    // #region agent log
    debugLog("test_magic_detection_suite.cpp:main", "Before setBaseDirectory");
    // #endregion
    
    runner.setBaseDirectory(baseDir);
    
    // #region agent log
    debugLog("test_magic_detection_suite.cpp:main", "After setBaseDirectory");
    // #endregion
    
    runner.setInstrumentationEnabled(instrumentationEnabled);
    
    // #region agent log
    debugLog("test_magic_detection_suite.cpp:main", "Before setDetectionMethod", {{"skipOcr", skipOcr}});
    // #endregion
    
    if (skipOcr) {
        qDebug() << "Setting detection method to 'hybrid' (skipping OCR)...";
        fflush(stdout);
        runner.setDetectionMethod("hybrid");
        
        // #region agent log
        debugLog("test_magic_detection_suite.cpp:main", "setDetectionMethod called", {{"method", "hybrid"}});
        // #endregion
    } else {
        qDebug() << "Using default 'ocr-first' detection method...";
        fflush(stdout);
        
        // #region agent log
        debugLog("test_magic_detection_suite.cpp:main", "Using default method", {{"method", "ocr-first"}});
        // #endregion
    }
    
    ocr_orc::TestReporter* reporter = runner.getReporter();
    
    // #region agent log
    debugLog("test_magic_detection_suite.cpp:main", "Got reporter", {{"reporterNull", reporter == nullptr}});
    // #endregion
    
    QStringList args = parser.positionalArguments();
    int exitCode = 0;
    
    if (args.isEmpty()) {
        // Run all tests
        qDebug() << "========================================";
        qDebug() << "Magic Detection Test Suite";
        qDebug() << "========================================";
        qDebug() << "Running full test suite...";
        qDebug() << "Base directory:" << baseDir;
        qDebug() << "Output directory:" << outputDir;
        qDebug() << "";
        fflush(stdout);  // Force immediate output
        fflush(stdout);
        
        // Verify base directory exists
        QDir baseDirCheck(baseDir);
        if (!baseDirCheck.exists()) {
            qCritical() << "ERROR: Base directory does not exist:" << baseDir;
            return 1;
        }
        qDebug() << "✓ Base directory found";
        fflush(stdout);
        
        // #region agent log
        debugLog("test_magic_detection_suite.cpp:main", "Before runTestSuite");
        // #endregion
        
        QMap<QString, ocr_orc::EvaluationMetrics> results = runner.runTestSuite();
        
        // #region agent log
        debugLog("test_magic_detection_suite.cpp:main", "After runTestSuite", {{"resultCount", results.size()}});
        // #endregion
        
        // Print detailed summary
        reporter->printSummary(results);
        
        // Save results to files
        if (generateReports) {
            QString jsonPath = QDir(outputDir).filePath("test_results.json");
            QString csvPath = QDir(outputDir).filePath("test_results.csv");
            QString htmlPath = QDir(outputDir).filePath("test_results.html");
            
            if (reporter->saveResultsToJson(results, jsonPath)) {
                qDebug() << "Results saved to:" << jsonPath;
            }
            if (reporter->saveResultsToCsv(results, csvPath)) {
                qDebug() << "Results saved to:" << csvPath;
            }
            if (reporter->generateHtmlReport(results, htmlPath, minPrecision, minRecall, minF1)) {
                qDebug() << "HTML report generated:" << htmlPath;
            }
        }
        
        // Export instrumentation data
        if (instrumentationEnabled) {
            QString instPath = QDir(outputDir).filePath("test_results_instrumentation.json");
            runner.getInstrumentation()->exportToJson(instPath);
            qDebug() << "Instrumentation data exported to" << instPath;
        }
        
        // Check pass/fail status
        if (!reporter->calculatePassStatus(results, minPrecision, minRecall, minF1)) {
            qWarning() << "\n⚠️  Test suite FAILED: Some tests did not meet minimum thresholds";
            qWarning() << "Minimum thresholds: Precision >=" << minPrecision 
                       << "Recall >=" << minRecall << "F1 >=" << minF1;
            exitCode = 1;
        } else {
            qDebug() << "\n✓ Test suite PASSED: All tests met minimum thresholds";
        }
    } else {
        // Run single test
        QString formId = args[0];
        qDebug() << "Running test for form:" << formId;
        
        ocr_orc::EvaluationMetrics metrics = runner.runTest(formId);
        
        // Get ground truth for detailed output
        ocr_orc::GroundTruthAnnotation groundTruth = runner.getGroundTruth(formId);
        
        // Print detailed results
        reporter->printDetailedResults(formId, metrics, groundTruth);
        
        // Save single result
        if (generateReports) {
            QString jsonPath = QDir(outputDir).filePath(QString("test_result_%1.json").arg(formId));
            QString htmlPath = QDir(outputDir).filePath(QString("test_result_%1.html").arg(formId));
            
            if (reporter->saveSingleResultToJson(formId, metrics, jsonPath)) {
                qDebug() << "Result saved to:" << jsonPath;
            }
            
            // Generate HTML report for single test
            QMap<QString, ocr_orc::EvaluationMetrics> singleResult;
            singleResult[formId] = metrics;
            if (reporter->generateHtmlReport(singleResult, htmlPath, minPrecision, minRecall, minF1)) {
                qDebug() << "HTML report generated:" << htmlPath;
            }
        }
        
        // Export instrumentation data
        if (instrumentationEnabled) {
            QString instPath = QDir(outputDir).filePath(QString("test_result_%1_instrumentation.json").arg(formId));
            runner.getInstrumentation()->exportToJson(instPath);
            qDebug() << "Instrumentation data exported to" << instPath;
        }
        
        // Check pass/fail status
        QMap<QString, ocr_orc::EvaluationMetrics> singleResult;
        singleResult[formId] = metrics;
        if (!reporter->calculatePassStatus(singleResult, minPrecision, minRecall, minF1)) {
            qWarning() << "\n⚠️  Test FAILED: Did not meet minimum thresholds";
            exitCode = 1;
        } else {
            qDebug() << "\n✓ Test PASSED";
        }
    }
    
    return exitCode;
}

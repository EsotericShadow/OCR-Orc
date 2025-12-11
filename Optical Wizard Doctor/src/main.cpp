// OCR-Orc - Main Entry Point

#include <QtWidgets/QApplication>
#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QTextStream>
#include <QtCore/QDateTime>
#include "ui/MainWindow.h"

using namespace ocr_orc;

// Custom message handler to write debug output to file
void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QFile logFile("/tmp/ocr-orc-rotation-debug.log");
    if (logFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
        QTextStream stream(&logFile);
        QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
        QString typeStr;
        switch (type) {
            case QtDebugMsg: typeStr = "DEBUG"; break;
            case QtWarningMsg: typeStr = "WARNING"; break;
            case QtCriticalMsg: typeStr = "CRITICAL"; break;
            case QtFatalMsg: typeStr = "FATAL"; break;
            case QtInfoMsg: typeStr = "INFO"; break;
        }
        // Include context information (file, line, function) if available
        if (context.file && context.line > 0) {
            stream << "[" << timestamp << "] [" << typeStr << "] [" 
                   << QFileInfo(context.file).fileName() << ":" << context.line;
            if (context.function) {
                stream << " " << context.function;
            }
            stream << "] " << msg << Qt::endl;
        } else {
            stream << "[" << timestamp << "] [" << typeStr << "] " << msg << Qt::endl;
        }
        logFile.close();
    }
    
    // Also output to console
    QTextStream consoleStream(stdout);
    if (context.file && context.line > 0) {
        consoleStream << "[" << QFileInfo(context.file).fileName() << ":" << context.line << "] " << msg << Qt::endl;
    } else {
        consoleStream << msg << Qt::endl;
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Install custom message handler for logging
    qInstallMessageHandler(customMessageHandler);
    
    // Set application properties
    app.setApplicationName("OCR-Orc");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("OCR-Orc");
    
    // Create and show main window
    MainWindow window;
    window.show();
    
    return app.exec();
}


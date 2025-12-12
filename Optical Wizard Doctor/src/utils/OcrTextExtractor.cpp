#include "OcrTextExtractor.h"
#include "ImageConverter.h"
#include "../core/CoordinateSystem.h"
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QElapsedTimer>
#include <QtCore/QThread>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QDateTime>
#include <QtCore/QVariantMap>
#include <cstdio>
#include <algorithm>
#include <cmath>

namespace ocr_orc {

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

OcrTextExtractor::OcrTextExtractor()
    : minConfidence(60.0)  // Expert recommendation: 60% threshold (default was 50.0)
{
}

OcrTextExtractor::~OcrTextExtractor()
{
}

QList<OCRTextRegion> OcrTextExtractor::extractTextRegions(const QImage& image)
{
    fprintf(stderr, "[OcrTextExtractor::extractTextRegions] ========== OCR EXTRACTION START ==========\n");
    fflush(stderr);
    
    QList<OCRTextRegion> regions;
    tesseract::TessBaseAPI* api = nullptr; // Declare at function scope for cleanup in catch blocks
    
    try {
        fprintf(stderr, "[OcrTextExtractor::extractTextRegions] Step 1: Validating input image...\n");
        fflush(stderr);
        if (image.isNull()) {
            fprintf(stderr, "[OcrTextExtractor::extractTextRegions] ERROR: Image is null!\n");
            fflush(stderr);
            return regions;
        }
        
        fprintf(stderr, "[OcrTextExtractor::extractTextRegions] Step 1: ✓ Image valid - Size: %dx%d, Format: %d\n", 
                image.width(), image.height(), image.format());
        fflush(stderr);
        qDebug() << "[OcrTextExtractor::extractTextRegions] Image dimensions:" << image.width() << "x" << image.height();
        
        fprintf(stderr, "[OcrTextExtractor::extractTextRegions] Step 2: Preprocessing image...\n");
        fflush(stderr);
        // Preprocess image
        cv::Mat preprocessed;
        try {
            preprocessed = preprocessImage(image);
            fprintf(stderr, "[OcrTextExtractor::extractTextRegions] Step 2: ✓ Preprocessing complete\n");
            fflush(stderr);
        } catch (const std::exception& e) {
            fprintf(stderr, "[OcrTextExtractor::extractTextRegions] EXCEPTION in preprocessImage(): %s\n", e.what());
            fflush(stderr);
            qCritical() << "[OcrTextExtractor::extractTextRegions] Exception in preprocessing:" << e.what();
            return regions;
        } catch (...) {
            fprintf(stderr, "[OcrTextExtractor::extractTextRegions] UNKNOWN EXCEPTION in preprocessImage()\n");
            fflush(stderr);
            qCritical() << "[OcrTextExtractor::extractTextRegions] Unknown exception in preprocessing";
            return regions;
        }
        
        fprintf(stderr, "[OcrTextExtractor::extractTextRegions] Step 3: Validating preprocessed image...\n");
        fflush(stderr);
        // Validate preprocessed image
        if (preprocessed.empty() || preprocessed.data == nullptr) {
            fprintf(stderr, "[OcrTextExtractor::extractTextRegions] ERROR: Preprocessed image is empty or invalid!\n");
            fprintf(stderr, "[OcrTextExtractor::extractTextRegions] empty=%d, data=%p\n", 
                    preprocessed.empty(), (void*)preprocessed.data);
            fflush(stderr);
            qWarning() << "OcrTextExtractor: Preprocessed image is empty or invalid";
            return regions;
        }
        
        fprintf(stderr, "[OcrTextExtractor::extractTextRegions] Step 3: ✓ Preprocessed image valid - Size: %dx%d\n", 
                preprocessed.cols, preprocessed.rows);
        fflush(stderr);
    
    // CRITICAL: Clone to ensure data is continuous and stays valid
    // cv::Mat might have padding or the data pointer might become invalid
    if (!preprocessed.isContinuous()) {
        qDebug() << "OcrTextExtractor: Image not continuous, cloning...";
        fflush(stdout);
        preprocessed = preprocessed.clone();
    }
    
    qDebug() << "OcrTextExtractor: Preprocessed image:" << preprocessed.cols << "x" << preprocessed.rows 
             << "continuous:" << preprocessed.isContinuous();
    fflush(stdout);
    
        fprintf(stderr, "[OcrTextExtractor::extractTextRegions] Step 4: Creating TessBaseAPI...\n");
        fflush(stderr);
        qDebug() << "OcrTextExtractor: Creating TessBaseAPI...";
        fflush(stdout);
        
        // THREAD SAFETY: Log thread context for debugging (Tesseract API should be created in worker thread)
        fprintf(stderr, "[OcrTextExtractor::extractTextRegions] Thread context: %p\n", (void*)QThread::currentThread());
        fflush(stderr);
        qDebug() << "[OcrTextExtractor::extractTextRegions] Creating API in thread:" << QThread::currentThread();
        
        try {
            api = new tesseract::TessBaseAPI();
            fprintf(stderr, "[OcrTextExtractor::extractTextRegions] Step 4: ✓ TessBaseAPI created\n");
            fflush(stderr);
        } catch (const std::exception& e) {
            fprintf(stderr, "[OcrTextExtractor::extractTextRegions] EXCEPTION creating TessBaseAPI: %s\n", e.what());
            fflush(stderr);
            qCritical() << "[OcrTextExtractor::extractTextRegions] Exception creating API:" << e.what();
            return regions;
        } catch (...) {
            fprintf(stderr, "[OcrTextExtractor::extractTextRegions] UNKNOWN EXCEPTION creating TessBaseAPI\n");
            fflush(stderr);
            qCritical() << "[OcrTextExtractor::extractTextRegions] Unknown exception creating API";
            return regions;
        }
        
        fprintf(stderr, "[OcrTextExtractor::extractTextRegions] Step 5: Finding tessdata directory...\n");
        fflush(stderr);
        qDebug() << "OcrTextExtractor: Initializing Tesseract with language 'eng'...";
        qDebug() << "OcrTextExtractor: This may take 10-30 seconds on first run...";
        fflush(stdout);
        
        // Try to find tessdata directory
        QString tessdataPath = "/opt/homebrew/Cellar/tesseract/5.5.1_1/share/tessdata";
        fprintf(stderr, "[OcrTextExtractor::extractTextRegions] Trying tessdata path 1: %s\n", tessdataPath.toLocal8Bit().constData());
        fflush(stderr);
        if (!QDir(tessdataPath).exists()) {
            tessdataPath = "/opt/homebrew/share/tessdata";
            fprintf(stderr, "[OcrTextExtractor::extractTextRegions] Trying tessdata path 2: %s\n", tessdataPath.toLocal8Bit().constData());
            fflush(stderr);
        }
        if (!QDir(tessdataPath).exists()) {
            tessdataPath = "/usr/local/share/tessdata";
            fprintf(stderr, "[OcrTextExtractor::extractTextRegions] Trying tessdata path 3: %s\n", tessdataPath.toLocal8Bit().constData());
            fflush(stderr);
        }
        if (!QDir(tessdataPath).exists()) {
            tessdataPath = "";  // Let Tesseract find it automatically
            fprintf(stderr, "[OcrTextExtractor::extractTextRegions] No tessdata found, using auto-detect\n");
            fflush(stderr);
        } else {
            fprintf(stderr, "[OcrTextExtractor::extractTextRegions] ✓ Found tessdata at: %s\n", tessdataPath.toLocal8Bit().constData());
            fflush(stderr);
        }
        
        qDebug() << "OcrTextExtractor: Using tessdata path:" << (tessdataPath.isEmpty() ? "auto-detect" : tessdataPath);
        fflush(stdout);
        
        fprintf(stderr, "[OcrTextExtractor::extractTextRegions] Step 6: Initializing Tesseract...\n");
        fprintf(stderr, "[OcrTextExtractor::extractTextRegions] WARNING: This may take 10-30 seconds!\n");
        fflush(stderr);
        
        const char* tessdata = tessdataPath.isEmpty() ? NULL : tessdataPath.toLocal8Bit().constData();
        int initResult = 0;
        try {
            initResult = api->Init(tessdata, "eng");
            fprintf(stderr, "[OcrTextExtractor::extractTextRegions] Step 6: Init() returned with code: %d\n", initResult);
            fflush(stderr);
        } catch (const std::exception& e) {
            fprintf(stderr, "[OcrTextExtractor::extractTextRegions] EXCEPTION in api->Init(): %s\n", e.what());
            fflush(stderr);
            qCritical() << "[OcrTextExtractor::extractTextRegions] Exception in Init:" << e.what();
            delete api;
            return regions;
        } catch (...) {
            fprintf(stderr, "[OcrTextExtractor::extractTextRegions] UNKNOWN EXCEPTION in api->Init()\n");
            fflush(stderr);
            qCritical() << "[OcrTextExtractor::extractTextRegions] Unknown exception in Init";
            delete api;
            return regions;
        }
        
        if (initResult) {
            fprintf(stderr, "[OcrTextExtractor::extractTextRegions] ERROR: Tesseract initialization failed (code: %d)\n", initResult);
            fflush(stderr);
            qWarning() << "OcrTextExtractor: Could not initialize Tesseract (error code:" << initResult << ")";
            qWarning() << "OcrTextExtractor: Tried tessdata path:" << (tessdataPath.isEmpty() ? "auto-detect" : tessdataPath);
            delete api;
            return regions;
        }
        
        fprintf(stderr, "[OcrTextExtractor::extractTextRegions] Step 6: ✓ Tesseract initialized successfully\n");
        fflush(stderr);
    
    qDebug() << "OcrTextExtractor: ✓ Tesseract initialized successfully";
    fflush(stdout);
    
    // Configure Tesseract
    configureTesseract(api);
    
    // Set image directly from cv::Mat data (simpler than converting to Pix)
    // Tesseract can accept raw image data
    int width = preprocessed.cols;
    int height = preprocessed.rows;
    int bytesPerPixel = 1;  // Grayscale = 1 byte per pixel
    int bytesPerLine = preprocessed.step;  // Step size (may include padding)
    
    // Set image to Tesseract using raw data
    // NOTE: preprocessed is cloned above to ensure data stays valid
    qDebug() << "OcrTextExtractor: Setting image to Tesseract (" << width << "x" << height << ")...";
    qDebug() << "OcrTextExtractor: Data pointer:" << (void*)preprocessed.data << "step:" << bytesPerLine;
    fflush(stdout);
    
    // Validate data pointer is valid
    if (preprocessed.data == nullptr) {
        qWarning() << "OcrTextExtractor: Image data pointer is null!";
        api->End();
        delete api;
        return regions;
    }
    
    api->SetImage(preprocessed.data, width, height, bytesPerPixel, bytesPerLine);
    
    // Perform OCR - THIS IS THE SLOW PART (can take 30-120 seconds)
    double megapixels = width * height / 1000000.0;
    fprintf(stderr, "[OcrTextExtractor::extractTextRegions] Starting OCR recognition...\n");
    fprintf(stderr, "[OcrTextExtractor::extractTextRegions] Processing %.2f megapixels\n", megapixels);
    fprintf(stderr, "[OcrTextExtractor::extractTextRegions] WARNING: This may take 1-2 minutes for large documents\n");
    fprintf(stderr, "[OcrTextExtractor::extractTextRegions] Estimated time: %.0f-%.0f seconds based on image size\n", 
            megapixels * 10, megapixels * 30);
    fflush(stderr);
    
    qDebug() << "OcrTextExtractor: Starting OCR recognition...";
    qDebug() << "OcrTextExtractor: ⏳ Processing" << QString::number(megapixels, 'f', 2) << "megapixels...";
    qDebug() << "OcrTextExtractor: ⚠️  This may take 1-2 minutes. Tesseract diagnostic messages are normal.";
    qDebug() << "OcrTextExtractor: ⚠️  If you see 'Estimating resolution', Tesseract is working - please wait.";
    fflush(stdout);
    
    QElapsedTimer ocrTimer;
    ocrTimer.start();
    
    // #region agent log
    debugLog("OcrTextExtractor.cpp:extractTextRegions", "Before Recognize call", {{"width", width}, {"height", height}, {"dataPtr", QString::number(reinterpret_cast<quintptr>(preprocessed.data), 16)}});
    // #endregion
    
    qDebug() << "OcrTextExtractor: Calling api->Recognize(0) NOW...";
    qDebug() << "OcrTextExtractor: This is a blocking call - the application is working, please be patient";
    fflush(stdout);
    
    // Recognize() is a blocking call - this is where it hangs (but it's working!)
    // Note: This can take 30-120 seconds for a full page
    // CRITICAL: Keep 'preprocessed' cv::Mat alive during this call!
    // 
    // On macOS, there may be threading issues with Qt. Try processing events
    // to prevent deadlocks, but Recognize() itself is blocking.
    
    // #region agent log
    debugLog("OcrTextExtractor.cpp:extractTextRegions", "About to call Recognize(0)");
    // #endregion
    
    fprintf(stderr, "[OcrTextExtractor::extractTextRegions] About to call Recognize(0) - this may take 1-2 minutes\n");
    fprintf(stderr, "[OcrTextExtractor::extractTextRegions] The application is NOT frozen - OCR is processing in the background\n");
    fprintf(stderr, "[OcrTextExtractor::extractTextRegions] Current thread: %p\n", (void*)QThread::currentThread());
    fprintf(stderr, "[OcrTextExtractor::extractTextRegions] Calling api->Recognize(0) NOW - this will block the thread...\n");
    fflush(stderr);
    
    int recognizeResult = api->Recognize(0);
    
    fprintf(stderr, "[OcrTextExtractor::extractTextRegions] ✓ api->Recognize(0) RETURNED!\n");
    fflush(stderr);
    
    // #region agent log
    debugLog("OcrTextExtractor.cpp:extractTextRegions", "Recognize(0) returned", {{"result", recognizeResult}, {"elapsedMs", ocrTimer.elapsed()}});
    // #endregion
    
    fprintf(stderr, "[OcrTextExtractor::extractTextRegions] Recognize(0) returned with code: %d (took %lld ms)\n", recognizeResult, ocrTimer.elapsed());
    fprintf(stderr, "[OcrTextExtractor::extractTextRegions] Step 6: About to get ResultIterator...\n");
    fflush(stderr);
    
    // If we get here, Recognize() completed (or errored)
    qDebug() << "OcrTextExtractor: api->Recognize(0) call returned";
    fflush(stdout);
    
    qint64 ocrTime = ocrTimer.elapsed();
    
    qDebug() << "OcrTextExtractor: api->Recognize(0) returned with code:" << recognizeResult;
    qDebug() << "OcrTextExtractor: ✓ OCR recognition completed in" << QString::number(ocrTime / 1000.0, 'f', 2) << "seconds";
    fflush(stdout);
    
    if (recognizeResult != 0) {
        qWarning() << "OcrTextExtractor: Recognize() returned error code:" << recognizeResult;
        delete api;
        return regions;
    }
    
    // Extract text regions using ResultIterator
    qDebug() << "OcrTextExtractor: Extracting text regions...";
    fflush(stdout);
    
    // #region agent log
    debugLog("OcrTextExtractor.cpp:extractTextRegions", "Before GetIterator", {{"apiNull", api == nullptr}});
    // #endregion
    
    fprintf(stderr, "[OcrTextExtractor] About to call GetIterator()\n");
    fflush(stderr);
    
    tesseract::ResultIterator* ri = nullptr;
    try {
        ri = api->GetIterator();
    } catch (...) {
        fprintf(stderr, "[OcrTextExtractor] Exception in GetIterator()!\n");
        fflush(stderr);
        // #region agent log
        debugLog("OcrTextExtractor.cpp:extractTextRegions", "Exception in GetIterator");
        // #endregion
        api->End();
        delete api;
        return regions;
    }
    
    fprintf(stderr, "[OcrTextExtractor] GetIterator() returned, ri=%p\n", (void*)ri);
    fflush(stderr);
    
    // #region agent log
    debugLog("OcrTextExtractor.cpp:extractTextRegions", "After GetIterator", {{"riNull", ri == nullptr}});
    // #endregion
    
    tesseract::PageIteratorLevel level = tesseract::RIL_WORD;
    
    if (ri != nullptr) {
        // #region agent log
        debugLog("OcrTextExtractor.cpp:extractTextRegions", "Starting iteration loop");
        // #endregion
        
        int blockId = 0;
        int lineId = 0;
        int wordId = 0;
        int currentBlockId = -1;
        int lastY = -1;
        int currentLineId = 0;
        int iterationCount = 0;
        
        // Check iterator validity before starting iteration
        // Verify we can safely call methods on the iterator
        try {
            // Try to get first word to verify iterator is valid
            const char* testWord = ri->GetUTF8Text(level);
            if (testWord == nullptr) {
                // Iterator might be empty, but we'll still try iteration
                fprintf(stderr, "[OcrTextExtractor] First word is null, iterator may be empty\n");
                fflush(stderr);
            } else {
                delete[] testWord;
            }
        } catch (...) {
            fprintf(stderr, "[OcrTextExtractor] Exception checking iterator validity, iterator may be invalid\n");
            fflush(stderr);
            delete ri;
            api->End();
            delete api;
            return regions;
        }
        
        // CRITICAL FIX: Use standard while loop pattern with explicit state management
        // Process first element, then check Next() before each subsequent iteration
        volatile bool hasNext = true;
        bool continueLoop = true;  // Additional flag to prevent optimization
        
        // Process first element (Tesseract iterator starts at first element)
        iterationCount++;
        volatile int currentIteration = iterationCount;
        
        // Main loop: process current element, then advance
        while (continueLoop) {
            
            if (currentIteration % 100 == 0 || currentIteration == 1 || currentIteration <= 5) {
                // #region agent log
                debugLog("OcrTextExtractor.cpp:extractTextRegions", "Iteration start", {{"count", currentIteration}});
                // #endregion
            }
            
            // #region agent log
            if (currentIteration <= 5 || currentIteration % 100 == 0) {
                debugLog("OcrTextExtractor.cpp:extractTextRegions", "Before GetUTF8Text", {{"count", currentIteration}});
            }
            // #endregion
            
            const char* word = nullptr;
            try {
                word = ri->GetUTF8Text(level);
            } catch (...) {
                fprintf(stderr, "[OcrTextExtractor] Exception in GetUTF8Text() at iteration %d\n", currentIteration);
                fflush(stderr);
                // #region agent log
                debugLog("OcrTextExtractor.cpp:extractTextRegions", "Exception in GetUTF8Text", {{"count", currentIteration}});
                // #endregion
                break;  // Exit loop on exception
            }
            
            // #region agent log
            if (currentIteration <= 5 || currentIteration % 100 == 0) {
                debugLog("OcrTextExtractor.cpp:extractTextRegions", "After GetUTF8Text", {{"count", currentIteration}, {"wordNull", word == nullptr}, {"wordLen", word != nullptr ? static_cast<int>(strlen(word)) : 0}});
            }
            // #endregion
            
            if (word != nullptr && strlen(word) > 0) {
                // #region agent log
                if (currentIteration <= 5 || currentIteration % 100 == 0) {
                    debugLog("OcrTextExtractor.cpp:extractTextRegions", "Before Confidence", {{"count", currentIteration}});
                }
                // #endregion
                
                float conf = 0.0f;
                try {
                    conf = ri->Confidence(level);
                } catch (...) {
                    fprintf(stderr, "[OcrTextExtractor] Exception in Confidence() at iteration %d\n", currentIteration);
                    fflush(stderr);
                    // #region agent log
                    debugLog("OcrTextExtractor.cpp:extractTextRegions", "Exception in Confidence", {{"count", currentIteration}});
                    // #endregion
                    delete[] word;
                    break;  // Exit loop on exception
                }
                
                // #region agent log
                if (currentIteration <= 5 || currentIteration % 100 == 0) {
                    debugLog("OcrTextExtractor.cpp:extractTextRegions", "Before BoundingBox", {{"count", currentIteration}});
                }
                // #endregion
                
                // Get bounding box
                int x1 = 0, y1 = 0, x2 = 0, y2 = 0;
                bool bboxOk = false;
                try {
                    bboxOk = ri->BoundingBox(level, &x1, &y1, &x2, &y2);
                } catch (...) {
                    fprintf(stderr, "[OcrTextExtractor] Exception in BoundingBox() at iteration %d\n", currentIteration);
                    fflush(stderr);
                    // #region agent log
                    debugLog("OcrTextExtractor.cpp:extractTextRegions", "Exception in BoundingBox", {{"count", currentIteration}});
                    // #endregion
                    delete[] word;
                    break;  // Exit loop on exception
                }
                
                if (bboxOk) {
                    
                    // #region agent log
                    if (currentIteration <= 5 || currentIteration % 100 == 0) {
                        debugLog("OcrTextExtractor.cpp:extractTextRegions", "After BoundingBox", {{"count", currentIteration}, {"x1", x1}, {"y1", y1}, {"x2", x2}, {"y2", y2}});
                    }
                    // #endregion
                    // Get block and line IDs
                    int block = 0;
                    try {
                        if (ri != nullptr) {
                            block = ri->BlockType();
                        }
                    } catch (...) {
                        fprintf(stderr, "[OcrTextExtractor] Exception in BlockType() at iteration %d\n", currentIteration);
                        fflush(stderr);
                        delete[] word;
                        break;  // Exit loop on exception
                    }
                    if (block != currentBlockId) {
                        currentBlockId = block;
                        blockId++;
                        lineId = 0;
                        wordId = 0;
                    }
                    
                    // Check if we're on a new line using Y-coordinate
                    // Use a simple approach: track last Y position
                    if (lastY == -1) {
                        lastY = y1;
                        lineId = currentLineId;
                        wordId = 0;
                    } else if (abs(y1 - lastY) > 5) {  // 5px tolerance for same line
                        currentLineId++;
                        lineId = currentLineId;
                        lastY = y1;
                        wordId = 0;
                    } else {
                        lineId = currentLineId;
                        wordId++;
                    }
                    
                    // Filter by confidence (expert recommendation: filter low-confidence OCR)
                    OCRTextRegion region;
                    region.text = QString::fromUtf8(word);
                    region.boundingBox = cv::Rect(x1, y1, x2 - x1, y2 - y1);
                    region.confidence = static_cast<double>(conf);
                    region.blockId = blockId;
                    region.lineId = lineId;
                    region.wordId = wordId;
                    region.isLowConfidence = (conf < minConfidence);
                    
                    // Convert to normalized coordinates
                    int imgWidth = image.width();
                    int imgHeight = image.height();
                    if (imgWidth > 0 && imgHeight > 0) {
                        ImageCoords imgCoords(x1, y1, x2, y2);
                        region.coords = CoordinateSystem::imageToNormalized(imgCoords, imgWidth, imgHeight);
                    }
                    
                    // Infer type from text
                    region.typeHint = inferTypeFromText(region.text);
                    
                    // Only add high-confidence regions as primary hints
                    // Low-confidence regions marked but can be used for secondary validation
                    if (conf >= minConfidence) {
                        regions.append(region);
                    }
                    // Note: Low-confidence regions are discarded here, but could be stored separately
                    // for secondary validation if needed
                }
                
                // #region agent log
                if (iterationCount <= 5 || iterationCount % 100 == 0) {
                    debugLog("OcrTextExtractor.cpp:extractTextRegions", "Before delete[] word", {{"count", iterationCount}});
                }
                // #endregion
                
                if (word != nullptr) {
                    delete[] word;
                    word = nullptr;  // Prevent double delete
                }
                
                // #region agent log
                if (iterationCount <= 5 || iterationCount % 100 == 0) {
                    debugLog("OcrTextExtractor.cpp:extractTextRegions", "After delete[] word", {{"count", iterationCount}});
                }
                // #endregion
            } else {
                // #region agent log
                if (iterationCount <= 5 || iterationCount % 100 == 0) {
                    debugLog("OcrTextExtractor.cpp:extractTextRegions", "Word null or empty, skipping", {{"count", iterationCount}});
                }
                // #endregion
            }
            
            // Safety check: prevent infinite loops
            // Tesseract should typically find < 1000 words per page, so 5000 is a safe upper limit
            // CRITICAL FIX: Check using volatile variable to ensure compiler doesn't optimize it away
            if (currentIteration > 5000) {
                fprintf(stderr, "[OcrTextExtractor] ERROR: Iteration count exceeded 5000, breaking loop to prevent infinite loop\n");
                fprintf(stderr, "[OcrTextExtractor] This suggests ri->Next() is not working correctly\n");
                fprintf(stderr, "[OcrTextExtractor] Current iteration: %d\n", currentIteration);
                fflush(stderr);
                // #region agent log
                debugLog("OcrTextExtractor.cpp:extractTextRegions", "Iteration limit exceeded - breaking", {{"count", currentIteration}});
                // #endregion
                hasNext = false;  // Explicitly set to false to break loop
                break;
            }
            
            // #region agent log
            if (currentIteration <= 5 || currentIteration % 100 == 0) {
                debugLog("OcrTextExtractor.cpp:extractTextRegions", "End of loop iteration", {{"count", currentIteration}});
            }
            // #endregion
            
            // CRITICAL FIX: Call Next() at END of iteration, check result explicitly
            // Store in volatile variable and update continueLoop to prevent optimization
            try {
                // Validate iterator is still valid before calling Next()
                if (ri == nullptr) {
                    fprintf(stderr, "[OcrTextExtractor] ERROR: Iterator became null at iteration %d, breaking loop\n", currentIteration);
                    fflush(stderr);
                    hasNext = false;
                    continueLoop = false;
                    break;
                }
                hasNext = ri->Next(level);
                // Force memory barrier by using the value
                volatile bool nextResult = hasNext;
                continueLoop = nextResult;  // Update non-volatile flag from volatile
                
                // Log the result for debugging
                if (currentIteration <= 5 || currentIteration % 100 == 0) {
                    fprintf(stderr, "[OcrTextExtractor] After Next() call: hasNext=%d, continueLoop=%d, iteration=%d\n", 
                            hasNext ? 1 : 0, continueLoop ? 1 : 0, currentIteration);
                    fflush(stderr);
                }
            } catch (...) {
                fprintf(stderr, "[OcrTextExtractor] Exception in Next() at iteration %d, breaking loop\n", currentIteration);
                fflush(stderr);
                hasNext = false;
                continueLoop = false;
                break;
            }
            
            // CRITICAL FIX: Explicit check - if Next() returned false, MUST break
            // Use both checks to ensure loop termination
            if (!hasNext || !continueLoop) {
                fprintf(stderr, "[OcrTextExtractor] Next() returned false, breaking loop at iteration %d\n", currentIteration);
                fflush(stderr);
                continueLoop = false;
                break;  // Force break
            }
            
            // Increment for next iteration
            iterationCount++;
            currentIteration = iterationCount;
            
        }  // End while loop - continues only if continueLoop is true
        
        fprintf(stderr, "[OcrTextExtractor] Loop completed. Total iterations: %d\n", iterationCount);
        fflush(stderr);
        fprintf(stderr, "[OcrTextExtractor] Regions found: %d\n", static_cast<int>(regions.size()));
        fflush(stderr);
        
        fprintf(stderr, "[OcrTextExtractor::extractTextRegions] Step 1: Cleaning up iterator (must be deleted before api->End())\n");
        fflush(stderr);
        
        // CRITICAL: Iterator must be deleted BEFORE api->End() is called (Tesseract requirement)
        // Safety: Verify iterator is still valid before deletion
        if (ri != nullptr) {
            fprintf(stderr, "[OcrTextExtractor::extractTextRegions] Step 1.1: Iterator is not null, deleting...\n");
            fflush(stderr);
            try {
                delete ri;
                ri = nullptr;  // Prevent double delete
                fprintf(stderr, "[OcrTextExtractor::extractTextRegions] Step 1.1: ✓ Iterator deleted successfully\n");
                fflush(stderr);
            } catch (const std::exception& e) {
                fprintf(stderr, "[OcrTextExtractor::extractTextRegions] EXCEPTION deleting iterator: %s\n", e.what());
                fflush(stderr);
                ri = nullptr;  // Mark as deleted even if exception
            } catch (...) {
                fprintf(stderr, "[OcrTextExtractor::extractTextRegions] UNKNOWN EXCEPTION deleting iterator\n");
                fflush(stderr);
                ri = nullptr;  // Mark as deleted even if exception
            }
        } else {
            fprintf(stderr, "[OcrTextExtractor::extractTextRegions] Step 1.1: Iterator is null, skipping delete\n");
            fflush(stderr);
        }
        
        fprintf(stderr, "[OcrTextExtractor::extractTextRegions] Step 1: ✓ Iterator cleanup complete\n");
        fflush(stderr);
    }
    
        // Cleanup API - CRITICAL FIX: Only delete once, with safety check
        fprintf(stderr, "[OcrTextExtractor::extractTextRegions] Step 8: Cleaning up API...\n");
        fflush(stderr);
        if (api != nullptr) {
            try {
                fprintf(stderr, "[OcrTextExtractor::extractTextRegions] Step 8.1: Calling api->End()...\n");
                fflush(stderr);
                api->End();
                fprintf(stderr, "[OcrTextExtractor::extractTextRegions] Step 8.1: ✓ api->End() completed\n");
                fflush(stderr);
            } catch (const std::exception& e) {
                fprintf(stderr, "[OcrTextExtractor::extractTextRegions] EXCEPTION in api->End(): %s\n", e.what());
                fflush(stderr);
            } catch (...) {
                fprintf(stderr, "[OcrTextExtractor::extractTextRegions] UNKNOWN EXCEPTION in api->End()\n");
                fflush(stderr);
            }
            
            try {
                fprintf(stderr, "[OcrTextExtractor::extractTextRegions] Step 8.2: Deleting api object...\n");
                fflush(stderr);
                delete api;
                api = nullptr;  // Prevent double delete
                fprintf(stderr, "[OcrTextExtractor::extractTextRegions] Step 8.2: ✓ api deleted successfully\n");
                fflush(stderr);
            } catch (const std::exception& e) {
                fprintf(stderr, "[OcrTextExtractor::extractTextRegions] EXCEPTION deleting api: %s\n", e.what());
                fflush(stderr);
                api = nullptr;  // Mark as deleted even if exception
            } catch (...) {
                fprintf(stderr, "[OcrTextExtractor::extractTextRegions] UNKNOWN EXCEPTION deleting api\n");
                fflush(stderr);
                api = nullptr;  // Mark as deleted even if exception
            }
        }
        
        fprintf(stderr, "[OcrTextExtractor::extractTextRegions] Step 9: Preparing to return %d regions\n", static_cast<int>(regions.size()));
        fflush(stderr);
        
        // Create a copy to ensure regions are valid when returned
        QList<OCRTextRegion> returnRegions = regions;
        fprintf(stderr, "[OcrTextExtractor::extractTextRegions] Step 10: Created copy, returning now\n");
        fprintf(stderr, "[OcrTextExtractor::extractTextRegions] ========== OCR EXTRACTION SUCCESS ==========\n");
        fflush(stderr);
        
        return returnRegions;
        
    } catch (const std::exception& e) {
        fprintf(stderr, "[OcrTextExtractor::extractTextRegions] CRITICAL EXCEPTION: %s\n", e.what());
        fprintf(stderr, "[OcrTextExtractor::extractTextRegions] ========== OCR EXTRACTION FAILED ==========\n");
        fflush(stderr);
        qCritical() << "[OcrTextExtractor::extractTextRegions] Exception:" << e.what();
        
        // Cleanup on error
        if (api != nullptr) {
            try {
                api->End();
            } catch (...) {
                // Ignore cleanup errors
            }
            try {
                delete api;
            } catch (...) {
                // Ignore cleanup errors
            }
            api = nullptr;
        }
        
        return regions; // Return empty list on error
    } catch (...) {
        fprintf(stderr, "[OcrTextExtractor::extractTextRegions] CRITICAL UNKNOWN EXCEPTION\n");
        fprintf(stderr, "[OcrTextExtractor::extractTextRegions] ========== OCR EXTRACTION FAILED ==========\n");
        fflush(stderr);
        qCritical() << "[OcrTextExtractor::extractTextRegions] Unknown exception";
        
        // Cleanup on error
        if (api != nullptr) {
            try {
                api->End();
            } catch (...) {
                // Ignore cleanup errors
            }
            try {
                delete api;
            } catch (...) {
                // Ignore cleanup errors
            }
            api = nullptr;
        }
        
        return regions; // Return empty list on error
    }
}

cv::Mat OcrTextExtractor::preprocessImage(const QImage& image)
{
    // Convert QImage to cv::Mat
    cv::Mat cvImage = ImageConverter::qImageToMat(image);
    
    // Convert to grayscale if needed
    cv::Mat gray;
    if (cvImage.channels() == 3) {
        cv::cvtColor(cvImage, gray, cv::COLOR_BGR2GRAY);
    } else if (cvImage.channels() == 4) {
        cv::cvtColor(cvImage, gray, cv::COLOR_BGRA2GRAY);
    } else {
        gray = cvImage.clone();
    }
    
    // Apply Gaussian blur for noise reduction (3x3 kernel)
    cv::Mat blurred;
    cv::GaussianBlur(gray, blurred, cv::Size(3, 3), 0);
    
    // Apply adaptive thresholding
    // Block size 11, C=2 (Gaussian method)
    cv::Mat binary;
    cv::adaptiveThreshold(blurred, binary, 255, 
                         cv::ADAPTIVE_THRESH_GAUSSIAN_C, 
                         cv::THRESH_BINARY, 11, 2);
    
    return binary;
}

void OcrTextExtractor::configureTesseract(void* apiPtr)
{
    tesseract::TessBaseAPI* api = static_cast<tesseract::TessBaseAPI*>(apiPtr);
    
    // Set PSM mode to 11 (Sparse Text) for forms
    // This handles scattered text elements effectively
    api->SetPageSegMode(tesseract::PSM_SPARSE_TEXT);
    
    // Alternative: PSM_SINGLE_BLOCK (6) for uniform forms
    // Can be made configurable later
    // api->SetPageSegMode(tesseract::PSM_SINGLE_BLOCK);
}

QList<OCRTextRegion> OcrTextExtractor::extractTextRegionsWithMultiplePSM(const QImage& image)
{
    QList<OCRTextRegion> bestResult;
    double bestAvgConfidence = 0.0;
    
    if (image.isNull()) {
        return bestResult;
    }
    
    // Preprocess image once (shared across all PSM modes)
    cv::Mat preprocessed = preprocessImage(image);
    
    // Try multiple PSM modes (expert recommendation)
    QList<int> psmModes = {
        tesseract::PSM_AUTO_OSD,      // 0: Automatic page segmentation with orientation and script detection
        tesseract::PSM_SINGLE_BLOCK,  // 6: Assume a single uniform block of text
        tesseract::PSM_SPARSE_TEXT    // 11: Sparse text (current default)
    };
    
    for (int psm : psmModes) {
        // Initialize Tesseract for this PSM mode
        tesseract::TessBaseAPI* api = new tesseract::TessBaseAPI();
        if (api->Init(NULL, "eng")) {
            delete api;
            continue;  // Skip this mode if initialization fails
        }
        
        // Set PSM mode
        api->SetPageSegMode(static_cast<tesseract::PageSegMode>(psm));
        
        // Set image
        int width = preprocessed.cols;
        int height = preprocessed.rows;
        int bytesPerPixel = 1;
        int bytesPerLine = preprocessed.step;
        api->SetImage(preprocessed.data, width, height, bytesPerPixel, bytesPerLine);
        
        // Perform OCR
        api->Recognize(0);
        
        // Extract text regions
        QList<OCRTextRegion> regions;
        tesseract::ResultIterator* ri = api->GetIterator();
        tesseract::PageIteratorLevel level = tesseract::RIL_WORD;
        
        if (ri != nullptr) {
            int blockId = 0;
            int lineId = 0;
            int wordId = 0;
            int currentBlockId = -1;
            int lastY = -1;
            int currentLineId = 0;
            
            do {
                const char* word = ri->GetUTF8Text(level);
                if (word != nullptr && strlen(word) > 0) {
                    float conf = ri->Confidence(level);
                    
                    int x1, y1, x2, y2;
                    if (ri->BoundingBox(level, &x1, &y1, &x2, &y2)) {
                        int block = ri->BlockType();
                        if (block != currentBlockId) {
                            currentBlockId = block;
                            blockId++;
                            lineId = 0;
                            wordId = 0;
                        }
                        
                        if (lastY == -1) {
                            lastY = y1;
                            lineId = currentLineId;
                            wordId = 0;
                        } else if (abs(y1 - lastY) > 5) {
                            currentLineId++;
                            lineId = currentLineId;
                            lastY = y1;
                            wordId = 0;
                        } else {
                            lineId = currentLineId;
                            wordId++;
                        }
                        
                        // Filter by confidence (expert recommendation)
                        OCRTextRegion region;
                        region.text = QString::fromUtf8(word);
                        region.boundingBox = cv::Rect(x1, y1, x2 - x1, y2 - y1);
                        region.confidence = static_cast<double>(conf);
                        region.blockId = blockId;
                        region.lineId = lineId;
                        region.wordId = wordId;
                        region.isLowConfidence = (conf < minConfidence);
                        
                        int imgWidth = image.width();
                        int imgHeight = image.height();
                        if (imgWidth > 0 && imgHeight > 0) {
                            ImageCoords imgCoords(x1, y1, x2, y2);
                            region.coords = CoordinateSystem::imageToNormalized(imgCoords, imgWidth, imgHeight);
                        }
                        
                        region.typeHint = inferTypeFromText(region.text);
                        
                        // Only add high-confidence regions
                        if (conf >= minConfidence) {
                            regions.append(region);
                        }
                    }
                    
                    delete[] word;
                }
            } while (ri->Next(level));
            
            delete ri;
        }
        
        // Calculate average confidence
        double avgConfidence = 0.0;
        if (!regions.isEmpty()) {
            double totalConf = 0.0;
            for (const OCRTextRegion& region : regions) {
                totalConf += region.confidence;
            }
            avgConfidence = totalConf / regions.size();
        }
        
        // Keep best result (highest average confidence)
        if (avgConfidence > bestAvgConfidence) {
            bestAvgConfidence = avgConfidence;
            bestResult = regions;
        }
        
        // Cleanup
        api->End();
        delete api;
    }
    
    // If all PSM modes failed, fall back to single mode
    if (bestResult.isEmpty()) {
        return extractTextRegions(image);
    }
    
    return bestResult;
}

QList<OCRTextRegion> OcrTextExtractor::filterByConfidence(const QList<OCRTextRegion>& regions, double minConf)
{
    QList<OCRTextRegion> filtered;
    
    for (const OCRTextRegion& region : regions) {
        if (region.confidence >= minConf) {
            filtered.append(region);
        }
    }
    
    return filtered;
}

QString OcrTextExtractor::inferTypeFromText(const QString& text)
{
    if (text.isEmpty()) {
        return "unknown";
    }
    
    bool hasLetters = false;
    bool hasNumbers = false;
    
    for (const QChar& ch : text) {
        if (ch.isLetter()) {
            hasLetters = true;
        } else if (ch.isDigit()) {
            hasNumbers = true;
        }
    }
    
    if (hasLetters && hasNumbers) {
        return "mixed";
    } else if (hasNumbers) {
        return "numbers";
    } else if (hasLetters) {
        return "letters";
    } else {
        return "unknown";
    }
}

} // namespace ocr_orc

#include "OcrTextExtractor.h"
#include "ImageConverter.h"
#include "../core/CoordinateSystem.h"
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <QtCore/QDebug>
#include <algorithm>
#include <cmath>

namespace ocr_orc {

OcrTextExtractor::OcrTextExtractor()
    : minConfidence(50.0)
{
}

OcrTextExtractor::~OcrTextExtractor()
{
}

QList<OCRTextRegion> OcrTextExtractor::extractTextRegions(const QImage& image)
{
    QList<OCRTextRegion> regions;
    
    if (image.isNull()) {
        return regions;
    }
    
    // Preprocess image
    cv::Mat preprocessed = preprocessImage(image);
    
    // Initialize Tesseract
    tesseract::TessBaseAPI* api = new tesseract::TessBaseAPI();
    if (api->Init(NULL, "eng")) {
        qWarning() << "OcrTextExtractor: Could not initialize Tesseract";
        delete api;
        return regions;
    }
    
    // Configure Tesseract
    configureTesseract(api);
    
    // Set image directly from cv::Mat data (simpler than converting to Pix)
    // Tesseract can accept raw image data
    int width = preprocessed.cols;
    int height = preprocessed.rows;
    int bytesPerPixel = 1;  // Grayscale = 1 byte per pixel
    int bytesPerLine = preprocessed.step;  // Step size (may include padding)
    
    // Set image to Tesseract using raw data
    api->SetImage(preprocessed.data, width, height, bytesPerPixel, bytesPerLine);
    
    // Perform OCR
    api->Recognize(0);
    
    // Extract text regions using ResultIterator
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
                
                // Get bounding box
                int x1, y1, x2, y2;
                if (ri->BoundingBox(level, &x1, &y1, &x2, &y2)) {
                    // Get block and line IDs
                    int block = ri->BlockType();
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
                    
                    // Filter by confidence
                    if (conf >= minConfidence) {
                        OCRTextRegion region;
                        region.text = QString::fromUtf8(word);
                        region.boundingBox = cv::Rect(x1, y1, x2 - x1, y2 - y1);
                        region.confidence = static_cast<double>(conf);
                        region.blockId = blockId;
                        region.lineId = lineId;
                        region.wordId = wordId;
                        
                        // Convert to normalized coordinates
                        int imgWidth = image.width();
                        int imgHeight = image.height();
                        if (imgWidth > 0 && imgHeight > 0) {
                            ImageCoords imgCoords(x1, y1, x2, y2);
                            region.coords = CoordinateSystem::imageToNormalized(imgCoords, imgWidth, imgHeight);
                        }
                        
                        // Infer type from text
                        region.typeHint = inferTypeFromText(region.text);
                        
                        regions.append(region);
                    }
                }
                
                delete[] word;
            }
        } while (ri->Next(level));
        
        delete ri;
    }
    
    // Cleanup
    api->End();
    delete api;
    
    return regions;
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

#include "PdfLoader.h"
#include "../core/Constants.h"
#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-page.h>
#include <poppler/cpp/poppler-page-renderer.h>
#include <poppler/cpp/poppler-image.h>
#include <QtCore/QFileInfo>
#if OCR_ORC_DEBUG_ENABLED
#include <QtCore/QDebug>
#endif
#include <memory>
#include <stdexcept>

namespace ocr_orc {

QImage PdfLoader::loadPdfFirstPage(const QString& filePath, int dpi) {
    // Validate DPI
    if (dpi < PdfConstants::MIN_DPI || dpi > PdfConstants::MAX_DPI) {
        OCR_ORC_WARNING("PdfLoader: Invalid DPI, using default:" << PdfConstants::DEFAULT_DPI);
        dpi = PdfConstants::DEFAULT_DPI;
    }
    
    // Check if file exists
    if (!QFileInfo::exists(filePath)) {
        OCR_ORC_WARNING("PdfLoader: File does not exist:" << filePath);
        return QImage();
    }
    
    // Convert QString to std::string for Poppler
    std::string filePathStd = filePath.toStdString();
    
    // Load PDF document using smart pointer for automatic cleanup
    std::unique_ptr<poppler::document> doc(poppler::document::load_from_file(filePathStd));
    if (!doc) {
        OCR_ORC_WARNING("PdfLoader: Failed to load PDF:" << filePath);
        return QImage();
    }
    
    // Check if document is locked (password-protected)
    if (doc->is_locked()) {
        OCR_ORC_WARNING("PdfLoader: PDF is password-protected:" << filePath);
        return QImage();
    }
    
    // Check if document has pages
    int pageCount = doc->pages();
    if (pageCount <= 0) {
        OCR_ORC_WARNING("PdfLoader: PDF has no pages:" << filePath);
        return QImage();
    }
    
    // Get first page using smart pointer
    std::unique_ptr<poppler::page> page(doc->create_page(0));
    if (!page) {
        OCR_ORC_WARNING("PdfLoader: Failed to create page 0:" << filePath);
        return QImage();
    }
    
    // Create page renderer
    poppler::page_renderer renderer;
    renderer.set_render_hint(poppler::page_renderer::antialiasing, true);
    renderer.set_render_hint(poppler::page_renderer::text_antialiasing, true);
    renderer.set_image_format(poppler::image::format_argb32);
    
    // Render page to poppler::image (with error handling)
    poppler::image popplerImg;
    try {
        popplerImg = renderer.render_page(page.get(), dpi, dpi);
    } catch (const std::exception& e) {
        OCR_ORC_WARNING("PdfLoader: Exception during rendering:" << e.what());
        return QImage();
    } catch (...) {
        OCR_ORC_WARNING("PdfLoader: Unknown exception during rendering");
        return QImage();
    }
    
    // Smart pointers automatically clean up doc and page
    
    // Check if rendering was successful
    if (!popplerImg.is_valid()) {
        OCR_ORC_WARNING("PdfLoader: Failed to render page:" << filePath);
        return QImage();
    }
    
    // Convert poppler::image to QImage
    int width = popplerImg.width();
    int height = popplerImg.height();
    const char* data = popplerImg.const_data();
    int bytesPerRow = popplerImg.bytes_per_row();
    
    // Create QImage from poppler image data
    // poppler::image format_argb32 is ARGB32, which matches QImage::Format_ARGB32
    QImage qImage(reinterpret_cast<const uchar*>(data), width, height, bytesPerRow, QImage::Format_ARGB32);
    
    // Make a copy since popplerImg will be destroyed
    QImage result = qImage.copy();
    
    return result;
}

bool PdfLoader::isValidPdf(const QString& filePath) {
    if (!QFileInfo::exists(filePath)) {
        return false;
    }
    
    std::string filePathStd = filePath.toStdString();
    std::unique_ptr<poppler::document> doc(poppler::document::load_from_file(filePathStd));
    
    if (!doc) {
        return false;
    }
    
    bool isValid = !doc->is_locked() && doc->pages() > 0;
    // Smart pointer automatically cleans up
    
    return isValid;
}

int PdfLoader::getPageCount(const QString& filePath) {
    if (!QFileInfo::exists(filePath)) {
        return -1;
    }
    
    std::string filePathStd = filePath.toStdString();
    std::unique_ptr<poppler::document> doc(poppler::document::load_from_file(filePathStd));
    
    if (!doc || doc->is_locked()) {
        return -1;
    }
    
    int pageCount = doc->pages();
    // Smart pointer automatically cleans up
    
    return pageCount;
}

} // namespace ocr_orc


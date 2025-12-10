#ifndef PDF_LOADER_H
#define PDF_LOADER_H

#include "../core/Constants.h"
#include <QtCore/QString>
#include <QtGui/QImage>

namespace ocr_orc {

/**
 * @brief Utility class for loading and rendering PDF files
 * 
 * Uses Poppler C++ library to load PDF documents and render pages to QImage.
 * Handles errors gracefully and provides a simple interface for PDF loading.
 */
class PdfLoader {
public:
    /**
     * @brief Load the first page of a PDF file and render it to a QImage
     * 
     * @param filePath Path to the PDF file
     * @param dpi Resolution for rendering (default: 150 DPI)
     * @return QImage containing the rendered page, or empty QImage on error
     * 
     * @note Uses 150 DPI by default for a balance between quality and memory usage.
     *       Higher DPI (e.g., 300) produces better quality but uses more memory.
     */
    static QImage loadPdfFirstPage(const QString& filePath, int dpi = PdfConstants::DEFAULT_DPI);
    
    /**
     * @brief Check if a file is a valid PDF
     * 
     * @param filePath Path to the file to check
     * @return true if the file is a valid PDF, false otherwise
     */
    static bool isValidPdf(const QString& filePath);
    
    /**
     * @brief Get the number of pages in a PDF file
     * 
     * @param filePath Path to the PDF file
     * @return Number of pages, or -1 on error
     */
    static int getPageCount(const QString& filePath);

private:
    // Private constructor - this is a utility class with only static methods
    PdfLoader() = delete;
};

} // namespace ocr_orc

#endif // PDF_LOADER_H


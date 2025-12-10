#ifndef CSV_EXPORTER_H
#define CSV_EXPORTER_H

#include "../models/DocumentState.h"
#include <QtCore/QString>

namespace ocr_orc {

/**
 * @brief CSV exporter for DocumentState
 * 
 * Exports regions to CSV format following RFC 4180 standard.
 * Coordinates are exported as percentages (0.0-1.0) with 6 decimal places.
 */
class CsvExporter {
public:
    /**
     * @brief Export DocumentState to CSV file
     * @param state DocumentState to export
     * @param filePath Path to output CSV file
     * @throws std::runtime_error if export fails
     */
    static void exportToFile(const DocumentState& state, const QString& filePath);
    
private:
    /**
     * @brief Escape CSV field according to RFC 4180
     * @param field Field value to escape
     * @return Escaped field string
     */
    static QString escapeCsvField(const QString& field);
    
    /**
     * @brief Format coordinate value to 6 decimal places
     * @param value Coordinate value (0.0-1.0)
     * @return Formatted string
     */
    static QString formatCoordinate(double value);
};

} // namespace ocr_orc

#endif // CSV_EXPORTER_H


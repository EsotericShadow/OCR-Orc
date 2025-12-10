#ifndef JSON_EXPORTER_H
#define JSON_EXPORTER_H

#include "../models/DocumentState.h"
#include <QtCore/QString>
#include <QtCore/QJsonObject>

namespace ocr_orc {

/**
 * @brief JSON exporter for DocumentState
 * 
 * Exports regions, groups, and metadata to JSON format.
 * Uses normalized coordinates (0.0-1.0) as the source of truth.
 */
class JsonExporter {
public:
    /**
     * @brief Export DocumentState to JSON file
     * @param state DocumentState to export
     * @param filePath Path to output JSON file
     * @throws std::runtime_error if export fails
     */
    static void exportToFile(const DocumentState& state, const QString& filePath);
    
private:
    /**
     * @brief Build root JSON object with all data
     * @param state DocumentState to export
     * @return QJsonObject containing all exported data
     */
    static QJsonObject buildRootObject(const DocumentState& state);
    
    /**
     * @brief Build regions JSON object
     * @param state DocumentState to export
     * @return QJsonObject containing all regions
     */
    static QJsonObject buildRegionsObject(const DocumentState& state);
    
    /**
     * @brief Build groups JSON object
     * @param state DocumentState to export
     * @return QJsonObject containing all groups
     */
    static QJsonObject buildGroupsObject(const DocumentState& state);
    
    /**
     * @brief Convert RegionData to JSON object
     * @param region RegionData to convert
     * @return QJsonObject representing the region
     */
    static QJsonObject regionToJson(const RegionData& region);
    
    /**
     * @brief Write JSON to file atomically (temp file + rename)
     * @param jsonData JSON data to write
     * @param filePath Target file path
     * @throws std::runtime_error if write fails
     */
    static void writeFileAtomically(const QByteArray& jsonData, const QString& filePath);
};

} // namespace ocr_orc

#endif // JSON_EXPORTER_H


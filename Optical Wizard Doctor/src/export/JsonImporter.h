#ifndef JSON_IMPORTER_H
#define JSON_IMPORTER_H

#include "../models/DocumentState.h"
#include "../core/CoordinateSystem.h"
#include <QtCore/QString>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonValue>

namespace ocr_orc {

/**
 * @brief JSON importer for DocumentState
 * 
 * Imports regions, groups, and metadata from JSON format.
 * Validates coordinates and reconstructs groups if missing.
 */
class JsonImporter {
public:
    /**
     * @brief Import DocumentState from JSON file
     * @param state DocumentState to populate
     * @param filePath Path to input JSON file
     * @throws std::runtime_error if import fails
     */
    static void importFromFile(DocumentState& state, const QString& filePath);
    
private:
    /**
     * @brief Parse coordinates from JSON value (supports dict and array formats)
     * @param value JSON value containing coordinates
     * @return NormalizedCoords structure
     * @throws std::runtime_error if coordinates are invalid
     */
    static NormalizedCoords parseCoordinates(const QJsonValue& value);
    
    /**
     * @brief Validate normalized coordinates are in valid range
     * @param coords Coordinates to validate
     * @return true if valid, false otherwise
     */
    static bool validateCoordinates(const NormalizedCoords& coords);
    
    /**
     * @brief Load regions from JSON object
     * @param state DocumentState to populate
     * @param regionsObj JSON object containing regions
     */
    static void loadRegions(DocumentState& state, const QJsonObject& regionsObj);
    
    /**
     * @brief Load groups from JSON object
     * @param state DocumentState to populate
     * @param groupsObj JSON object containing groups
     */
    static void loadGroups(DocumentState& state, const QJsonObject& groupsObj);
    
    /**
     * @brief Reconstruct groups from region data (backward compatibility)
     * @param state DocumentState to populate
     */
    static void reconstructGroups(DocumentState& state);
};

} // namespace ocr_orc

#endif // JSON_IMPORTER_H


#ifndef INPUT_VALIDATOR_H
#define INPUT_VALIDATOR_H

#include <QtCore/QString>
#include <QtCore/QList>
#include "../core/CoordinateSystem.h"
#include "../core/Constants.h"

namespace ocr_orc {

/**
 * @brief Utility class for validating user inputs
 */
class InputValidator {
public:
    /**
     * @brief Validate region name
     * @param name Region name to validate
     * @param existingNames List of existing region names (for uniqueness check)
     * @return Empty string if valid, error message otherwise
     */
    static QString validateRegionName(const QString& name, const QList<QString>& existingNames = QList<QString>());
    
    /**
     * @brief Validate group name
     * @param name Group name to validate
     * @param existingNames List of existing group names (for uniqueness check)
     * @return Empty string if valid, error message otherwise
     */
    static QString validateGroupName(const QString& name, const QList<QString>& existingNames = QList<QString>());
    
    /**
     * @brief Validate normalized coordinates
     * @param coords Coordinates to validate
     * @return Empty string if valid, error message otherwise
     */
    static QString validateNormalizedCoords(const NormalizedCoords& coords);
    
    /**
     * @brief Validate file path
     * @param filePath Path to validate
     * @param mustExist If true, file must exist
     * @return Empty string if valid, error message otherwise
     */
    static QString validateFilePath(const QString& filePath, bool mustExist = false);
    
    /**
     * @brief Validate DPI value
     * @param dpi DPI to validate
     * @return Empty string if valid, error message otherwise
     */
    static QString validateDpi(int dpi);
    
    /**
     * @brief Sanitize region name (remove invalid characters)
     * @param name Name to sanitize
     * @return Sanitized name
     */
    static QString sanitizeRegionName(const QString& name);
    
    /**
     * @brief Sanitize group name (remove invalid characters)
     * @param name Name to sanitize
     * @return Sanitized name
     */
    static QString sanitizeGroupName(const QString& name);
    
    /**
     * @brief Sanitize file path (prevent path traversal)
     * @param filePath Path to sanitize
     * @return Sanitized path, or empty string if invalid
     */
    static QString sanitizeFilePath(const QString& filePath);
};

} // namespace ocr_orc

#endif // INPUT_VALIDATOR_H


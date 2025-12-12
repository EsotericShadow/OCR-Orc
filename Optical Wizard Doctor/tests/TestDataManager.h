#ifndef TEST_DATA_MANAGER_H
#define TEST_DATA_MANAGER_H

#include "../src/core/CoordinateSystem.h"
#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QJsonObject>
#include <QtGui/QImage>

namespace ocr_orc {

/**
 * @brief Ground truth region annotation
 */
struct GroundTruthRegion {
    QString name;
    NormalizedCoords normalizedCoords;
    QString type;  // "text", "checkbox", "number", etc.
    QString group;  // Group name (e.g., "personal_info")
    double confidence;  // Expected confidence (if known)
    
    GroundTruthRegion()
        : confidence(1.0) {}
};

/**
 * @brief Ground truth annotation for a form
 */
struct GroundTruthAnnotation {
    QString formId;
    QString imagePath;
    QString pdfPath;
    QList<GroundTruthRegion> expectedRegions;
    QString documentType;  // "government", "medical", "tax", "handwritten"
    QMap<QString, QVariant> metadata;  // Additional metadata
    
    GroundTruthAnnotation() {}
};

/**
 * @brief Test data manager for loading PDF forms and ground truth annotations
 */
class TestDataManager {
public:
    TestDataManager();
    ~TestDataManager() = default;
    
    /**
     * @brief Set base directory for test data
     * @param baseDir Base directory (e.g., "tests/data")
     */
    void setBaseDirectory(const QString& baseDir);
    
    /**
     * @brief Load ground truth annotation from JSON file
     * @param annotationPath Path to JSON annotation file
     * @return Ground truth annotation
     */
    GroundTruthAnnotation loadGroundTruth(const QString& annotationPath) const;
    
    /**
     * @brief Load image from PDF or image file
     * @param imagePath Path to PDF or image file
     * @return Loaded image
     */
    QImage loadImage(const QString& imagePath) const;
    
    /**
     * @brief Get all available test forms
     * @return List of form IDs
     */
    QList<QString> getAvailableForms() const;
    
    /**
     * @brief Get ground truth for a specific form
     * @param formId Form ID
     * @return Ground truth annotation
     */
    GroundTruthAnnotation getGroundTruth(const QString& formId) const;
    
    /**
     * @brief Validate ground truth annotation
     * @param annotation Annotation to validate
     * @return True if valid, false otherwise
     */
    bool validateGroundTruth(const GroundTruthAnnotation& annotation) const;
    
    /**
     * @brief Create ground truth annotation template
     * @param formId Form ID
     * @param imagePath Path to image/PDF
     * @param documentType Document type
     * @return Template annotation (empty regions list)
     */
    GroundTruthAnnotation createTemplate(const QString& formId, const QString& imagePath,
                                        const QString& documentType) const;
    
    /**
     * @brief Save ground truth annotation to JSON file
     * @param annotation Annotation to save
     * @param filePath Path to output JSON file
     */
    void saveGroundTruth(const GroundTruthAnnotation& annotation, const QString& filePath) const;
    
    /**
     * @brief Get forms directory path
     * @return Forms directory path
     */
    QString getFormsDirectory() const;
    
    /**
     * @brief Get ground truth directory path
     * @return Ground truth directory path
     */
    QString getGroundTruthDirectory() const;

private:
    QString baseDirectory;
    QString formsDirectory;
    QString groundTruthDirectory;
    GroundTruthRegion parseRegion(const QJsonObject& regionObj) const;
    QJsonObject regionToJson(const GroundTruthRegion& region) const;
};

} // namespace ocr_orc

#endif // TEST_DATA_MANAGER_H

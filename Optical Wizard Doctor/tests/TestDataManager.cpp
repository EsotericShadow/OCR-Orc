#include "TestDataManager.h"
#include "../src/utils/PdfLoader.h"
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtCore/QDebug>
#include <QtGui/QImageReader>

namespace ocr_orc {

TestDataManager::TestDataManager()
    : baseDirectory("tests/data")
{
    // Initialize directory paths
    QDir baseDir(baseDirectory);
    formsDirectory = baseDir.absoluteFilePath("forms");
    groundTruthDirectory = baseDir.absoluteFilePath("ground_truth");
}

void TestDataManager::setBaseDirectory(const QString& baseDir)
{
    baseDirectory = baseDir;
    // Update directory paths
    QDir baseDirObj(baseDirectory);
    formsDirectory = baseDirObj.absoluteFilePath("forms");
    groundTruthDirectory = baseDirObj.absoluteFilePath("ground_truth");
}


GroundTruthAnnotation TestDataManager::loadGroundTruth(const QString& annotationPath) const
{
    GroundTruthAnnotation annotation;
    
    QFile file(annotationPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open annotation file:" << annotationPath;
        return annotation;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "Failed to parse JSON:" << error.errorString();
        return annotation;
    }
    
    QJsonObject root = doc.object();
    
    annotation.formId = root["formId"].toString();
    annotation.imagePath = root["imagePath"].toString();
    annotation.pdfPath = root.contains("pdfPath") ? root["pdfPath"].toString() : annotation.imagePath;
    annotation.documentType = root["documentType"].toString();
    
    // Parse expected regions
    if (root.contains("expectedRegions") && root["expectedRegions"].isArray()) {
        QJsonArray regionsArray = root["expectedRegions"].toArray();
        for (const QJsonValue& value : regionsArray) {
            if (value.isObject()) {
                GroundTruthRegion region = parseRegion(value.toObject());
                annotation.expectedRegions.append(region);
            }
        }
    }
    
    // Parse metadata
    if (root.contains("metadata") && root["metadata"].isObject()) {
        QJsonObject metadataObj = root["metadata"].toObject();
        for (auto it = metadataObj.begin(); it != metadataObj.end(); ++it) {
            annotation.metadata[it.key()] = it.value().toVariant();
        }
    }
    
    return annotation;
}

QImage TestDataManager::loadImage(const QString& imagePath) const
{
    QFileInfo fileInfo(imagePath);
    
    // Check if it's a PDF
    if (fileInfo.suffix().toLower() == "pdf") {
        QImage image = PdfLoader::loadPdfFirstPage(imagePath);
        if (!image.isNull()) {
            return image;
        }
        qWarning() << "Failed to load PDF:" << imagePath;
        return QImage();
    }
    
    // Try to load as image
    QImageReader reader(imagePath);
    QImage image = reader.read();
    if (image.isNull()) {
        qWarning() << "Failed to load image:" << imagePath << reader.errorString();
    }
    return image;
}

QList<QString> TestDataManager::getAvailableForms() const
{
    QList<QString> forms;
    
    QDir gtDir(groundTruthDirectory);
    if (!gtDir.exists()) {
        qWarning() << "Ground truth directory does not exist:" << groundTruthDirectory;
        return forms;
    }
    
    QStringList filters;
    filters << "*.json";
    QFileInfoList files = gtDir.entryInfoList(filters, QDir::Files);
    
    for (const QFileInfo& fileInfo : files) {
        QString formId = fileInfo.baseName();
        forms.append(formId);
    }
    
    return forms;
}

GroundTruthAnnotation TestDataManager::getGroundTruth(const QString& formId) const
{
    QString annotationPath = QDir(groundTruthDirectory).filePath(formId + ".json");
    return loadGroundTruth(annotationPath);
}

bool TestDataManager::validateGroundTruth(const GroundTruthAnnotation& annotation) const
{
    if (annotation.formId.isEmpty()) {
        qWarning() << "Form ID is empty";
        return false;
    }
    
    if (annotation.imagePath.isEmpty() && annotation.pdfPath.isEmpty()) {
        qWarning() << "Image/PDF path is empty";
        return false;
    }
    
    // Validate regions
    for (const GroundTruthRegion& region : annotation.expectedRegions) {
        if (region.name.isEmpty()) {
            qWarning() << "Region name is empty";
            return false;
        }
        
        if (!CoordinateSystem::isValidNormalized(region.normalizedCoords)) {
            qWarning() << "Invalid normalized coordinates for region:" << region.name;
            return false;
        }
    }
    
    return true;
}

GroundTruthAnnotation TestDataManager::createTemplate(const QString& formId, const QString& imagePath,
                                                     const QString& documentType) const
{
    GroundTruthAnnotation annotation;
    annotation.formId = formId;
    annotation.imagePath = imagePath;
    annotation.pdfPath = imagePath;
    annotation.documentType = documentType;
    annotation.expectedRegions = QList<GroundTruthRegion>();
    
    QMap<QString, QVariant> metadata;
    metadata["totalFields"] = 0;
    metadata["hasCheckboxes"] = false;
    metadata["hasGrids"] = false;
    annotation.metadata = metadata;
    
    return annotation;
}

void TestDataManager::saveGroundTruth(const GroundTruthAnnotation& annotation, const QString& filePath) const
{
    QJsonObject root;
    root["formId"] = annotation.formId;
    root["imagePath"] = annotation.imagePath;
    root["pdfPath"] = annotation.pdfPath;
    root["documentType"] = annotation.documentType;
    
    // Serialize regions
    QJsonArray regionsArray;
    for (const GroundTruthRegion& region : annotation.expectedRegions) {
        regionsArray.append(regionToJson(region));
    }
    root["expectedRegions"] = regionsArray;
    
    // Serialize metadata
    QJsonObject metadataObj;
    for (auto it = annotation.metadata.begin(); it != annotation.metadata.end(); ++it) {
        metadataObj[it.key()] = QJsonValue::fromVariant(it.value());
    }
    root["metadata"] = metadataObj;
    
    // Write to file
    QJsonDocument doc(root);
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    } else {
        qWarning() << "Failed to write ground truth to" << filePath;
    }
}

GroundTruthRegion TestDataManager::parseRegion(const QJsonObject& regionObj) const
{
    GroundTruthRegion region;
    
    region.name = regionObj["name"].toString();
    region.type = regionObj["type"].toString();
    region.group = regionObj["group"].toString();
    region.confidence = regionObj.contains("confidence") ? regionObj["confidence"].toDouble() : 1.0;
    
    // Parse normalized coordinates
    if (regionObj.contains("normalizedCoords") && regionObj["normalizedCoords"].isObject()) {
        QJsonObject coordsObj = regionObj["normalizedCoords"].toObject();
        region.normalizedCoords.x1 = coordsObj["x1"].toDouble();
        region.normalizedCoords.y1 = coordsObj["y1"].toDouble();
        region.normalizedCoords.x2 = coordsObj["x2"].toDouble();
        region.normalizedCoords.y2 = coordsObj["y2"].toDouble();
    }
    
    return region;
}

QJsonObject TestDataManager::regionToJson(const GroundTruthRegion& region) const
{
    QJsonObject obj;
    obj["name"] = region.name;
    obj["type"] = region.type;
    obj["group"] = region.group;
    obj["confidence"] = region.confidence;
    
    QJsonObject coordsObj;
    coordsObj["x1"] = region.normalizedCoords.x1;
    coordsObj["y1"] = region.normalizedCoords.y1;
    coordsObj["x2"] = region.normalizedCoords.x2;
    coordsObj["y2"] = region.normalizedCoords.y2;
    obj["normalizedCoords"] = coordsObj;
    
    return obj;
}

QString TestDataManager::getFormsDirectory() const
{
    QDir baseDir(baseDirectory);
    return baseDir.absoluteFilePath("forms");
}

QString TestDataManager::getGroundTruthDirectory() const
{
    QDir baseDir(baseDirectory);
    return baseDir.absoluteFilePath("ground_truth");
}

} // namespace ocr_orc

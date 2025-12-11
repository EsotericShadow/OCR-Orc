#include "GroupInferencer.h"
#include "RegionDetector.h"
#include "SpatialClusterer.h"
#include "patterns/PostalCodePatternDetector.h"
#include "patterns/NameFieldPatternDetector.h"
#include "patterns/NumberSequencePatternDetector.h"
#include <cmath>
#if OCR_ORC_DEBUG_ENABLED
#include <QtCore/QDebug>
#endif

namespace ocr_orc {

GroupInferencer::GroupInferencer() {
}

bool GroupInferencer::areTypesConsistent(const QList<DetectedRegion>& regions) {
    if (regions.isEmpty()) {
        return true;
    }
    
    QString firstType = regions[0].inferredType;
    if (firstType == "unknown") {
        return true; // Unknown types are considered consistent
    }
    
    for (const DetectedRegion& region : regions) {
        if (region.inferredType != "unknown" && region.inferredType != firstType) {
            return false; // Type mismatch
        }
    }
    
    return true;
}

bool GroupInferencer::areSizesConsistent(const QList<DetectedRegion>& regions, double tolerance) {
    if (regions.size() < 2) {
        return true;
    }
    
    // Calculate average size
    double avgWidth = 0.0;
    double avgHeight = 0.0;
    for (const auto& region : regions) {
        avgWidth += (region.coords.x2 - region.coords.x1);
        avgHeight += (region.coords.y2 - region.coords.y1);
    }
    avgWidth /= regions.size();
    avgHeight /= regions.size();
    
    // Check if all regions are within tolerance
    for (const auto& region : regions) {
        double width = region.coords.x2 - region.coords.x1;
        double height = region.coords.y2 - region.coords.y1;
        
        double widthDev = std::abs(width - avgWidth) / avgWidth;
        double heightDev = std::abs(height - avgHeight) / avgHeight;
        
        if (widthDev > tolerance || heightDev > tolerance) {
            return false;
        }
    }
    
    return true;
}

QList<DetectedGroup> GroupInferencer::inferGroupsFromSpatial(const QList<DetectedRegion>& regions) {
    QList<DetectedGroup> groups;
    
    if (regions.isEmpty()) {
        return groups;
    }
    
    // Cluster by horizontal alignment (rows)
    QList<QList<DetectedRegion>> rows = clusterer.clusterByHorizontalAlignment(regions, 0.01);
    
    int groupIndex = 0;
    for (const auto& row : rows) {
        if (row.size() < 2) {
            continue; // Need at least 2 regions for a group
        }
        
        // Check type and size consistency
        if (areTypesConsistent(row) && areSizesConsistent(row, 0.3)) {
            // Create group
            DetectedGroup group;
            group.name = QString("Group_%1").arg(groupIndex + 1);
            
            // Generate region names
            for (int i = 0; i < row.size(); ++i) {
                group.regionNames.append(QString("region_%1_%2").arg(groupIndex + 1).arg(i + 1));
            }
            
            // Suggest color based on inferred type
            if (!row.isEmpty()) {
                QString type = row[0].inferredType;
                if (type == "letters") {
                    group.suggestedColor = groupIndex % 2 == 0 ? "blue" : "red";
                } else if (type == "numbers") {
                    group.suggestedColor = "green";
                } else {
                    group.suggestedColor = "yellow";
                }
            } else {
                group.suggestedColor = "blue"; // Default
            }
            
            group.confidence = 0.7; // Medium confidence for spatial grouping
            
            groups.append(group);
            groupIndex++;
        }
    }
    
    #if OCR_ORC_DEBUG_ENABLED
    qDebug() << "[GroupInferencer] Spatial grouping: Found" << groups.size() << "groups";
    #endif
    
    return groups;
}

QList<DetectedGroup> GroupInferencer::inferGroupsFromPatterns(const QList<DetectedRegion>& regions) {
    QList<DetectedGroup> groups;
    
    if (regions.isEmpty()) {
        return groups;
    }
    
    // Detect postal code patterns
    PostalCodePattern postalPattern = postalCodeDetector.detectPattern(regions);
    if (postalCodeDetector.validatePattern(postalPattern)) {
        DetectedGroup postalGroup;
        postalGroup.name = "Postalcode";
        postalGroup.suggestedColor = "yellow";
        postalGroup.confidence = postalPattern.confidence;
        
        // Add regions from both groups
        for (int i = 0; i < postalPattern.group1.size(); ++i) {
            postalGroup.regionNames.append(QString("Postal_code_cell_%1").arg(i + 1));
        }
        for (int i = 0; i < postalPattern.group2.size(); ++i) {
            postalGroup.regionNames.append(QString("Postal_code_cell_%1").arg(i + 4));
        }
        
        groups.append(postalGroup);
    }
    
    // Detect name field patterns
    QList<QList<DetectedRegion>> nameFields = nameFieldDetector.detectNameFields(regions);
    for (int i = 0; i < nameFields.size(); ++i) {
        const auto& nameField = nameFields[i];
        if (!nameField.isEmpty()) {
            DetectedGroup nameGroup;
            nameGroup.name = nameField[0].suggestedGroup;
            nameGroup.suggestedColor = nameField[0].suggestedColor;
            nameGroup.confidence = 0.9; // High confidence for pattern-based detection
            
            for (int j = 0; j < nameField.size(); ++j) {
                nameGroup.regionNames.append(QString("%1_cell_%2")
                    .arg(nameGroup.name).arg(j + 1));
            }
            
            groups.append(nameGroup);
        }
    }
    
    // Detect number sequence patterns
    QList<QList<DetectedRegion>> numberSequences = numberSequenceDetector.detectNumberSequences(regions);
    for (int i = 0; i < numberSequences.size(); ++i) {
        const auto& numberSequence = numberSequences[i];
        if (!numberSequence.isEmpty()) {
            DetectedGroup numberGroup;
            numberGroup.name = numberSequence[0].suggestedGroup;
            numberGroup.suggestedColor = numberSequence[0].suggestedColor;
            numberGroup.confidence = 0.9; // High confidence for pattern-based detection
            
            for (int j = 0; j < numberSequence.size(); ++j) {
                numberGroup.regionNames.append(QString("%1_cell_%2")
                    .arg(numberGroup.name).arg(j + 1));
            }
            
            groups.append(numberGroup);
        }
    }
    
    #if OCR_ORC_DEBUG_ENABLED
    qDebug() << "[GroupInferencer] Pattern-based grouping: Found" << groups.size() << "groups";
    #endif
    
    return groups;
}

QString GroupInferencer::suggestColorForGroup(const QString& groupName, const QString& patternType) {
    // Use pattern type if provided
    if (patternType == "postal_code") {
        return "yellow";
    } else if (patternType == "name_field") {
        // Alternate between red and blue
        if (groupName.contains("Firstname", Qt::CaseInsensitive)) {
            return "red";
        } else if (groupName.contains("Lastname", Qt::CaseInsensitive)) {
            return "blue";
        }
        return "red"; // Default for name fields
    } else if (patternType == "number_sequence") {
        return "green";
    }
    
    // Infer from group name
    if (groupName.contains("Postal", Qt::CaseInsensitive) || 
        groupName.contains("Post", Qt::CaseInsensitive)) {
        return "yellow";
    } else if (groupName.contains("First", Qt::CaseInsensitive)) {
        return "red";
    } else if (groupName.contains("Last", Qt::CaseInsensitive)) {
        return "blue";
    } else if (groupName.contains("Student", Qt::CaseInsensitive) ||
               groupName.contains("Number", Qt::CaseInsensitive)) {
        return "green";
    }
    
    return "blue"; // Default color
}

} // namespace ocr_orc

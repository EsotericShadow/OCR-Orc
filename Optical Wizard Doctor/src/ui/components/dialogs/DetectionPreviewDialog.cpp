#include "DetectionPreviewDialog.h"
#include "../../../utils/RegionDetector.h"
#include "../../../core/CoordinateSystem.h"
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QListWidgetItem>
#include <QtWidgets/QAbstractItemView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtGui/QPixmap>
#include <QtGui/QPainter>
#include <QtGui/QColor>
#include <QtGui/QFont>
#include <QtCore/QDebug>

namespace ocr_orc {

DetectionPreviewDialog::DetectionPreviewDialog(QWidget* parent,
                                                 const DetectionResult& result,
                                                 const QImage& sourceImage)
    : QDialog(parent)
    , detectionResult(result)
    , sourceImage(sourceImage)
    , regionListWidget(nullptr)
    , previewLabel(nullptr)
    , acceptAllButton(nullptr)
    , rejectAllButton(nullptr)
    , acceptFilteredButton(nullptr)
    , acceptButton(nullptr)
    , cancelButton(nullptr)
    , confidenceFilterSlider(nullptr)
    , confidenceLabel(nullptr)
    , currentConfidenceFilter(0.0)
    , groupsLabel(nullptr)
    , patternsLabel(nullptr)
    , groupsListWidget(nullptr)
{
    setWindowTitle("Magic Detect - Review Detected Regions");
    setMinimumSize(900, 700);
    resize(1000, 750);
    
    // Initialize accepted indices (start with all rejected)
    acceptedIndices.clear();
    currentConfidenceFilter = 0.0;
    
    setupUI();
    updateRegionList();
    updatePreview();
}

void DetectionPreviewDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    
    // Top section: Preview image and region list side by side
    QHBoxLayout* contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(10);
    
    // Left side: Preview image with overlays
    QVBoxLayout* previewLayout = new QVBoxLayout();
    QLabel* previewTitle = new QLabel("Preview:", this);
    previewTitle->setStyleSheet("font-weight: bold; font-size: 12pt;");
    previewLayout->addWidget(previewTitle);
    
    previewLabel = new QLabel(this);
    previewLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    previewLabel->setAlignment(Qt::AlignCenter);
    previewLabel->setStyleSheet("border: 1px solid #ccc; background-color: #f0f0f0;");
    previewLabel->setMinimumSize(500, 400);
    previewLayout->addWidget(previewLabel);
    
    // Patterns detected label
    patternsLabel = new QLabel("Patterns: None detected", this);
    patternsLabel->setStyleSheet("font-style: italic; color: #666; padding: 5px;");
    previewLayout->addWidget(patternsLabel);
    
    contentLayout->addLayout(previewLayout, 2); // 2/3 width
    
    // Right side: Region list and controls
    QVBoxLayout* rightLayout = new QVBoxLayout();
    
    // Confidence filter
    QLabel* filterTitle = new QLabel("Filter by Confidence:", this);
    filterTitle->setStyleSheet("font-weight: bold;");
    rightLayout->addWidget(filterTitle);
    
    QHBoxLayout* filterLayout = new QHBoxLayout();
    confidenceFilterSlider = new QSlider(Qt::Horizontal, this);
    confidenceFilterSlider->setMinimum(0);
    confidenceFilterSlider->setMaximum(100);
    confidenceFilterSlider->setValue(0);
    confidenceFilterSlider->setToolTip("Filter regions by minimum confidence (0-100%)");
    filterLayout->addWidget(confidenceFilterSlider);
    
    confidenceLabel = new QLabel("0%", this);
    confidenceLabel->setMinimumWidth(40);
    filterLayout->addWidget(confidenceLabel);
    rightLayout->addLayout(filterLayout);
    
    connect(confidenceFilterSlider, &QSlider::valueChanged, this, &DetectionPreviewDialog::onConfidenceFilterChanged);
    
    // Inferred groups section
    QLabel* groupsTitle = new QLabel("Inferred Groups:", this);
    groupsTitle->setStyleSheet("font-weight: bold;");
    rightLayout->addWidget(groupsTitle);
    
    groupsListWidget = new QListWidget(this);
    groupsListWidget->setMaximumHeight(100);
    groupsListWidget->setToolTip("Groups automatically inferred from patterns");
    rightLayout->addWidget(groupsListWidget);
    
    groupsLabel = new QLabel("", this);
    groupsLabel->setStyleSheet("font-style: italic; color: #666; font-size: 9pt;");
    rightLayout->addWidget(groupsLabel);
    
    // Region list
    QLabel* listTitle = new QLabel("Detected Regions:", this);
    listTitle->setStyleSheet("font-weight: bold;");
    rightLayout->addWidget(listTitle);
    
    regionListWidget = new QListWidget(this);
    regionListWidget->setMinimumWidth(300);
    regionListWidget->setMaximumWidth(400);
    regionListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    rightLayout->addWidget(regionListWidget, 1);
    
    connect(regionListWidget, &QListWidget::itemClicked, this, &DetectionPreviewDialog::onRegionItemClicked);
    
    // Individual region actions
    QHBoxLayout* regionActionLayout = new QHBoxLayout();
    QPushButton* acceptRegionButton = new QPushButton("Accept Selected", this);
    QPushButton* rejectRegionButton = new QPushButton("Reject Selected", this);
    connect(acceptRegionButton, &QPushButton::clicked, this, &DetectionPreviewDialog::onRegionAccepted);
    connect(rejectRegionButton, &QPushButton::clicked, this, &DetectionPreviewDialog::onRegionRejected);
    regionActionLayout->addWidget(acceptRegionButton);
    regionActionLayout->addWidget(rejectRegionButton);
    rightLayout->addLayout(regionActionLayout);
    
    contentLayout->addLayout(rightLayout, 1); // 1/3 width
    mainLayout->addLayout(contentLayout, 1);
    
    // Bottom section: Batch action buttons
    QHBoxLayout* batchLayout = new QHBoxLayout();
    batchLayout->setSpacing(10);
    
    acceptAllButton = new QPushButton("Accept All", this);
    rejectAllButton = new QPushButton("Reject All", this);
    acceptFilteredButton = new QPushButton("Accept High-Confidence", this);
    
    acceptAllButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; padding: 8px; }");
    rejectAllButton->setStyleSheet("QPushButton { background-color: #f44336; color: white; padding: 8px; }");
    acceptFilteredButton->setStyleSheet("QPushButton { background-color: #2196F3; color: white; padding: 8px; }");
    
    connect(acceptAllButton, &QPushButton::clicked, this, &DetectionPreviewDialog::onAcceptAllClicked);
    connect(rejectAllButton, &QPushButton::clicked, this, &DetectionPreviewDialog::onRejectAllClicked);
    connect(acceptFilteredButton, &QPushButton::clicked, this, &DetectionPreviewDialog::onAcceptFilteredClicked);
    
    batchLayout->addWidget(acceptAllButton);
    batchLayout->addWidget(rejectAllButton);
    batchLayout->addWidget(acceptFilteredButton);
    batchLayout->addStretch();
    
    // Dialog buttons
    acceptButton = new QPushButton("Accept", this);
    cancelButton = new QPushButton("Cancel", this);
    acceptButton->setDefault(true);
    acceptButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; padding: 8px 20px; font-weight: bold; }");
    cancelButton->setStyleSheet("QPushButton { padding: 8px 20px; }");
    
    connect(acceptButton, &QPushButton::clicked, this, &DetectionPreviewDialog::onAcceptButtonClicked);
    connect(cancelButton, &QPushButton::clicked, this, &DetectionPreviewDialog::onCancelButtonClicked);
    
    batchLayout->addWidget(acceptButton);
    batchLayout->addWidget(cancelButton);
    mainLayout->addLayout(batchLayout);
    
    // Summary label
    QLabel* summaryLabel = new QLabel(
        QString("Total: %1 regions | High: %2 | Medium: %3 | Low: %4")
            .arg(detectionResult.totalDetected)
            .arg(detectionResult.highConfidence)
            .arg(detectionResult.mediumConfidence)
            .arg(detectionResult.lowConfidence),
        this
    );
    summaryLabel->setStyleSheet("font-weight: bold; padding: 5px;");
    mainLayout->addWidget(summaryLabel);
}

void DetectionPreviewDialog::updatePreview() {
    if (sourceImage.isNull()) {
        previewLabel->setText("No image available");
        return;
    }
    
    // Create pixmap from source image
    QPixmap pixmap = QPixmap::fromImage(sourceImage);
    
    // Scale to fit preview label while maintaining aspect ratio
    QSize labelSize = previewLabel->size();
    if (labelSize.width() < 100) labelSize.setWidth(500);
    if (labelSize.height() < 100) labelSize.setHeight(400);
    
    pixmap = pixmap.scaled(labelSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    
    // Create painter to draw overlays
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Calculate scale factor (pixmap may be scaled down)
    double scaleX = (double)pixmap.width() / sourceImage.width();
    double scaleY = (double)pixmap.height() / sourceImage.height();
    
    // Draw each detected region
    for (int i = 0; i < detectionResult.regions.size(); ++i) {
        const DetectedRegion& region = detectionResult.regions[i];
        
        // Check if filtered out
        if (region.confidence < currentConfidenceFilter) {
            continue;
        }
        
        // Convert normalized coordinates to pixmap coordinates
        int x1 = (int)(region.coords.x1 * pixmap.width());
        int y1 = (int)(region.coords.y1 * pixmap.height());
        int x2 = (int)(region.coords.x2 * pixmap.width());
        int y2 = (int)(region.coords.y2 * pixmap.height());
        
        // Determine color based on confidence
        QColor regionColor;
        if (region.confidence >= 0.8) {
            regionColor = QColor(76, 175, 80, 128); // Green, 50% opacity
        } else if (region.confidence >= 0.5) {
            regionColor = QColor(255, 193, 7, 128); // Yellow, 50% opacity
        } else {
            regionColor = QColor(244, 67, 54, 128); // Red, 50% opacity
        }
        
        // Draw rectangle
        painter.setPen(QPen(regionColor, 2));
        painter.setBrush(QBrush(regionColor));
        painter.drawRect(x1, y1, x2 - x1, y2 - y1);
        
        // Draw region number
        painter.setPen(Qt::black);
        painter.setFont(QFont("Arial", 10, QFont::Bold));
        QString label = QString("%1").arg(i + 1);
        painter.drawText(x1 + 3, y1 + 15, label);
    }
    
    painter.end();
    
    // Set pixmap to label
    previewLabel->setPixmap(pixmap);
}

void DetectionPreviewDialog::updateRegionList() {
    if (!regionListWidget) return;
    
    regionListWidget->clear();
    
    for (int i = 0; i < detectionResult.regions.size(); ++i) {
        const DetectedRegion& region = detectionResult.regions[i];
        
        // Skip if filtered out
        if (region.confidence < currentConfidenceFilter) {
            continue;
        }
        
        QListWidgetItem* item = new QListWidgetItem();
        QString text = QString("Region %1 - Confidence: %2% - Method: %3")
                          .arg(i + 1)
                          .arg(region.confidence * 100, 0, 'f', 1)
                          .arg(region.method);
        
        // Add OCR-first specific information
        if (region.method == "ocr-first") {
            QString typeInfo = region.inferredType != "unknown" ? 
                QString(" | Type: %1").arg(region.inferredType) : "";
            QString groupInfo = !region.suggestedGroup.isEmpty() ? 
                QString(" | Group: %1").arg(region.suggestedGroup) : "";
            QString colorInfo = !region.suggestedColor.isEmpty() ? 
                QString(" | Color: %1").arg(region.suggestedColor) : "";
            text += typeInfo + groupInfo + colorInfo;
        }
        
        item->setText(text);
        
        // Set background color based on confidence
        QColor bgColor;
        if (region.confidence >= 0.8) {
            bgColor = QColor(200, 255, 200); // Light green
        } else if (region.confidence >= 0.5) {
            bgColor = QColor(255, 255, 200); // Light yellow
        } else {
            bgColor = QColor(255, 200, 200); // Light red
        }
        item->setBackground(bgColor);
        
        // Set check state based on acceptance
        item->setCheckState(acceptedIndices.contains(i) ? Qt::Checked : Qt::Unchecked);
        
        // Store region index in item data
        item->setData(Qt::UserRole, i);
        
        regionListWidget->addItem(item);
    }
    
    // Update patterns label
    if (patternsLabel) {
        QString patternText = "Patterns: ";
        if (detectionResult.methodUsed == "ocr-first") {
            // Check for checkbox pattern from region data
            bool hasCheckboxLeft = false;
            bool hasCheckboxRight = false;
            for (const DetectedRegion& region : detectionResult.regions) {
                if (region.suggestedGroup == "CheckboxGroup") {
                    // Could enhance to detect left/right from checkbox detection
                    hasCheckboxLeft = true;  // Simplified
                }
            }
            if (hasCheckboxLeft || hasCheckboxRight) {
                patternText += "Checkbox pattern detected";
            } else {
                patternText += "Text fields detected";
            }
        } else {
            patternText += "None detected";
        }
        patternsLabel->setText(patternText);
    }
    
    // Update groups list widget
    if (groupsListWidget) {
        groupsListWidget->clear();
        for (const DetectedGroup& group : detectionResult.inferredGroups) {
            QString groupText = QString("%1 (%2 regions) - Color: %3")
                                  .arg(group.name)
                                  .arg(group.regionNames.size())
                                  .arg(group.suggestedColor);
            groupsListWidget->addItem(groupText);
        }
        
        if (groupsListWidget->count() == 0) {
            groupsListWidget->addItem("No groups detected");
        }
    }
    
    // Update groups label
    if (groupsLabel) {
        if (detectionResult.inferredGroups.isEmpty()) {
            groupsLabel->setText("");
        } else {
            groupsLabel->setText(QString("Found %1 group(s)").arg(detectionResult.inferredGroups.size()));
        }
    }
}

void DetectionPreviewDialog::onConfidenceFilterChanged(int value) {
    currentConfidenceFilter = value / 100.0;
    confidenceLabel->setText(QString("%1%").arg(value));
    filterRegionsByConfidence(currentConfidenceFilter);
}

void DetectionPreviewDialog::filterRegionsByConfidence(double minConfidence) {
    currentConfidenceFilter = minConfidence;
    updateRegionList();
    updatePreview();
}

void DetectionPreviewDialog::onRegionItemClicked(QListWidgetItem* item) {
    // Highlight region in preview (could be enhanced)
    updatePreview();
}

void DetectionPreviewDialog::onRegionAccepted() {
    QListWidgetItem* item = regionListWidget->currentItem();
    if (!item) return;
    
    int index = item->data(Qt::UserRole).toInt();
    acceptedIndices.insert(index);
    item->setCheckState(Qt::Checked);
    updatePreview();
}

void DetectionPreviewDialog::onRegionRejected() {
    QListWidgetItem* item = regionListWidget->currentItem();
    if (!item) return;
    
    int index = item->data(Qt::UserRole).toInt();
    acceptedIndices.remove(index);
    item->setCheckState(Qt::Unchecked);
    updatePreview();
}

void DetectionPreviewDialog::onAcceptAllClicked() {
    acceptedIndices.clear();
    for (int i = 0; i < detectionResult.regions.size(); ++i) {
        if (detectionResult.regions[i].confidence >= currentConfidenceFilter) {
            acceptedIndices.insert(i);
        }
    }
    updateRegionList();
    updatePreview();
}

void DetectionPreviewDialog::onRejectAllClicked() {
    acceptedIndices.clear();
    updateRegionList();
    updatePreview();
}

void DetectionPreviewDialog::onAcceptFilteredClicked() {
    acceptedIndices.clear();
    for (int i = 0; i < detectionResult.regions.size(); ++i) {
        if (detectionResult.regions[i].confidence >= 0.8) {
            acceptedIndices.insert(i);
        }
    }
    updateRegionList();
    updatePreview();
}

void DetectionPreviewDialog::onAcceptButtonClicked() {
    QList<DetectedRegion> accepted = getAcceptedRegions();
    emit regionsAccepted(accepted);
    accept();
}

void DetectionPreviewDialog::onCancelButtonClicked() {
    reject();
}

QList<DetectedRegion> DetectionPreviewDialog::getAcceptedRegions() const {
    QList<DetectedRegion> accepted;
    for (int index : acceptedIndices) {
        if (index >= 0 && index < detectionResult.regions.size()) {
            accepted.append(detectionResult.regions[index]);
        }
    }
    return accepted;
}

} // namespace ocr_orc

#ifndef DETECTION_PREVIEW_DIALOG_H
#define DETECTION_PREVIEW_DIALOG_H

#include <QtWidgets/QDialog>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QListWidgetItem>
#include <QtWidgets/QAbstractItemView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtCore/QList>
#include <QtCore/QSet>
#include <QtGui/QImage>
#include "../../../utils/RegionDetector.h"

namespace ocr_orc {

/**
 * @brief Dialog for previewing and accepting/rejecting auto-detected regions
 * 
 * Shows detected regions as overlays on the source image and allows users to:
 * - Accept/reject individual regions
 * - Accept/reject all regions
 * - Filter by confidence level
 * - See region details (confidence, method)
 */
class DetectionPreviewDialog : public QDialog {
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent widget
     * @param result DetectionResult with all detected regions
     * @param sourceImage Original image for preview
     */
    explicit DetectionPreviewDialog(QWidget* parent, 
                                    const DetectionResult& result, 
                                    const QImage& sourceImage);
    ~DetectionPreviewDialog() = default;
    
    /**
     * @brief Get list of accepted regions
     * @return QList of DetectedRegion that were accepted by user
     */
    QList<DetectedRegion> getAcceptedRegions() const;
    
    /**
     * @brief Get the full detection result (for accessing groups/patterns)
     * @return Full DetectionResult with groups and patterns
     */
    DetectionResult getDetectionResult() const { return detectionResult; }

signals:
    /**
     * @brief Emitted when user clicks Accept button
     * @param regions List of accepted DetectedRegion objects
     */
    void regionsAccepted(const QList<DetectedRegion>& regions);

private slots:
    void onAcceptAllClicked();
    void onRejectAllClicked();
    void onAcceptFilteredClicked();
    void onRegionItemClicked(QListWidgetItem* item);
    void onRegionAccepted();
    void onRegionRejected();
    void onConfidenceFilterChanged(int value);
    void onAcceptButtonClicked();
    void onCancelButtonClicked();

private:
    void setupUI();
    void updatePreview();
    void updateRegionList();
    void filterRegionsByConfidence(double minConfidence);
    
    DetectionResult detectionResult;
    QImage sourceImage;
    QListWidget* regionListWidget;
    QLabel* previewLabel;
    QPushButton* acceptAllButton;
    QPushButton* rejectAllButton;
    QPushButton* acceptFilteredButton;
    QPushButton* acceptButton;
    QPushButton* cancelButton;
    QSlider* confidenceFilterSlider;
    QLabel* confidenceLabel;
    QSet<int> acceptedIndices;
    double currentConfidenceFilter;
    QLabel* groupsLabel;
    QLabel* patternsLabel;
    QListWidget* groupsListWidget;
};

} // namespace ocr_orc

#endif // DETECTION_PREVIEW_DIALOG_H

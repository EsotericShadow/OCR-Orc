#ifndef MAGIC_DETECT_PARAMS_DIALOG_H
#define MAGIC_DETECT_PARAMS_DIALOG_H

#include <QtWidgets/QDialog>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QSlider>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QCheckBox>
#include <QtCore/QSettings>

namespace ocr_orc {

/**
 * @brief Parameters structure for Magic Detect configuration
 */
struct DetectionParameters {
    // OCR Overlap Threshold (0.0-1.0, default 0.20 = 20% - Very Lenient)
    double ocrOverlapThreshold = 0.20;
    
    // Brightness Thresholds (Very Lenient defaults)
    double baseBrightnessThreshold = 0.50;  // Lower threshold (50%)
    double brightnessAdaptiveFactor = 0.70;  // 70% of local brightness (more lenient)
    
    // Edge Density Thresholds (Very Lenient defaults - allows borders/edges)
    double edgeDensityThreshold = 0.15;  // Higher threshold (allows more edges)
    double horizontalEdgeDensityThreshold = 0.25;  // Much higher (allows horizontal borders)
    double verticalEdgeDensityThreshold = 0.15;  // Higher (allows vertical borders)
    
    // IoU Threshold for consensus matching (0.0-1.0, default 0.40 = 40% - Lenient)
    double iouThreshold = 0.40;
    
    // OCR Confidence Threshold (0-100, default 40.0 - Lower, includes more text)
    double ocrConfidenceThreshold = 40.0;
    
    // Overfitting Percentages (0-100, default 50%/70% - More expansion)
    double horizontalOverfitPercent = 50.0;
    double verticalOverfitPercent = 70.0;
    
    // Hough Line Detection (default 4 - Need more lines to reject)
    int minHorizontalLines = 4;  // Minimum lines to consider as text
    
    // Consensus Mode (default false = lenient)
    bool strictConsensus = false;  // false = lenient, true = strict
    
    // Checkbox Detection Parameters
    int minCheckboxSize = 10;          // Minimum checkbox size in pixels (default: 10)
    int maxCheckboxSize = 60;          // Maximum checkbox size in pixels (default: 60)
    double checkboxAspectRatioMin = 0.6;  // Minimum aspect ratio (width/height) for square detection
    double checkboxAspectRatioMax = 1.6;  // Maximum aspect ratio (width/height) for square detection
    double checkboxRectangularity = 0.5;   // Minimum rectangularity (0.0-1.0) - how close to perfect rectangle
    bool enableStandaloneCheckboxDetection = true;  // Enable scanning entire image for checkboxes
    
    // Reset to defaults (Very Lenient configuration)
    void resetToDefaults() {
        *this = DetectionParameters();
    }
};

/**
 * @brief Dialog for configuring Magic Detect parameters
 * 
 * Allows users to adjust all detection thresholds and parameters
 * before running Magic Detect, enabling experimentation and tuning.
 */
class MagicDetectParamsDialog : public QDialog {
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent widget
     * @param currentParams Current parameters (for loading defaults)
     */
    explicit MagicDetectParamsDialog(QWidget* parent, 
                                     const DetectionParameters& currentParams = DetectionParameters());
    virtual ~MagicDetectParamsDialog() = default;
    
    /**
     * @brief Get the configured parameters
     * @return DetectionParameters with user's settings
     */
    DetectionParameters getParameters() const;
    
    /**
     * @brief Check if user clicked "Run" (true) or "Cancel" (false)
     * @return True if Run was clicked
     */
    bool shouldRun() const { return shouldRunDetection; }

private slots:
    void onResetClicked();
    void onRunClicked();
    void onCancelClicked();
    void updatePreviewLabels();

private:
    void setupUI();
    void loadSettings();
    void saveSettings();
    void connectSignals();
    void applyTheme();
    
    DetectionParameters params;
    bool shouldRunDetection;
    
    // UI Components - OCR Overlap
    QDoubleSpinBox* ocrOverlapSpinBox;
    QLabel* ocrOverlapPreview;
    
    // UI Components - Brightness
    QDoubleSpinBox* baseBrightnessSpinBox;
    QDoubleSpinBox* brightnessAdaptiveSpinBox;
    QLabel* brightnessPreview;
    
    // UI Components - Edge Density
    QDoubleSpinBox* edgeDensitySpinBox;
    QDoubleSpinBox* horizontalEdgeDensitySpinBox;
    QDoubleSpinBox* verticalEdgeDensitySpinBox;
    QLabel* edgeDensityPreview;
    
    // UI Components - IoU
    QDoubleSpinBox* iouThresholdSpinBox;
    QLabel* iouPreview;
    
    // UI Components - OCR Confidence
    QDoubleSpinBox* ocrConfidenceSpinBox;
    QLabel* ocrConfidencePreview;
    
    // UI Components - Overfitting
    QDoubleSpinBox* horizontalOverfitSpinBox;
    QDoubleSpinBox* verticalOverfitSpinBox;
    QLabel* overfitPreview;
    
    // UI Components - Hough Lines
    QSpinBox* minHorizontalLinesSpinBox;
    QLabel* houghPreview;
    
    // UI Components - Consensus Mode
    QCheckBox* strictConsensusCheckBox;
    
    // UI Components - Checkbox Detection
    QSpinBox* minCheckboxSizeSpinBox;
    QSpinBox* maxCheckboxSizeSpinBox;
    QDoubleSpinBox* checkboxAspectRatioMinSpinBox;
    QDoubleSpinBox* checkboxAspectRatioMaxSpinBox;
    QDoubleSpinBox* checkboxRectangularitySpinBox;
    QCheckBox* enableStandaloneCheckboxCheckBox;
    QLabel* checkboxPreview;
    
    // Preset Buttons
    QPushButton* presetVeryLenientButton;
    QPushButton* presetBalancedButton;
    QPushButton* presetStrictButton;
    
    // Buttons
    QPushButton* resetButton;
    QPushButton* runButton;
    QPushButton* cancelButton;
    QPushButton* helpButton;
    
    // Helper methods
    void applyPreset(const QString& presetName);
    void showHelpDialog();
};

} // namespace ocr_orc

#endif // MAGIC_DETECT_PARAMS_DIALOG_H


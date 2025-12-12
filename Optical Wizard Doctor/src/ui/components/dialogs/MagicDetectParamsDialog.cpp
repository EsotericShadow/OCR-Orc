#include "MagicDetectParamsDialog.h"
#include "../../ThemeManager.h"
#include <QtCore/QSettings>
#include <QtCore/QDebug>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QTextEdit>

namespace ocr_orc {

MagicDetectParamsDialog::MagicDetectParamsDialog(QWidget* parent, const DetectionParameters& currentParams)
    : QDialog(parent)
    , params(currentParams)
    , shouldRunDetection(false)
    , ocrOverlapSpinBox(nullptr)
    , ocrOverlapPreview(nullptr)
    , baseBrightnessSpinBox(nullptr)
    , brightnessAdaptiveSpinBox(nullptr)
    , brightnessPreview(nullptr)
    , edgeDensitySpinBox(nullptr)
    , horizontalEdgeDensitySpinBox(nullptr)
    , verticalEdgeDensitySpinBox(nullptr)
    , edgeDensityPreview(nullptr)
    , iouThresholdSpinBox(nullptr)
    , iouPreview(nullptr)
    , ocrConfidenceSpinBox(nullptr)
    , ocrConfidencePreview(nullptr)
    , horizontalOverfitSpinBox(nullptr)
    , verticalOverfitSpinBox(nullptr)
    , overfitPreview(nullptr)
    , minHorizontalLinesSpinBox(nullptr)
    , houghPreview(nullptr)
    , strictConsensusCheckBox(nullptr)
    , minCheckboxSizeSpinBox(nullptr)
    , maxCheckboxSizeSpinBox(nullptr)
    , checkboxAspectRatioMinSpinBox(nullptr)
    , checkboxAspectRatioMaxSpinBox(nullptr)
    , checkboxRectangularitySpinBox(nullptr)
    , enableStandaloneCheckboxCheckBox(nullptr)
    , checkboxPreview(nullptr)
    , presetVeryLenientButton(nullptr)
    , presetBalancedButton(nullptr)
    , presetStrictButton(nullptr)
    , resetButton(nullptr)
    , runButton(nullptr)
    , cancelButton(nullptr)
    , helpButton(nullptr)
{
    setWindowTitle("🎯 Magic Detect - Configure Parameters");
    setMinimumSize(900, 700);
    resize(1000, 800);
    
    setupUI();
    connectSignals();
    loadSettings();
    updatePreviewLabels();
    
    // Apply theme
    applyTheme();
}

// Helper function to create styled card widgets with theme support
static QFrame* createStyledCard(QWidget* parent, const QString& title) {
    ThemeManager& themeManager = ThemeManager::instance();
    ThemeColors colors = themeManager.getColors();
    
    QFrame* card = new QFrame(parent);
    card->setFrameShape(QFrame::StyledPanel);
    card->setStyleSheet(
        QString(
            "QFrame {"
            "  background-color: %1;"
            "  border: 1px solid %2;"
            "  border-radius: 8px;"
            "}"
        ).arg(colors.surface.name(), colors.border.name())
    );
    card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    card->setMinimumHeight(100);
    
    QVBoxLayout* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(16, 16, 16, 16);
    cardLayout->setSpacing(10);
    
    if (!title.isEmpty()) {
        QLabel* titleLabel = new QLabel(title, card);
        titleLabel->setStyleSheet(
            QString("font-weight: bold; font-size: 11pt; color: %1; padding-bottom: 4px;")
                .arg(colors.text.name())
        );
        cardLayout->addWidget(titleLabel);
    }
    
    return card;
}

void MagicDetectParamsDialog::setupUI() {
    // === MAIN LAYOUT ===
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // === HEADER SECTION ===
    // Note: Theme colors will be applied in applyTheme()
    QLabel* titleLabel = new QLabel("🎯 Magic Detect Parameters", this);
    titleLabel->setObjectName("dialogTitle");
    mainLayout->addWidget(titleLabel);
    
    QLabel* descLabel = new QLabel(
        "Fine-tune detection sensitivity. Use presets for quick setup, or customize individual parameters below.",
        this);
    descLabel->setWordWrap(true);
    descLabel->setObjectName("dialogDescription");
    mainLayout->addWidget(descLabel);
    
    // === PRESET BUTTONS SECTION ===
    QHBoxLayout* presetLayout = new QHBoxLayout();
    presetLayout->setSpacing(12);
    presetLayout->setContentsMargins(0, 0, 0, 0);
    
    presetVeryLenientButton = new QPushButton("🔍 Very Lenient", this);
    presetVeryLenientButton->setObjectName("presetVeryLenient");
    presetVeryLenientButton->setToolTip("Catch everything - may include false positives");
    
    presetBalancedButton = new QPushButton("⚖️ Balanced", this);
    presetBalancedButton->setObjectName("presetBalanced");
    presetBalancedButton->setToolTip("Balanced detection - good for most forms");
    
    presetStrictButton = new QPushButton("🎯 Strict", this);
    presetStrictButton->setObjectName("presetStrict");
    presetStrictButton->setToolTip("Strict detection - fewer false positives, may miss some fields");
    
    presetLayout->addWidget(presetVeryLenientButton);
    presetLayout->addWidget(presetBalancedButton);
    presetLayout->addWidget(presetStrictButton);
    presetLayout->addStretch();
    
    mainLayout->addLayout(presetLayout);
    
    // === SCROLL AREA WITH GRID LAYOUT ===
    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setObjectName("dialogScrollArea");
    
    QWidget* scrollContent = new QWidget();
    scrollContent->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    
    QGridLayout* gridLayout = new QGridLayout(scrollContent);
    gridLayout->setSpacing(12);
    gridLayout->setContentsMargins(12, 12, 12, 12);
    gridLayout->setColumnStretch(0, 1);
    gridLayout->setColumnStretch(1, 1);
    gridLayout->setColumnStretch(2, 1);
    
    // === ROW 1: TEXT DETECTION ===
    
    // Card 1: Text Overlap Filter
    QFrame* ocrCard = createStyledCard(scrollContent, "📝 Text Overlap Filter");
    QVBoxLayout* ocrCardLayout = static_cast<QVBoxLayout*>(ocrCard->layout());
    QFormLayout* ocrForm = new QFormLayout();
    ocrForm->setSpacing(8);
    ocrForm->setContentsMargins(0, 0, 0, 0);
    
    ocrOverlapSpinBox = new QDoubleSpinBox(ocrCard);
    ocrOverlapSpinBox->setRange(0.0, 1.0);
    ocrOverlapSpinBox->setSingleStep(0.01);
    ocrOverlapSpinBox->setDecimals(2);
    ocrOverlapSpinBox->setValue(params.ocrOverlapThreshold);
    ocrOverlapSpinBox->setToolTip(
        "<b>Text Overlap Threshold</b><br/>"
        "Regions that overlap with detected OCR text by more than this percentage will be rejected as text regions.<br/>"
        "<b>Lower values (0.1-0.3):</b> More lenient - allows regions near text<br/>"
        "<b>Higher values (0.5-0.8):</b> More strict - rejects regions with any text overlap<br/>"
        "<b>Default:</b> 0.20 (20%)"
    );
    ocrOverlapSpinBox->setMinimumWidth(120);
    ocrOverlapSpinBox->setMinimumHeight(28);
    ocrForm->addRow("Overlap Threshold:", ocrOverlapSpinBox);
    
    ocrOverlapPreview = new QLabel("", ocrCard);
    ocrOverlapPreview->setObjectName("previewLabel");
    ocrOverlapPreview->setWordWrap(true);
    ocrForm->addRow("", ocrOverlapPreview);
    
    ocrCardLayout->addLayout(ocrForm);
    gridLayout->addWidget(ocrCard, 0, 0);
    
    // Card 2: OCR Confidence
    QFrame* ocrConfCard = createStyledCard(scrollContent, "📊 OCR Confidence");
    QVBoxLayout* ocrConfCardLayout = static_cast<QVBoxLayout*>(ocrConfCard->layout());
    QFormLayout* ocrConfForm = new QFormLayout();
    ocrConfForm->setSpacing(8);
    ocrConfForm->setContentsMargins(0, 0, 0, 0);
    
    ocrConfidenceSpinBox = new QDoubleSpinBox(ocrConfCard);
    ocrConfidenceSpinBox->setRange(0.0, 100.0);
    ocrConfidenceSpinBox->setSingleStep(1.0);
    ocrConfidenceSpinBox->setDecimals(1);
    ocrConfidenceSpinBox->setValue(params.ocrConfidenceThreshold);
    ocrConfidenceSpinBox->setToolTip(
        "<b>OCR Confidence Threshold</b><br/>"
        "Minimum confidence level (0-100) required for OCR text to be considered valid.<br/>"
        "<b>Lower values (20-40):</b> Includes more text, even with lower confidence<br/>"
        "<b>Higher values (60-90):</b> Only includes high-confidence text<br/>"
        "<b>Default:</b> 40.0"
    );
    ocrConfidenceSpinBox->setMinimumWidth(120);
    ocrConfidenceSpinBox->setMinimumHeight(28);
    ocrConfForm->addRow("Confidence (%):", ocrConfidenceSpinBox);
    
    ocrConfidencePreview = new QLabel("", ocrConfCard);
    ocrConfidencePreview->setObjectName("previewLabel");
    ocrConfidencePreview->setWordWrap(true);
    ocrConfForm->addRow("", ocrConfidencePreview);
    
    ocrConfCardLayout->addLayout(ocrConfForm);
    gridLayout->addWidget(ocrConfCard, 0, 1);
    
    // Card 3: Text Line Detection
    QFrame* houghCard = createStyledCard(scrollContent, "📏 Text Line Detection");
    QVBoxLayout* houghCardLayout = static_cast<QVBoxLayout*>(houghCard->layout());
    QFormLayout* houghForm = new QFormLayout();
    houghForm->setSpacing(8);
    houghForm->setContentsMargins(0, 0, 0, 0);
    
    minHorizontalLinesSpinBox = new QSpinBox(houghCard);
    minHorizontalLinesSpinBox->setRange(1, 10);
    minHorizontalLinesSpinBox->setValue(params.minHorizontalLines);
    minHorizontalLinesSpinBox->setToolTip(
        "<b>Minimum Horizontal Lines</b><br/>"
        "Regions with this many or more horizontal lines will be rejected as text regions.<br/>"
        "Used to filter out paragraphs and text blocks from form field detection.<br/>"
        "<b>Lower values (1-3):</b> More strict - rejects regions with few lines<br/>"
        "<b>Higher values (5-8):</b> More lenient - allows regions with more lines<br/>"
        "<b>Default:</b> 4"
    );
    minHorizontalLinesSpinBox->setMinimumWidth(120);
    minHorizontalLinesSpinBox->setMinimumHeight(28);
    houghForm->addRow("Min Lines:", minHorizontalLinesSpinBox);
    
    houghPreview = new QLabel("", houghCard);
    houghPreview->setObjectName("previewLabel");
    houghPreview->setWordWrap(true);
    houghForm->addRow("", houghPreview);
    
    houghCardLayout->addLayout(houghForm);
    gridLayout->addWidget(houghCard, 0, 2);
    
    // === ROW 2: IMAGE ANALYSIS ===
    
    // Card 4: Brightness Filter
    QFrame* brightnessCard = createStyledCard(scrollContent, "💡 Brightness Filter");
    QVBoxLayout* brightnessCardLayout = static_cast<QVBoxLayout*>(brightnessCard->layout());
    QFormLayout* brightnessForm = new QFormLayout();
    brightnessForm->setSpacing(8);
    brightnessForm->setContentsMargins(0, 0, 0, 0);
    
    baseBrightnessSpinBox = new QDoubleSpinBox(brightnessCard);
    baseBrightnessSpinBox->setRange(0.0, 1.0);
    baseBrightnessSpinBox->setSingleStep(0.01);
    baseBrightnessSpinBox->setDecimals(2);
    baseBrightnessSpinBox->setValue(params.baseBrightnessThreshold);
    baseBrightnessSpinBox->setToolTip("Base brightness threshold. Lower = more lenient.");
    baseBrightnessSpinBox->setMinimumWidth(120);
    baseBrightnessSpinBox->setMinimumHeight(28);
    brightnessForm->addRow("Base Threshold:", baseBrightnessSpinBox);
    
    brightnessAdaptiveSpinBox = new QDoubleSpinBox(brightnessCard);
    brightnessAdaptiveSpinBox->setRange(0.0, 1.0);
    brightnessAdaptiveSpinBox->setSingleStep(0.01);
    brightnessAdaptiveSpinBox->setDecimals(2);
    brightnessAdaptiveSpinBox->setValue(params.brightnessAdaptiveFactor);
    brightnessAdaptiveSpinBox->setToolTip("Adaptive factor: threshold = local_brightness × factor");
    brightnessAdaptiveSpinBox->setMinimumWidth(120);
    brightnessAdaptiveSpinBox->setMinimumHeight(28);
    brightnessForm->addRow("Adaptive Factor:", brightnessAdaptiveSpinBox);
    
    brightnessPreview = new QLabel("", brightnessCard);
    brightnessPreview->setObjectName("previewLabel");
    brightnessPreview->setWordWrap(true);
    brightnessForm->addRow("", brightnessPreview);
    
    brightnessCardLayout->addLayout(brightnessForm);
    gridLayout->addWidget(brightnessCard, 1, 0);
    
    // Card 5: Edge Density Filter (spans 2 columns)
    QFrame* edgeCard = createStyledCard(scrollContent, "🔲 Edge Density Filter");
    QVBoxLayout* edgeCardLayout = static_cast<QVBoxLayout*>(edgeCard->layout());
    QFormLayout* edgeForm = new QFormLayout();
    edgeForm->setSpacing(8);
    edgeForm->setContentsMargins(0, 0, 0, 0);
    
    edgeDensitySpinBox = new QDoubleSpinBox(edgeCard);
    edgeDensitySpinBox->setRange(0.0, 1.0);
    edgeDensitySpinBox->setSingleStep(0.01);
    edgeDensitySpinBox->setDecimals(3);
    edgeDensitySpinBox->setValue(params.edgeDensityThreshold);
    edgeDensitySpinBox->setToolTip("Total edge density threshold.");
    edgeDensitySpinBox->setMinimumWidth(120);
    edgeDensitySpinBox->setMinimumHeight(28);
    edgeForm->addRow("Total Edge Density:", edgeDensitySpinBox);
    
    horizontalEdgeDensitySpinBox = new QDoubleSpinBox(edgeCard);
    horizontalEdgeDensitySpinBox->setRange(0.0, 1.0);
    horizontalEdgeDensitySpinBox->setSingleStep(0.01);
    horizontalEdgeDensitySpinBox->setDecimals(3);
    horizontalEdgeDensitySpinBox->setValue(params.horizontalEdgeDensityThreshold);
    horizontalEdgeDensitySpinBox->setToolTip("Horizontal edge density (text lines).");
    horizontalEdgeDensitySpinBox->setMinimumWidth(120);
    horizontalEdgeDensitySpinBox->setMinimumHeight(28);
    edgeForm->addRow("Horizontal:", horizontalEdgeDensitySpinBox);
    
    verticalEdgeDensitySpinBox = new QDoubleSpinBox(edgeCard);
    verticalEdgeDensitySpinBox->setRange(0.0, 1.0);
    verticalEdgeDensitySpinBox->setSingleStep(0.01);
    verticalEdgeDensitySpinBox->setDecimals(3);
    verticalEdgeDensitySpinBox->setValue(params.verticalEdgeDensityThreshold);
    verticalEdgeDensitySpinBox->setToolTip("Vertical edge density (borders).");
    verticalEdgeDensitySpinBox->setMinimumWidth(120);
    verticalEdgeDensitySpinBox->setMinimumHeight(28);
    edgeForm->addRow("Vertical:", verticalEdgeDensitySpinBox);
    
    edgeDensityPreview = new QLabel("", edgeCard);
    edgeDensityPreview->setObjectName("previewLabel");
    edgeDensityPreview->setWordWrap(true);
    edgeForm->addRow("", edgeDensityPreview);
    
    edgeCardLayout->addLayout(edgeForm);
    gridLayout->addWidget(edgeCard, 1, 1, 1, 2);
    
    // === ROW 3: CHECKBOX DETECTION ===
    
    // Card 6: Checkbox Detection (spans full width)
    QFrame* checkboxCard = createStyledCard(scrollContent, "☑️ Checkbox Detection");
    QVBoxLayout* checkboxCardLayout = static_cast<QVBoxLayout*>(checkboxCard->layout());
    
    // Enable standalone detection
    enableStandaloneCheckboxCheckBox = new QCheckBox("Enable Standalone Checkbox Detection", checkboxCard);
    enableStandaloneCheckboxCheckBox->setChecked(params.enableStandaloneCheckboxDetection);
    enableStandaloneCheckboxCheckBox->setToolTip("Scan entire image for checkboxes, not just near text");
    checkboxCardLayout->addWidget(enableStandaloneCheckboxCheckBox);
    
    // Size range
    QHBoxLayout* sizeLayout = new QHBoxLayout();
    sizeLayout->setSpacing(8);
    QLabel* sizeLabel = new QLabel("Size Range (pixels):", checkboxCard);
    sizeLayout->addWidget(sizeLabel);
    minCheckboxSizeSpinBox = new QSpinBox(checkboxCard);
    minCheckboxSizeSpinBox->setRange(5, 50);
    minCheckboxSizeSpinBox->setValue(params.minCheckboxSize);
    minCheckboxSizeSpinBox->setToolTip("Minimum checkbox size");
    minCheckboxSizeSpinBox->setMinimumWidth(100);
    minCheckboxSizeSpinBox->setMinimumHeight(28);
    sizeLayout->addWidget(minCheckboxSizeSpinBox);
    sizeLayout->addWidget(new QLabel("to", checkboxCard));
    maxCheckboxSizeSpinBox = new QSpinBox(checkboxCard);
    maxCheckboxSizeSpinBox->setRange(20, 200);
    maxCheckboxSizeSpinBox->setValue(params.maxCheckboxSize);
    maxCheckboxSizeSpinBox->setToolTip("Maximum checkbox size");
    maxCheckboxSizeSpinBox->setMinimumWidth(100);
    maxCheckboxSizeSpinBox->setMinimumHeight(28);
    sizeLayout->addWidget(maxCheckboxSizeSpinBox);
    sizeLayout->addStretch();
    checkboxCardLayout->addLayout(sizeLayout);
    
    // Aspect ratio
    QHBoxLayout* aspectLayout = new QHBoxLayout();
    aspectLayout->setSpacing(8);
    QLabel* aspectLabel = new QLabel("Aspect Ratio:", checkboxCard);
    aspectLayout->addWidget(aspectLabel);
    checkboxAspectRatioMinSpinBox = new QDoubleSpinBox(checkboxCard);
    checkboxAspectRatioMinSpinBox->setRange(0.3, 1.0);
    checkboxAspectRatioMinSpinBox->setSingleStep(0.1);
    checkboxAspectRatioMinSpinBox->setDecimals(1);
    checkboxAspectRatioMinSpinBox->setValue(params.checkboxAspectRatioMin);
    checkboxAspectRatioMinSpinBox->setToolTip("Minimum width/height ratio (square = 1.0)");
    checkboxAspectRatioMinSpinBox->setMinimumWidth(100);
    checkboxAspectRatioMinSpinBox->setMinimumHeight(28);
    aspectLayout->addWidget(checkboxAspectRatioMinSpinBox);
    aspectLayout->addWidget(new QLabel("to", checkboxCard));
    checkboxAspectRatioMaxSpinBox = new QDoubleSpinBox(checkboxCard);
    checkboxAspectRatioMaxSpinBox->setRange(1.0, 2.0);
    checkboxAspectRatioMaxSpinBox->setSingleStep(0.1);
    checkboxAspectRatioMaxSpinBox->setDecimals(1);
    checkboxAspectRatioMaxSpinBox->setValue(params.checkboxAspectRatioMax);
    checkboxAspectRatioMaxSpinBox->setToolTip("Maximum width/height ratio");
    checkboxAspectRatioMaxSpinBox->setMinimumWidth(100);
    checkboxAspectRatioMaxSpinBox->setMinimumHeight(28);
    aspectLayout->addWidget(checkboxAspectRatioMaxSpinBox);
    aspectLayout->addStretch();
    checkboxCardLayout->addLayout(aspectLayout);
    
    // Rectangularity
    QHBoxLayout* rectLayout = new QHBoxLayout();
    rectLayout->setSpacing(8);
    QLabel* rectLabel = new QLabel("Rectangularity:", checkboxCard);
    rectLayout->addWidget(rectLabel);
    checkboxRectangularitySpinBox = new QDoubleSpinBox(checkboxCard);
    checkboxRectangularitySpinBox->setRange(0.0, 1.0);
    checkboxRectangularitySpinBox->setSingleStep(0.05);
    checkboxRectangularitySpinBox->setDecimals(2);
    checkboxRectangularitySpinBox->setValue(params.checkboxRectangularity);
    checkboxRectangularitySpinBox->setToolTip("How close to perfect rectangle (1.0 = perfect)");
    checkboxRectangularitySpinBox->setMinimumWidth(100);
    checkboxRectangularitySpinBox->setMinimumHeight(28);
    rectLayout->addWidget(checkboxRectangularitySpinBox);
    rectLayout->addStretch();
    checkboxCardLayout->addLayout(rectLayout);
    
    checkboxPreview = new QLabel("", checkboxCard);
    checkboxPreview->setObjectName("previewLabel");
    checkboxPreview->setWordWrap(true);
    checkboxCardLayout->addWidget(checkboxPreview);
    
    gridLayout->addWidget(checkboxCard, 2, 0, 1, 3);
    
    // === ROW 4: ADVANCED SETTINGS ===
    
    // Card 7: Consensus Matching
    QFrame* iouCard = createStyledCard(scrollContent, "🔗 Consensus Matching");
    QVBoxLayout* iouCardLayout = static_cast<QVBoxLayout*>(iouCard->layout());
    QFormLayout* iouForm = new QFormLayout();
    iouForm->setSpacing(8);
    iouForm->setContentsMargins(0, 0, 0, 0);
    
    iouThresholdSpinBox = new QDoubleSpinBox(iouCard);
    iouThresholdSpinBox->setRange(0.0, 1.0);
    iouThresholdSpinBox->setSingleStep(0.05);
    iouThresholdSpinBox->setDecimals(2);
    iouThresholdSpinBox->setValue(params.iouThreshold);
    iouThresholdSpinBox->setToolTip(
        "<b>IoU (Intersection over Union) Threshold</b><br/>"
        "Minimum overlap (0.0-1.0) required for two detections to be considered the same region.<br/>"
        "Used to merge detections from different methods (OCR and rectangle detection).<br/>"
        "<b>Lower values (0.2-0.4):</b> More lenient - merges regions with less overlap<br/>"
        "<b>Higher values (0.6-0.8):</b> More strict - requires more overlap to merge<br/>"
        "<b>Default:</b> 0.40"
    );
    iouThresholdSpinBox->setMinimumWidth(120);
    iouThresholdSpinBox->setMinimumHeight(28);
    iouForm->addRow("IoU Threshold:", iouThresholdSpinBox);
    
    iouPreview = new QLabel("", iouCard);
    iouPreview->setObjectName("previewLabel");
    iouPreview->setWordWrap(true);
    iouForm->addRow("", iouPreview);
    
    iouCardLayout->addLayout(iouForm);
    gridLayout->addWidget(iouCard, 3, 0);
    
    // Card 8: Region Expansion
    QFrame* overfitCard = createStyledCard(scrollContent, "📐 Region Expansion");
    QVBoxLayout* overfitCardLayout = static_cast<QVBoxLayout*>(overfitCard->layout());
    QFormLayout* overfitForm = new QFormLayout();
    overfitForm->setSpacing(8);
    overfitForm->setContentsMargins(0, 0, 0, 0);
    
    horizontalOverfitSpinBox = new QDoubleSpinBox(overfitCard);
    horizontalOverfitSpinBox->setRange(0.0, 100.0);
    horizontalOverfitSpinBox->setSingleStep(5.0);
    horizontalOverfitSpinBox->setDecimals(1);
    horizontalOverfitSpinBox->setValue(params.horizontalOverfitPercent);
    horizontalOverfitSpinBox->setToolTip(
        "<b>Horizontal Region Expansion</b><br/>"
        "Percentage (0-100) to expand detected regions horizontally.<br/>"
        "Useful for capturing text that extends slightly beyond the detected boundaries.<br/>"
        "<b>Lower values (0-30%):</b> Minimal expansion - tight boundaries<br/>"
        "<b>Higher values (50-100%):</b> More expansion - captures surrounding area<br/>"
        "<b>Default:</b> 50.0%"
    );
    horizontalOverfitSpinBox->setMinimumWidth(120);
    horizontalOverfitSpinBox->setMinimumHeight(28);
    overfitForm->addRow("Horizontal (%):", horizontalOverfitSpinBox);
    
    verticalOverfitSpinBox = new QDoubleSpinBox(overfitCard);
    verticalOverfitSpinBox->setRange(0.0, 100.0);
    verticalOverfitSpinBox->setSingleStep(5.0);
    verticalOverfitSpinBox->setDecimals(1);
    verticalOverfitSpinBox->setValue(params.verticalOverfitPercent);
    verticalOverfitSpinBox->setToolTip(
        "<b>Vertical Region Expansion</b><br/>"
        "Percentage (0-100) to expand detected regions vertically.<br/>"
        "Useful for capturing multi-line text fields or fields with labels above/below.<br/>"
        "<b>Lower values (0-40%):</b> Minimal expansion - tight boundaries<br/>"
        "<b>Higher values (60-100%):</b> More expansion - captures surrounding area<br/>"
        "<b>Default:</b> 70.0%"
    );
    verticalOverfitSpinBox->setMinimumWidth(120);
    verticalOverfitSpinBox->setMinimumHeight(28);
    overfitForm->addRow("Vertical (%):", verticalOverfitSpinBox);
    
    overfitPreview = new QLabel("", overfitCard);
    overfitPreview->setObjectName("previewLabel");
    overfitPreview->setWordWrap(true);
    overfitForm->addRow("", overfitPreview);
    
    overfitCardLayout->addLayout(overfitForm);
    gridLayout->addWidget(overfitCard, 3, 1);
    
    // Card 9: Advanced Options
    QFrame* consensusCard = createStyledCard(scrollContent, "⚙️ Advanced");
    QVBoxLayout* consensusCardLayout = static_cast<QVBoxLayout*>(consensusCard->layout());
    
    strictConsensusCheckBox = new QCheckBox("Strict Consensus Mode", consensusCard);
    strictConsensusCheckBox->setChecked(params.strictConsensus);
    strictConsensusCheckBox->setToolTip(
        "<b>Strict Consensus Mode</b><br/>"
        "<b>Unchecked (Lenient):</b> Includes regions detected by OCR alone if confidence is high<br/>"
        "<b>Checked (Strict):</b> Only includes regions detected by BOTH OCR and rectangle detection<br/>"
        "<b>Recommendation:</b> Use lenient for forms with many text fields, strict for structured forms"
    );
    consensusCardLayout->addWidget(strictConsensusCheckBox);
    consensusCardLayout->addStretch();
    
    gridLayout->addWidget(consensusCard, 3, 2);
    
    // === FINALIZE SCROLL AREA ===
    scrollArea->setWidget(scrollContent);
    mainLayout->addWidget(scrollArea, 1);  // Stretch factor
    
    // === Action Buttons ===
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(12);
    
    helpButton = new QPushButton("❓ Help", this);
    helpButton->setObjectName("helpButton");
    helpButton->setToolTip("Show detailed help and explanations for all parameters");
    
    resetButton = new QPushButton("🔄 Reset to Defaults", this);
    resetButton->setObjectName("resetButton");
    resetButton->setToolTip("Reset all parameters to their default values (Very Lenient preset)");
    
    cancelButton = new QPushButton("Cancel", this);
    cancelButton->setObjectName("cancelButton");
    cancelButton->setToolTip("Cancel and close this dialog without running detection");
    
    runButton = new QPushButton("▶️ Run Magic Detect", this);
    runButton->setObjectName("runButton");
    runButton->setToolTip("Run Magic Detect with the configured parameters");
    
    buttonLayout->addWidget(helpButton);
    buttonLayout->addWidget(resetButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(runButton);
    
    mainLayout->addLayout(buttonLayout);
}

void MagicDetectParamsDialog::connectSignals() {
    connect(helpButton, &QPushButton::clicked, this, &MagicDetectParamsDialog::showHelpDialog);
    connect(resetButton, &QPushButton::clicked, this, &MagicDetectParamsDialog::onResetClicked);
    connect(runButton, &QPushButton::clicked, this, &MagicDetectParamsDialog::onRunClicked);
    connect(cancelButton, &QPushButton::clicked, this, &MagicDetectParamsDialog::onCancelClicked);
    
    // Connect preset buttons
    connect(presetVeryLenientButton, &QPushButton::clicked, this, [this]() { applyPreset("VeryLenient"); });
    connect(presetBalancedButton, &QPushButton::clicked, this, [this]() { applyPreset("Balanced"); });
    connect(presetStrictButton, &QPushButton::clicked, this, [this]() { applyPreset("Strict"); });
    
    // Connect all spinboxes to update preview labels
    connect(ocrOverlapSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &MagicDetectParamsDialog::updatePreviewLabels);
    connect(baseBrightnessSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &MagicDetectParamsDialog::updatePreviewLabels);
    connect(brightnessAdaptiveSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &MagicDetectParamsDialog::updatePreviewLabels);
    connect(edgeDensitySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &MagicDetectParamsDialog::updatePreviewLabels);
    connect(horizontalEdgeDensitySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &MagicDetectParamsDialog::updatePreviewLabels);
    connect(verticalEdgeDensitySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &MagicDetectParamsDialog::updatePreviewLabels);
    connect(iouThresholdSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &MagicDetectParamsDialog::updatePreviewLabels);
    connect(ocrConfidenceSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &MagicDetectParamsDialog::updatePreviewLabels);
    connect(horizontalOverfitSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &MagicDetectParamsDialog::updatePreviewLabels);
    connect(verticalOverfitSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &MagicDetectParamsDialog::updatePreviewLabels);
    connect(minHorizontalLinesSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), 
            this, &MagicDetectParamsDialog::updatePreviewLabels);
    
    // Connect checkbox controls
    connect(minCheckboxSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), 
            this, &MagicDetectParamsDialog::updatePreviewLabels);
    connect(maxCheckboxSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), 
            this, &MagicDetectParamsDialog::updatePreviewLabels);
    connect(checkboxAspectRatioMinSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &MagicDetectParamsDialog::updatePreviewLabels);
    connect(checkboxAspectRatioMaxSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &MagicDetectParamsDialog::updatePreviewLabels);
    connect(checkboxRectangularitySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &MagicDetectParamsDialog::updatePreviewLabels);
}

void MagicDetectParamsDialog::updatePreviewLabels() {
    // OCR Overlap
    double ocrOverlapPercent = ocrOverlapSpinBox->value() * 100.0;
    ocrOverlapPreview->setText(
        QString("Regions overlapping with OCR text by >%1%% will be rejected")
            .arg(ocrOverlapPercent, 0, 'f', 1));
    
    // Brightness
    double baseBright = baseBrightnessSpinBox->value();
    double adaptive = brightnessAdaptiveSpinBox->value();
    brightnessPreview->setText(
        QString("Base: %1, Adaptive: %2%% of local brightness")
            .arg(baseBright, 0, 'f', 2)
            .arg(adaptive * 100.0, 0, 'f', 1));
    
    // Edge Density
    double totalEdge = edgeDensitySpinBox->value();
    double horizEdge = horizontalEdgeDensitySpinBox->value();
    double vertEdge = verticalEdgeDensitySpinBox->value();
    edgeDensityPreview->setText(
        QString("Total: %1, Horizontal: %2, Vertical: %3")
            .arg(totalEdge, 0, 'f', 3)
            .arg(horizEdge, 0, 'f', 3)
            .arg(vertEdge, 0, 'f', 3));
    
    // IoU
    double iou = iouThresholdSpinBox->value();
    iouPreview->setText(
        QString("Regions must overlap by >%1%% to be considered a match")
            .arg(iou * 100.0, 0, 'f', 1));
    
    // OCR Confidence
    double ocrConf = ocrConfidenceSpinBox->value();
    ocrConfidencePreview->setText(
        QString("Only OCR text with confidence ≥%1%% will be considered")
            .arg(ocrConf, 0, 'f', 1));
    
    // Overfitting
    double horizOverfit = horizontalOverfitSpinBox->value();
    double vertOverfit = verticalOverfitSpinBox->value();
    overfitPreview->setText(
        QString("Regions will expand by %1%% horizontally, %2%% vertically")
            .arg(horizOverfit, 0, 'f', 1)
            .arg(vertOverfit, 0, 'f', 1));
    
    // Hough Lines
    int minLines = minHorizontalLinesSpinBox->value();
    houghPreview->setText(
        QString("Regions with ≥%1 horizontal lines will be rejected as text")
            .arg(minLines));
    
    // Checkbox Detection
    int minSize = minCheckboxSizeSpinBox->value();
    int maxSize = maxCheckboxSizeSpinBox->value();
    double aspectMin = checkboxAspectRatioMinSpinBox->value();
    double aspectMax = checkboxAspectRatioMaxSpinBox->value();
    double rect = checkboxRectangularitySpinBox->value();
    bool enabled = enableStandaloneCheckboxCheckBox->isChecked();
    
    checkboxPreview->setText(
        QString("Size: %1-%2px | Aspect: %.1f-%.1f | Rectangularity: %.2f | Standalone: %3")
            .arg(minSize).arg(maxSize)
            .arg(aspectMin).arg(aspectMax)
            .arg(rect)
            .arg(enabled ? "ON" : "OFF"));
}

void MagicDetectParamsDialog::onResetClicked() {
    params.resetToDefaults();
    
    ocrOverlapSpinBox->setValue(params.ocrOverlapThreshold);
    baseBrightnessSpinBox->setValue(params.baseBrightnessThreshold);
    brightnessAdaptiveSpinBox->setValue(params.brightnessAdaptiveFactor);
    edgeDensitySpinBox->setValue(params.edgeDensityThreshold);
    horizontalEdgeDensitySpinBox->setValue(params.horizontalEdgeDensityThreshold);
    verticalEdgeDensitySpinBox->setValue(params.verticalEdgeDensityThreshold);
    iouThresholdSpinBox->setValue(params.iouThreshold);
    ocrConfidenceSpinBox->setValue(params.ocrConfidenceThreshold);
    horizontalOverfitSpinBox->setValue(params.horizontalOverfitPercent);
    verticalOverfitSpinBox->setValue(params.verticalOverfitPercent);
    minHorizontalLinesSpinBox->setValue(params.minHorizontalLines);
    strictConsensusCheckBox->setChecked(params.strictConsensus);
    
    // Checkbox parameters
    minCheckboxSizeSpinBox->setValue(params.minCheckboxSize);
    maxCheckboxSizeSpinBox->setValue(params.maxCheckboxSize);
    checkboxAspectRatioMinSpinBox->setValue(params.checkboxAspectRatioMin);
    checkboxAspectRatioMaxSpinBox->setValue(params.checkboxAspectRatioMax);
    checkboxRectangularitySpinBox->setValue(params.checkboxRectangularity);
    enableStandaloneCheckboxCheckBox->setChecked(params.enableStandaloneCheckboxDetection);
    
    updatePreviewLabels();
}

void MagicDetectParamsDialog::onRunClicked() {
    shouldRunDetection = true;
    saveSettings();
    accept();
}

void MagicDetectParamsDialog::onCancelClicked() {
    shouldRunDetection = false;
    reject();
}

DetectionParameters MagicDetectParamsDialog::getParameters() const {
    DetectionParameters p;
    p.ocrOverlapThreshold = ocrOverlapSpinBox->value();
    p.baseBrightnessThreshold = baseBrightnessSpinBox->value();
    p.brightnessAdaptiveFactor = brightnessAdaptiveSpinBox->value();
    p.edgeDensityThreshold = edgeDensitySpinBox->value();
    p.horizontalEdgeDensityThreshold = horizontalEdgeDensitySpinBox->value();
    p.verticalEdgeDensityThreshold = verticalEdgeDensitySpinBox->value();
    p.iouThreshold = iouThresholdSpinBox->value();
    p.ocrConfidenceThreshold = ocrConfidenceSpinBox->value();
    p.horizontalOverfitPercent = horizontalOverfitSpinBox->value();
    p.verticalOverfitPercent = verticalOverfitSpinBox->value();
    p.minHorizontalLines = minHorizontalLinesSpinBox->value();
    p.strictConsensus = strictConsensusCheckBox->isChecked();
    
    // Checkbox parameters
    p.minCheckboxSize = minCheckboxSizeSpinBox->value();
    p.maxCheckboxSize = maxCheckboxSizeSpinBox->value();
    p.checkboxAspectRatioMin = checkboxAspectRatioMinSpinBox->value();
    p.checkboxAspectRatioMax = checkboxAspectRatioMaxSpinBox->value();
    p.checkboxRectangularity = checkboxRectangularitySpinBox->value();
    p.enableStandaloneCheckboxDetection = enableStandaloneCheckboxCheckBox->isChecked();
    
    return p;
}

void MagicDetectParamsDialog::loadSettings() {
    QSettings settings;
    settings.beginGroup("MagicDetectParams");
    
    if (settings.contains("ocrOverlapThreshold")) {
        ocrOverlapSpinBox->setValue(settings.value("ocrOverlapThreshold").toDouble());
    }
    if (settings.contains("baseBrightnessThreshold")) {
        baseBrightnessSpinBox->setValue(settings.value("baseBrightnessThreshold").toDouble());
    }
    if (settings.contains("brightnessAdaptiveFactor")) {
        brightnessAdaptiveSpinBox->setValue(settings.value("brightnessAdaptiveFactor").toDouble());
    }
    if (settings.contains("edgeDensityThreshold")) {
        edgeDensitySpinBox->setValue(settings.value("edgeDensityThreshold").toDouble());
    }
    if (settings.contains("horizontalEdgeDensityThreshold")) {
        horizontalEdgeDensitySpinBox->setValue(settings.value("horizontalEdgeDensityThreshold").toDouble());
    }
    if (settings.contains("verticalEdgeDensityThreshold")) {
        verticalEdgeDensitySpinBox->setValue(settings.value("verticalEdgeDensityThreshold").toDouble());
    }
    if (settings.contains("iouThreshold")) {
        iouThresholdSpinBox->setValue(settings.value("iouThreshold").toDouble());
    }
    if (settings.contains("ocrConfidenceThreshold")) {
        ocrConfidenceSpinBox->setValue(settings.value("ocrConfidenceThreshold").toDouble());
    }
    if (settings.contains("horizontalOverfitPercent")) {
        horizontalOverfitSpinBox->setValue(settings.value("horizontalOverfitPercent").toDouble());
    }
    if (settings.contains("verticalOverfitPercent")) {
        verticalOverfitSpinBox->setValue(settings.value("verticalOverfitPercent").toDouble());
    }
    if (settings.contains("minHorizontalLines")) {
        minHorizontalLinesSpinBox->setValue(settings.value("minHorizontalLines").toInt());
    }
    if (settings.contains("strictConsensus")) {
        strictConsensusCheckBox->setChecked(settings.value("strictConsensus").toBool());
    }
    
    // Checkbox parameters
    if (settings.contains("minCheckboxSize")) {
        minCheckboxSizeSpinBox->setValue(settings.value("minCheckboxSize").toInt());
    }
    if (settings.contains("maxCheckboxSize")) {
        maxCheckboxSizeSpinBox->setValue(settings.value("maxCheckboxSize").toInt());
    }
    if (settings.contains("checkboxAspectRatioMin")) {
        checkboxAspectRatioMinSpinBox->setValue(settings.value("checkboxAspectRatioMin").toDouble());
    }
    if (settings.contains("checkboxAspectRatioMax")) {
        checkboxAspectRatioMaxSpinBox->setValue(settings.value("checkboxAspectRatioMax").toDouble());
    }
    if (settings.contains("checkboxRectangularity")) {
        checkboxRectangularitySpinBox->setValue(settings.value("checkboxRectangularity").toDouble());
    }
    if (settings.contains("enableStandaloneCheckboxDetection")) {
        enableStandaloneCheckboxCheckBox->setChecked(settings.value("enableStandaloneCheckboxDetection").toBool());
    }
    
    settings.endGroup();
}

void MagicDetectParamsDialog::saveSettings() {
    QSettings settings;
    settings.beginGroup("MagicDetectParams");
    
    settings.setValue("ocrOverlapThreshold", ocrOverlapSpinBox->value());
    settings.setValue("baseBrightnessThreshold", baseBrightnessSpinBox->value());
    settings.setValue("brightnessAdaptiveFactor", brightnessAdaptiveSpinBox->value());
    settings.setValue("edgeDensityThreshold", edgeDensitySpinBox->value());
    settings.setValue("horizontalEdgeDensityThreshold", horizontalEdgeDensitySpinBox->value());
    settings.setValue("verticalEdgeDensityThreshold", verticalEdgeDensitySpinBox->value());
    settings.setValue("iouThreshold", iouThresholdSpinBox->value());
    settings.setValue("ocrConfidenceThreshold", ocrConfidenceSpinBox->value());
    settings.setValue("horizontalOverfitPercent", horizontalOverfitSpinBox->value());
    settings.setValue("verticalOverfitPercent", verticalOverfitSpinBox->value());
    settings.setValue("minHorizontalLines", minHorizontalLinesSpinBox->value());
    settings.setValue("strictConsensus", strictConsensusCheckBox->isChecked());
    
    // Checkbox parameters
    settings.setValue("minCheckboxSize", minCheckboxSizeSpinBox->value());
    settings.setValue("maxCheckboxSize", maxCheckboxSizeSpinBox->value());
    settings.setValue("checkboxAspectRatioMin", checkboxAspectRatioMinSpinBox->value());
    settings.setValue("checkboxAspectRatioMax", checkboxAspectRatioMaxSpinBox->value());
    settings.setValue("checkboxRectangularity", checkboxRectangularitySpinBox->value());
    settings.setValue("enableStandaloneCheckboxDetection", enableStandaloneCheckboxCheckBox->isChecked());
    
    settings.endGroup();
}

void MagicDetectParamsDialog::applyPreset(const QString& presetName) {
    DetectionParameters preset;
    
    if (presetName == "VeryLenient") {
        // Very Lenient (current defaults)
        preset.ocrOverlapThreshold = 0.20;
        preset.baseBrightnessThreshold = 0.50;
        preset.brightnessAdaptiveFactor = 0.70;
        preset.edgeDensityThreshold = 0.15;
        preset.horizontalEdgeDensityThreshold = 0.25;
        preset.verticalEdgeDensityThreshold = 0.15;
        preset.iouThreshold = 0.40;
        preset.ocrConfidenceThreshold = 40.0;
        preset.horizontalOverfitPercent = 50.0;
        preset.verticalOverfitPercent = 70.0;
        preset.minHorizontalLines = 4;
        preset.strictConsensus = false;
        preset.minCheckboxSize = 10;
        preset.maxCheckboxSize = 60;
        preset.checkboxAspectRatioMin = 0.6;
        preset.checkboxAspectRatioMax = 1.6;
        preset.checkboxRectangularity = 0.5;
        preset.enableStandaloneCheckboxDetection = true;
    } else if (presetName == "Balanced") {
        // Balanced
        preset.ocrOverlapThreshold = 0.15;
        preset.baseBrightnessThreshold = 0.60;
        preset.brightnessAdaptiveFactor = 0.80;
        preset.edgeDensityThreshold = 0.10;
        preset.horizontalEdgeDensityThreshold = 0.15;
        preset.verticalEdgeDensityThreshold = 0.08;
        preset.iouThreshold = 0.50;
        preset.ocrConfidenceThreshold = 50.0;
        preset.horizontalOverfitPercent = 40.0;
        preset.verticalOverfitPercent = 60.0;
        preset.minHorizontalLines = 3;
        preset.strictConsensus = false;
        preset.minCheckboxSize = 12;
        preset.maxCheckboxSize = 50;
        preset.checkboxAspectRatioMin = 0.7;
        preset.checkboxAspectRatioMax = 1.4;
        preset.checkboxRectangularity = 0.6;
        preset.enableStandaloneCheckboxDetection = true;
    } else if (presetName == "Strict") {
        // Strict
        preset.ocrOverlapThreshold = 0.10;
        preset.baseBrightnessThreshold = 0.70;
        preset.brightnessAdaptiveFactor = 0.85;
        preset.edgeDensityThreshold = 0.08;
        preset.horizontalEdgeDensityThreshold = 0.10;
        preset.verticalEdgeDensityThreshold = 0.06;
        preset.iouThreshold = 0.60;
        preset.ocrConfidenceThreshold = 60.0;
        preset.horizontalOverfitPercent = 40.0;
        preset.verticalOverfitPercent = 60.0;
        preset.minHorizontalLines = 2;
        preset.strictConsensus = true;
        preset.minCheckboxSize = 15;
        preset.maxCheckboxSize = 40;
        preset.checkboxAspectRatioMin = 0.8;
        preset.checkboxAspectRatioMax = 1.2;
        preset.checkboxRectangularity = 0.7;
        preset.enableStandaloneCheckboxDetection = false;
    } else {
        return;  // Unknown preset
    }
    
    // Apply preset values
    ocrOverlapSpinBox->setValue(preset.ocrOverlapThreshold);
    baseBrightnessSpinBox->setValue(preset.baseBrightnessThreshold);
    brightnessAdaptiveSpinBox->setValue(preset.brightnessAdaptiveFactor);
    edgeDensitySpinBox->setValue(preset.edgeDensityThreshold);
    horizontalEdgeDensitySpinBox->setValue(preset.horizontalEdgeDensityThreshold);
    verticalEdgeDensitySpinBox->setValue(preset.verticalEdgeDensityThreshold);
    iouThresholdSpinBox->setValue(preset.iouThreshold);
    ocrConfidenceSpinBox->setValue(preset.ocrConfidenceThreshold);
    horizontalOverfitSpinBox->setValue(preset.horizontalOverfitPercent);
    verticalOverfitSpinBox->setValue(preset.verticalOverfitPercent);
    minHorizontalLinesSpinBox->setValue(preset.minHorizontalLines);
    strictConsensusCheckBox->setChecked(preset.strictConsensus);
    
    // Checkbox parameters
    minCheckboxSizeSpinBox->setValue(preset.minCheckboxSize);
    maxCheckboxSizeSpinBox->setValue(preset.maxCheckboxSize);
    checkboxAspectRatioMinSpinBox->setValue(preset.checkboxAspectRatioMin);
    checkboxAspectRatioMaxSpinBox->setValue(preset.checkboxAspectRatioMax);
    checkboxRectangularitySpinBox->setValue(preset.checkboxRectangularity);
    enableStandaloneCheckboxCheckBox->setChecked(preset.enableStandaloneCheckboxDetection);
    
    updatePreviewLabels();
}

void MagicDetectParamsDialog::showHelpDialog() {
    ThemeManager& themeManager = ThemeManager::instance();
    ThemeColors colors = themeManager.getColors();
    
    QDialog* helpDialog = new QDialog(this);
    helpDialog->setWindowTitle("Magic Detect - Help & Parameter Guide");
    helpDialog->setMinimumSize(800, 700);
    helpDialog->resize(900, 800);
    
    // Apply theme
    themeManager.applyTheme(helpDialog);
    
    QVBoxLayout* layout = new QVBoxLayout(helpDialog);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(15);
    
    QTextEdit* helpText = new QTextEdit(helpDialog);
    helpText->setReadOnly(true);
    helpText->setStyleSheet(
        QString(
            "QTextEdit {"
            "  background-color: %1;"
            "  color: %2;"
            "  border: 1px solid %3;"
            "  border-radius: 4px;"
            "  padding: 10px;"
            "  font-size: 10pt;"
            "}"
        ).arg(colors.surface.name(), colors.text.name(), colors.border.name())
    );
    
    QString html = QString(R"(
        <html>
        <head>
            <style>
                body { color: %1; background-color: %2; font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Arial, sans-serif; }
                h1 { color: %3; font-size: 24pt; margin-bottom: 10px; }
                h2 { color: %3; font-size: 16pt; margin-top: 20px; margin-bottom: 10px; border-bottom: 1px solid %4; padding-bottom: 5px; }
                h3 { color: %3; font-size: 13pt; margin-top: 15px; margin-bottom: 8px; }
                p { margin: 8px 0; line-height: 1.5; }
                ul { margin: 8px 0; padding-left: 25px; }
                li { margin: 5px 0; line-height: 1.4; }
                code { background-color: %5; padding: 2px 6px; border-radius: 3px; font-family: 'Courier New', monospace; }
                .param { background-color: %5; padding: 12px; margin: 10px 0; border-left: 3px solid %3; border-radius: 4px; }
                .param-name { font-weight: bold; color: %3; font-size: 11pt; }
                .default { color: %6; font-style: italic; }
            </style>
        </head>
        <body>
            <h1>🎯 Magic Detect - Parameter Guide</h1>
            
            <p>Magic Detect automatically identifies form fields, text areas, and checkboxes in your document. 
            This guide explains each parameter and how to adjust them for best results.</p>
            
            <h2>Quick Start</h2>
            <ul>
                <li><b>Very Lenient:</b> Catches everything, may include false positives. Good for initial scanning.</li>
                <li><b>Balanced:</b> Good default for most forms. Balanced between accuracy and coverage.</li>
                <li><b>Strict:</b> Fewer false positives, but may miss some fields. Use for clean, structured forms.</li>
            </ul>
            
            <h2>Text Detection Parameters</h2>
            
            <div class="param">
                <div class="param-name">📝 Text Overlap Threshold</div>
                <p>Regions that overlap with detected OCR text by more than this percentage are rejected as text regions.</p>
                <ul>
                    <li><b>Lower (0.1-0.3):</b> More lenient - allows regions near text</li>
                    <li><b>Higher (0.5-0.8):</b> More strict - rejects regions with any text overlap</li>
                    <li class="default">Default: 0.20 (20%)</li>
                </ul>
            </div>
            
            <div class="param">
                <div class="param-name">📊 OCR Confidence</div>
                <p>Minimum confidence level (0-100) required for OCR text to be considered valid.</p>
                <ul>
                    <li><b>Lower (20-40):</b> Includes more text, even with lower confidence</li>
                    <li><b>Higher (60-90):</b> Only includes high-confidence text</li>
                    <li class="default">Default: 40.0</li>
                </ul>
            </div>
            
            <div class="param">
                <div class="param-name">📏 Minimum Horizontal Lines</div>
                <p>Regions with this many or more horizontal lines will be rejected as text regions.</p>
                <p>Used to filter out paragraphs and text blocks from form field detection.</p>
                <ul>
                    <li><b>Lower (1-3):</b> More strict - rejects regions with few lines</li>
                    <li><b>Higher (5-8):</b> More lenient - allows regions with more lines</li>
                    <li class="default">Default: 4</li>
                </ul>
            </div>
            
            <h2>Image Analysis Parameters</h2>
            
            <div class="param">
                <div class="param-name">💡 Brightness Filter</div>
                <p><b>Base Threshold:</b> Base brightness level (0.0-1.0) for detecting form fields. Lower values detect darker regions (empty fields).</p>
                <p><b>Adaptive Factor:</b> Multiplier for local brightness. Final threshold = local_brightness × factor</p>
                <ul>
                    <li><b>Lower values:</b> More lenient - detects darker regions</li>
                    <li><b>Higher values:</b> More strict - only detects very dark regions</li>
                    <li class="default">Default: Base 0.50, Adaptive 0.70</li>
                </ul>
            </div>
            
            <div class="param">
                <div class="param-name">🔲 Edge Density Filter</div>
                <p>Measures the proportion of pixels that are edges (detected via Canny edge detection).</p>
                <ul>
                    <li><b>Total Edge Density:</b> Overall edge density required for form field detection</li>
                    <li><b>Horizontal Edge Density:</b> Used to identify text lines (high = likely text)</li>
                    <li><b>Vertical Edge Density:</b> Used to identify borders and vertical lines</li>
                    <li><b>Lower values:</b> More lenient - detects regions with fewer edges</li>
                    <li><b>Higher values:</b> More strict - requires more edges</li>
                    <li class="default">Default: Total 0.15, Horizontal 0.25, Vertical 0.15</li>
                </ul>
            </div>
            
            <h2>Checkbox Detection</h2>
            
            <div class="param">
                <div class="param-name">☑️ Checkbox Detection Parameters</div>
                <p><b>Enable Standalone Detection:</b> When enabled, scans entire image for checkboxes, not just near text.</p>
                <p><b>Size Range:</b> Minimum and maximum size in pixels for checkbox detection.</p>
                <p><b>Aspect Ratio:</b> Width/height ratio range (square = 1.0). Allows rectangular checkboxes.</p>
                <p><b>Rectangularity:</b> How close to perfect rectangle (0.0-1.0, where 1.0 = perfect).</p>
                <ul>
                    <li class="default">Default: Size 10-60px, Aspect 0.6-1.6, Rectangularity 0.50</li>
                </ul>
            </div>
            
            <h2>Advanced Parameters</h2>
            
            <div class="param">
                <div class="param-name">🔗 IoU (Intersection over Union) Threshold</div>
                <p>Minimum overlap (0.0-1.0) required for two detections to be considered the same region.</p>
                <p>Used to merge detections from different methods (OCR and rectangle detection).</p>
                <ul>
                    <li><b>Lower (0.2-0.4):</b> More lenient - merges regions with less overlap</li>
                    <li><b>Higher (0.6-0.8):</b> More strict - requires more overlap to merge</li>
                    <li class="default">Default: 0.40</li>
                </ul>
            </div>
            
            <div class="param">
                <div class="param-name">📐 Region Expansion</div>
                <p>Percentage (0-100) to expand detected regions horizontally and vertically.</p>
                <p>Useful for capturing text that extends slightly beyond detected boundaries.</p>
                <ul>
                    <li><b>Lower (0-30%):</b> Minimal expansion - tight boundaries</li>
                    <li><b>Higher (50-100%):</b> More expansion - captures surrounding area</li>
                    <li class="default">Default: Horizontal 50%, Vertical 70%</li>
                </ul>
            </div>
            
            <div class="param">
                <div class="param-name">⚙️ Strict Consensus Mode</div>
                <p><b>Unchecked (Lenient):</b> Includes regions detected by OCR alone if confidence is high</p>
                <p><b>Checked (Strict):</b> Only includes regions detected by BOTH OCR and rectangle detection</p>
                <ul>
                    <li><b>Recommendation:</b> Use lenient for forms with many text fields, strict for structured forms</li>
                    <li class="default">Default: Lenient (unchecked)</li>
                </ul>
            </div>
            
            <h2>Tips & Best Practices</h2>
            <ul>
                <li><b>Start with presets:</b> Try "Very Lenient" first, then adjust if you see too many false positives</li>
                <li><b>Adjust incrementally:</b> Change one parameter at a time to see its effect</li>
                <li><b>Use preview labels:</b> The preview text below each parameter shows the current value</li>
                <li><b>For dense forms:</b> Increase edge density thresholds to reduce false positives</li>
                <li><b>For sparse forms:</b> Decrease thresholds to catch more fields</li>
                <li><b>For checkboxes:</b> Enable standalone detection and adjust size range based on your checkbox size</li>
            </ul>
            
            <h2>Understanding the Detection Process</h2>
            <p>Magic Detect uses a multi-stage process:</p>
            <ol>
                <li><b>OCR Extraction:</b> Detects text regions using Tesseract OCR</li>
                <li><b>Pattern Analysis:</b> Analyzes edge patterns to identify form fields</li>
                <li><b>Rectangle Detection:</b> Finds rectangular regions that might be form fields</li>
                <li><b>Match & Merge:</b> Combines detections from different methods using IoU threshold</li>
                <li><b>Filtering:</b> Applies brightness, edge density, and text overlap filters</li>
                <li><b>Checkbox Detection:</b> Scans for square/rectangular shapes matching checkbox criteria</li>
            </ol>
            
        </body>
        </html>
    )").arg(colors.text.name(), colors.background.name(), colors.primary.name(), 
            colors.border.name(), colors.surface.name(), colors.textSecondary.name());
    
    helpText->setHtml(html);
    layout->addWidget(helpText);
    
    QPushButton* closeButton = new QPushButton("Close", helpDialog);
    closeButton->setObjectName("helpCloseButton");
    closeButton->setStyleSheet(
        QString(
            "QPushButton {"
            "  padding: 8px 20px;"
            "  background-color: %1;"
            "  color: %2;"
            "  border: 1px solid %3;"
            "  border-radius: 4px;"
            "}"
            "QPushButton:hover {"
            "  background-color: %4;"
            "}"
        ).arg(colors.primary.name(), "white", colors.primary.darker(120).name(), colors.primaryHover.name())
    );
    connect(closeButton, &QPushButton::clicked, helpDialog, &QDialog::accept);
    
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    layout->addLayout(buttonLayout);
    
    helpDialog->exec();
    delete helpDialog;
}

void MagicDetectParamsDialog::applyTheme() {
    ThemeManager& themeManager = ThemeManager::instance();
    ThemeColors colors = themeManager.getColors();
    
    // Apply base theme stylesheet
    themeManager.applyTheme(this);
    
    // Custom styles for dialog-specific elements
    QString dialogStylesheet;
    
    // Title
    dialogStylesheet += QString(
        "#dialogTitle {"
        "  font-size: 20pt;"
        "  font-weight: bold;"
        "  color: %1;"
        "  padding-bottom: 4px;"
        "}"
    ).arg(colors.text.name());
    
    // Description
    dialogStylesheet += QString(
        "#dialogDescription {"
        "  color: %1;"
        "  font-size: 10pt;"
        "  padding-bottom: 8px;"
        "}"
    ).arg(colors.textSecondary.name());
    
    // Scroll area
    dialogStylesheet += QString(
        "#dialogScrollArea {"
        "  border: none;"
        "  background-color: %1;"
        "}"
    ).arg(colors.background.name());
    
    // Preview labels
    dialogStylesheet += QString(
        "#previewLabel {"
        "  color: %1;"
        "  font-size: 9pt;"
        "  font-style: italic;"
        "}"
    ).arg(colors.textSecondary.name());
    
    // Spinboxes - make them larger and ensure text is visible
    dialogStylesheet += QString(
        "QDoubleSpinBox, QSpinBox {"
        "  background-color: %1;"
        "  color: %2;"
        "  border: 1px solid %3;"
        "  border-radius: 4px;"
        "  padding: 4px 8px;"
        "  font-size: 11pt;"
        "  font-weight: normal;"
        "  min-width: 120px;"
        "  min-height: 28px;"
        "}"
        "QDoubleSpinBox:hover, QSpinBox:hover {"
        "  border-color: %4;"
        "}"
        "QDoubleSpinBox:focus, QSpinBox:focus {"
        "  border-color: %4;"
        "  border-width: 2px;"
        "}"
    ).arg(colors.surface.name(), colors.text.name(), colors.border.name(), colors.primary.name());
    
    // Preset buttons
    dialogStylesheet += QString(
        "#presetVeryLenient {"
        "  padding: 10px 20px;"
        "  background-color: %1;"
        "  color: %2;"
        "  border: none;"
        "  border-radius: 6px;"
        "  font-weight: bold;"
        "}"
        "#presetVeryLenient:hover {"
        "  background-color: %3;"
        "}"
    ).arg(colors.success.name(), "white", colors.success.darker(120).name());
    
    dialogStylesheet += QString(
        "#presetBalanced {"
        "  padding: 10px 20px;"
        "  background-color: %1;"
        "  color: %2;"
        "  border: none;"
        "  border-radius: 6px;"
        "  font-weight: bold;"
        "}"
        "#presetBalanced:hover {"
        "  background-color: %3;"
        "}"
    ).arg(colors.primary.name(), "white", colors.primaryHover.name());
    
    dialogStylesheet += QString(
        "#presetStrict {"
        "  padding: 10px 20px;"
        "  background-color: %1;"
        "  color: %2;"
        "  border: none;"
        "  border-radius: 6px;"
        "  font-weight: bold;"
        "}"
        "#presetStrict:hover {"
        "  background-color: %3;"
        "}"
    ).arg(colors.error.name(), "white", colors.error.darker(120).name());
    
    // Action buttons
    dialogStylesheet += QString(
        "#resetButton {"
        "  padding: 10px 20px;"
        "  background-color: %1;"
        "  color: %2;"
        "  border: none;"
        "  border-radius: 6px;"
        "  font-weight: bold;"
        "}"
        "#resetButton:hover {"
        "  background-color: %3;"
        "}"
    ).arg(colors.textSecondary.name(), "white", colors.textSecondary.darker(120).name());
    
    dialogStylesheet += QString(
        "#cancelButton {"
        "  padding: 10px 20px;"
        "  background-color: %1;"
        "  color: %2;"
        "  border: 1px solid %3;"
        "  border-radius: 6px;"
        "}"
        "#cancelButton:hover {"
        "  background-color: %4;"
        "}"
    ).arg(colors.surface.name(), colors.text.name(), colors.border.name(), colors.selectionHover.name());
    
    dialogStylesheet += QString(
        "#runButton {"
        "  padding: 10px 30px;"
        "  background-color: %1;"
        "  color: %2;"
        "  border: none;"
        "  border-radius: 6px;"
        "  font-weight: bold;"
        "  font-size: 11pt;"
        "}"
        "#runButton:hover {"
        "  background-color: %3;"
        "}"
    ).arg(colors.success.name(), "white", colors.success.darker(120).name());
    
    // Help button
    dialogStylesheet += QString(
        "#helpButton {"
        "  padding: 10px 20px;"
        "  background-color: %1;"
        "  color: %2;"
        "  border: 1px solid %3;"
        "  border-radius: 6px;"
        "  font-weight: bold;"
        "}"
        "#helpButton:hover {"
        "  background-color: %4;"
        "}"
    ).arg(colors.surface.name(), colors.text.name(), colors.border.name(), colors.selectionHover.name());
    
    // Apply combined stylesheet
    setStyleSheet(styleSheet() + dialogStylesheet);
}

} // namespace ocr_orc


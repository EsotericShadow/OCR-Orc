#include "MainWindowDialogsController.h"
#include "../../components/dialogs/PreferencesDialog.h"
#include "../../components/dialogs/ExportDialog.h"
#include "../../../models/DocumentState.h"
#include "../../../models/RegionData.h"
#include <QtWidgets/QDialog>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTextEdit>

namespace ocr_orc {

MainWindowDialogsController::MainWindowDialogsController() {
}

MainWindowDialogsController::~MainWindowDialogsController() {
}

void MainWindowDialogsController::showHelp(::QWidget* parent) {
    if (!parent) {
        return;
    }
    
    // Use MainWindowHelpDialog functionality directly
    // (HelpDialog is a wrapper around MainWindowHelpDialog)
    QDialog* helpDialog = new QDialog(parent);
    helpDialog->setWindowTitle("OCR-Orc - Help");
    helpDialog->setMinimumSize(700, 800);
    helpDialog->resize(700, 800);
    
    QVBoxLayout* layout = new QVBoxLayout(helpDialog);
    layout->setContentsMargins(20, 20, 20, 20);
    
    QTextEdit* textEdit = new QTextEdit(helpDialog);
    textEdit->setReadOnly(true);
    textEdit->setStyleSheet("color: #000000; background-color: #ffffff;");
    
    QString helpText = R"(<h1>OCR-Orc Help</h1>
<h2>Getting Started</h2>
<p>OCR-Orc is a tool for calibrating PDF coordinates for OCR processing.</p>

<h2>Creating Regions</h2>
<ul>
<li>Select <b>Create Mode</b> from the toolbar</li>
<li>Click and drag on the canvas to create a rectangular region</li>
<li>Enter a name, color, and optional group in the control panel before creating</li>
</ul>

<h2>Selecting and Editing Regions</h2>
<ul>
<li>Select <b>Select Mode</b> from the toolbar</li>
<li>Click on a region to select it</li>
<li>Double-click a region to edit its name</li>
<li>Use the region editor in the side panel to modify properties</li>
</ul>

<h2>Resizing and Moving Regions</h2>
<ul>
<li>In Select Mode, drag a region to move it</li>
<li>Drag the resize handles to resize a region</li>
<li>Hold Shift while resizing to maintain aspect ratio</li>
</ul>

<h2>Groups</h2>
<ul>
<li>Select regions and click <b>Create Group</b> to group them</li>
<li>Use <b>Add to Group</b> to add selected regions to an existing group</li>
<li>Use <b>Remove from Group</b> to remove regions from a group</li>
</ul>

<h2>Export</h2>
<ul>
<li>Use <b>Export Coordinates</b> to save region coordinates as JSON or CSV</li>
<li>Use <b>Export Mask</b> to generate image masks for regions</li>
</ul>

<h2>Keyboard Shortcuts</h2>
<ul>
<li><b>Cmd+Z</b> (Mac) / <b>Ctrl+Z</b> (Windows/Linux): Undo</li>
<li><b>Cmd+Shift+Z</b> (Mac) / <b>Ctrl+Shift+Z</b> (Windows/Linux): Redo</li>
<li><b>Delete</b>: Delete selected regions</li>
</ul>
)";
    
    textEdit->setHtml(helpText);
    layout->addWidget(textEdit);
    
    QPushButton* closeButton = new QPushButton("Close", helpDialog);
    closeButton->setStyleSheet("color: #000000;");
    QObject::connect(closeButton, &QPushButton::clicked, helpDialog, &QDialog::accept);
    layout->addWidget(closeButton);
    
    helpDialog->exec();
    delete helpDialog;
}

void MainWindowDialogsController::showPreferences(::QWidget* parent) {
    if (!parent) {
        return;
    }
    
    PreferencesDialog dialog(parent);
    dialog.exec();
}

ExportDialog* MainWindowDialogsController::createExportDialog(::QWidget* parent) {
    if (!parent) {
        return nullptr;
    }
    
    return new ExportDialog(parent);
}

QString MainWindowDialogsController::showEditNameDialog(::QWidget* parent, const QString& currentName) {
    if (!parent) {
        return QString();
    }
    
    QDialog dialog(parent);
    dialog.setWindowTitle("Edit Region Name");
    dialog.setMinimumSize(350, 120);
    
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    layout->setSpacing(10);
    
    QLabel* label = new QLabel("New Name:", &dialog);
    label->setStyleSheet("color: #000000;");
    layout->addWidget(label);
    
    QLineEdit* lineEdit = new QLineEdit(&dialog);
    lineEdit->setText(currentName);
    lineEdit->selectAll();  // Select all for easy replacement
    lineEdit->setFocus();
    lineEdit->setStyleSheet("color: #000000; background-color: #ffffff;");
    layout->addWidget(lineEdit);
    
    QPushButton* saveButton = new QPushButton("Save", &dialog);
    saveButton->setStyleSheet("color: #000000;");
    QPushButton* cancelButton = new QPushButton("Cancel", &dialog);
    cancelButton->setStyleSheet("color: #000000;");
    
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);
    
    // Connect Enter key to save
    QObject::connect(lineEdit, &QLineEdit::returnPressed, saveButton, &QPushButton::click);
    QObject::connect(saveButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    QObject::connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);
    
    if (dialog.exec() == QDialog::Accepted) {
        return lineEdit->text().trimmed();
    }
    
    return QString(); // User cancelled
}

QString MainWindowDialogsController::showChangeColorDialog(::QWidget* parent,
                                                          DocumentState* documentState,
                                                          const QString& regionName,
                                                          const GetRegionCallback& getRegion) {
    if (!parent || !documentState || !documentState->hasRegion(regionName)) {
        return QString();
    }
    
    RegionData region = getRegion(regionName);
    QString currentColor = region.color;
    
    QDialog dialog(parent);
    dialog.setWindowTitle("Change Region Color");
    dialog.setMinimumSize(300, 150);
    
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    layout->setSpacing(10);
    
    QLabel* label = new QLabel("Select Color:", &dialog);
    label->setStyleSheet("color: #000000;");
    layout->addWidget(label);
    
    QComboBox* colorCombo = new QComboBox(&dialog);
    colorCombo->setStyleSheet("color: #000000; background-color: #ffffff;");
    QStringList colors = {"blue", "red", "green", "yellow", "purple", "orange", "cyan"};
    colorCombo->addItems(colors);
    
    // Pre-select current color
    int currentIndex = colors.indexOf(currentColor);
    if (currentIndex >= 0) {
        colorCombo->setCurrentIndex(currentIndex);
    } else {
        colorCombo->setCurrentIndex(0);
    }
    
    layout->addWidget(colorCombo);
    
    QPushButton* saveButton = new QPushButton("Save", &dialog);
    saveButton->setStyleSheet("color: #000000;");
    QPushButton* cancelButton = new QPushButton("Cancel", &dialog);
    cancelButton->setStyleSheet("color: #000000;");
    
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);
    
    QObject::connect(saveButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    QObject::connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);
    
    if (dialog.exec() == QDialog::Accepted) {
        return colorCombo->currentText();
    }
    
    return QString(); // User cancelled
}

} // namespace ocr_orc


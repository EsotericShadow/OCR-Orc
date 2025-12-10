#include "HelpDialog.h"
#include <QtWidgets/QDialog>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QPushButton>
#include <QtCore/QObject>

namespace ocr_orc {

MainWindowHelpDialog::MainWindowHelpDialog() {
}

MainWindowHelpDialog::~MainWindowHelpDialog() {
}

void MainWindowHelpDialog::showHelp(QWidget* parentWidget) {
    QDialog* helpDialog = new QDialog(parentWidget);
    helpDialog->setWindowTitle("OCR-Orc - Help");
    helpDialog->setMinimumSize(700, 800);
    helpDialog->resize(700, 800);
    
    QVBoxLayout* layout = new QVBoxLayout(helpDialog);
    layout->setContentsMargins(15, 15, 15, 15);
    layout->setSpacing(10);
    
    // Create scrollable text area
    QTextEdit* helpText = new QTextEdit(helpDialog);
    helpText->setReadOnly(true);
    helpText->setStyleSheet("color: #000000; background-color: #ffffff;");
    
    // Build help content
    QString helpContent = R"(
<h1>OCR-Orc - Help</h1>

<h2>Basic Workflow</h2>
<p><b>1. Load a PDF Document</b></p>
<ul>
<li>Click "Load PDF" button or press <code>Cmd+O</code> (macOS) / <code>Ctrl+O</code> (Windows/Linux)</li>
<li>Or drag and drop a PDF file onto the application window</li>
</ul>

<p><b>2. Create Regions</b></p>
<ul>
<li>Press <code>1</code> or click "Create" mode button</li>
<li>Click and drag on the canvas to create a rectangular region</li>
<li>Regions are automatically named (or use the Region Name field)</li>
</ul>

<p><b>3. Select and Manipulate Regions</b></p>
<ul>
<li>Press <code>2</code> or click "Select" mode button</li>
<li>Click a region to select it</li>
<li><code>Cmd+Click</code> / <code>Ctrl+Click</code> to multi-select</li>
<li><code>Shift+Drag</code> on selected region to move it</li>
<li>Drag resize handles (8 handles on corners/edges) to resize</li>
</ul>

<p><b>4. Organize into Groups</b></p>
<ul>
<li>Select 2+ regions</li>
<li>Press <code>Cmd+G</code> / <code>Ctrl+G</code> or click "Group Selected"</li>
<li>Enter a group name (e.g., "lastname", "firstname")</li>
</ul>

<p><b>5. Export Coordinates</b></p>
<ul>
<li>Click "Export Coordinates" or press <code>Cmd+S</code> / <code>Ctrl+S</code></li>
<li>Choose format: JSON or CSV</li>
<li>Coordinates are exported in normalized format (0.0-1.0)</li>
</ul>

<h2>Selection Modes</h2>

<p><b>Create Mode (Key: 1)</b></p>
<ul>
<li>Click and drag to create new regions</li>
<li>Cursor shows crosshair</li>
</ul>

<p><b>Select Mode (Key: 2)</b></p>
<ul>
<li>Click regions to select them</li>
<li>Move selected regions with <code>Shift+Drag</code></li>
<li>Resize with drag handles</li>
<li>Box select by dragging on empty canvas</li>
</ul>

<h2>Keyboard Shortcuts</h2>

<h3>File Operations</h3>
<ul>
<li><code>Cmd+O</code> / <code>Ctrl+O</code> - Load PDF</li>
<li><code>Cmd+S</code> / <code>Ctrl+S</code> - Export Coordinates</li>
<li><code>Cmd+Shift+S</code> / <code>Ctrl+Shift+S</code> - Load Coordinates</li>
<li><code>Cmd+M</code> / <code>Ctrl+M</code> - Export Mask</li>
</ul>

<h3>Mode Switching</h3>
<ul>
<li><code>1</code> - Switch to Create Mode</li>
<li><code>2</code> - Switch to Select Mode</li>
</ul>

<h3>Zoom Operations</h3>
<ul>
<li><code>+</code> or <code>=</code> - Zoom In</li>
<li><code>-</code> - Zoom Out</li>
<li><code>0</code> - Reset Zoom</li>
<li><code>Cmd+Wheel</code> / <code>Ctrl+Wheel</code> - Zoom to cursor</li>
</ul>

<h3>Region Operations</h3>
<ul>
<li><code>Delete</code> or <code>Backspace</code> - Delete Selected Regions</li>
<li><code>E</code> - Edit Selected Region Name</li>
<li><code>Escape</code> - Cancel drag/resize operation</li>
</ul>

<h3>Selection Operations</h3>
<ul>
<li><code>Cmd+A</code> / <code>Ctrl+A</code> - Select All</li>
<li><code>Cmd+D</code> / <code>Ctrl+D</code> - Duplicate Selected</li>
<li><code>Cmd+I</code> / <code>Ctrl+I</code> - Invert Selection</li>
<li><code>Cmd+Click</code> / <code>Ctrl+Click</code> - Toggle Multi-Select</li>
</ul>

<h3>Group Operations</h3>
<ul>
<li><code>Cmd+G</code> / <code>Ctrl+G</code> - Create Group from Selection</li>
<li><code>Cmd+Shift+G</code> / <code>Ctrl+Shift+G</code> - Remove from Group</li>
</ul>

<h3>Undo/Redo</h3>
<ul>
<li><code>Cmd+Z</code> / <code>Ctrl+Z</code> - Undo</li>
<li><code>Cmd+Shift+Z</code> / <code>Ctrl+Shift+Z</code> - Redo</li>
<li><code>Cmd+Y</code> / <code>Ctrl+Y</code> - Redo (alternative)</li>
</ul>

<h3>Help</h3>
<ul>
<li><code>F1</code> - Show this help window</li>
</ul>

<h2>Export Formats</h2>

<p><b>JSON Export</b></p>
<ul>
<li>Primary format with full metadata</li>
<li>Includes: PDF path, image size, aspect ratio, regions, groups</li>
<li>Coordinates in normalized format (0.0-1.0)</li>
<li>Version field for compatibility</li>
</ul>

<p><b>CSV Export</b></p>
<ul>
<li>Comma-separated values format</li>
<li>Compatible with Excel, Google Sheets, LibreOffice</li>
<li>Columns: Region Name, Group, Color, X1 (%), Y1 (%), X2 (%), Y2 (%)</li>
<li>RFC 4180 compliant with proper escaping</li>
</ul>

<h2>Mask Export</h2>
<ul>
<li>Generates black/white mask image for OCR preprocessing</li>
<li>Black background (hidden areas)</li>
<li>White rectangles for regions (visible for OCR)</li>
<li>Automatically exports coordinate JSON alongside mask</li>
<li>Uses image coordinates (pixels), not normalized</li>
</ul>

<h2>Coordinate Systems</h2>

<p>The application uses three coordinate systems:</p>

<p><b>1. Normalized Coordinates (0.0-1.0)</b></p>
<ul>
<li>Source of truth for export/import</li>
<li>Independent of image/display size</li>
<li>Always between 0.0 and 1.0</li>
</ul>

<p><b>2. Image Coordinates (Pixels)</b></p>
<ul>
<li>Calculated from normalized coordinates</li>
<li>Used for mask generation</li>
<li>Matches original PDF page dimensions</li>
</ul>

<p><b>3. Canvas Coordinates (Display Pixels)</b></p>
<ul>
<li>Calculated from normalized coordinates</li>
<li>Affected by zoom and pan</li>
<li>Used for rendering on screen</li>
</ul>

<h2>Tips and Tricks</h2>

<ul>
<li><b>Quick Region Creation:</b> Leave Region Name empty for auto-naming</li>
<li><b>Multi-Select:</b> Hold <code>Cmd</code> / <code>Ctrl</code> while clicking to select multiple regions</li>
<li><b>Box Selection:</b> Drag on empty canvas to select multiple regions at once</li>
<li><b>Zoom to Cursor:</b> Use <code>Cmd+Wheel</code> / <code>Ctrl+Wheel</code> to zoom centered on cursor</li>
<li><b>Pan Canvas:</b> Middle-click drag or <code>Alt+Left-click</code> drag</li>
<li><b>Cancel Operations:</b> Press <code>Escape</code> to cancel drag, resize, or creation</li>
<li><b>Undo Everything:</b> Undo/Redo supports up to 50 levels</li>
<li><b>Group Management:</b> Select regions from canvas or Regions list - both work!</li>
<li><b>Drag and Drop:</b> Drag PDF files directly onto the window to load them</li>
</ul>

<h2>Workflow Example</h2>

<ol>
<li>Load PDF document</li>
<li>Switch to Create Mode (<code>1</code>)</li>
<li>Create regions by clicking and dragging</li>
<li>Switch to Select Mode (<code>2</code>)</li>
<li>Select related regions (e.g., all "Last Name" cells)</li>
<li>Group them (<code>Cmd+G</code>) with name "lastname"</li>
<li>Repeat for other field types</li>
<li>Export coordinates (<code>Cmd+S</code>) in JSON or CSV format</li>
<li>Optionally export mask image for OCR preprocessing</li>
</ol>

<h2>Getting Help</h2>
<ul>
<li>Press <code>F1</code> to open this help window</li>
<li>Hover over buttons to see tooltips</li>
<li>All keyboard shortcuts are listed above</li>
</ul>
)";
    
    helpText->setHtml(helpContent);
    layout->addWidget(helpText);
    
    // Close button
    QPushButton* closeButton = new QPushButton("Close", helpDialog);
    closeButton->setStyleSheet("color: #000000;");
    QObject::connect(closeButton, &QPushButton::clicked, helpDialog, &QDialog::accept);
    
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    layout->addLayout(buttonLayout);
    
    helpDialog->exec();
    delete helpDialog;
}

} // namespace ocr_orc


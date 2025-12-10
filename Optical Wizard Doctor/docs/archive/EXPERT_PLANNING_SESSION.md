# Expert Planning Session: Optical Wizard Doctor C++ Implementation
## Executive Team Discussion - December 2025

---

## Participants

- **Sarah Chen** - Software Engineering Expert (20+ years, C++ standards committee contributor)
- **Marcus Rodriguez** - DevOps Expert (CI/CD, build systems, deployment automation)
- **Dr. Elena Volkov** - User Experience Expert (UX research, human-computer interaction)
- **James Park** - User Interface Expert (Modern UI/UX design, visual design systems)
- **Dr. Kenji Tanaka** - OCR Expert (Computer vision, document processing, OCR systems)

---

## Session Transcript

### Initial Analysis Phase

**Sarah Chen:** Alright team, I've reviewed all 12 specification documents and the Python prototype. This is a well-documented coordinate calibration tool for OCR workflows. Before we start arguing about architecture, let's establish what we're actually building.

**Dr. Kenji Tanaka:** The core value proposition is clear: users need to visually define extraction regions on PDF forms, export normalized coordinates (0.0-1.0) that work across resolutions, and generate mask images for OCR preprocessing. The three-coordinate system is sound - normalized as source of truth, image for pixel operations, canvas for display.

**Marcus Rodriguez:** Hold on. Before we dive into features, I need to understand the deployment target. Are we building for Windows, macOS, Linux, or all three? The Python prototype uses Tkinter which is cross-platform, but we're going C++.

**Sarah Chen:** Good point. The spec says "cross-platform compatible" but doesn't specify. Given modern C++ and the frameworks available, I'd recommend Qt 6.7 or 7.0. It's mature, well-documented, and handles cross-platform natively. But let's not commit yet.

**James Park:** I've reviewed the UI specification. It's functional but dated. The color scheme is basic, the layout is traditional desktop app from 2010. We need to modernize this while keeping functionality. Glassmorphism, dark mode, better typography - all the 2025 trends.

**Dr. Elena Volkov:** James, I appreciate the aesthetic concerns, but let's prioritize usability over trends. The workflow documentation shows users need to:
1. Load PDF
2. Create multiple regions quickly
3. Group them logically
4. Export coordinates

The current UI supports this. We should enhance, not redesign. Focus on efficiency, keyboard shortcuts, and reducing cognitive load.

**James Park:** I'm not suggesting a redesign. I'm saying we can have both - modern aesthetics AND better UX. The toolbar is cluttered. We can use a ribbon-style interface or a more compact design. And dark mode isn't a trend - it's accessibility.

**Dr. Elena Volkov:** Fair. Dark mode is essential. But let's validate the workflow first. The spec mentions 78 functions. That's a lot. Are we implementing all of them in phase one?

**Sarah Chen:** No. We need to break this down incrementally. Let me propose a phased approach:

**Phase 1: Core Foundation**
- Window setup, basic UI framework
- PDF loading and display
- Single coordinate system (normalized only, add others later)
- Basic region creation (click and drag)
- Simple export (JSON only)

**Phase 2: Interaction**
- Region selection and movement
- Resize handles
- Multi-selection
- Zoom and pan

**Phase 3: Organization**
- Groups
- Region editing (name, color)
- Undo/Redo

**Phase 4: Export/Import**
- All export formats
- Import functionality
- Mask generation

**Phase 5: Polish**
- Keyboard shortcuts
- Help system
- Performance optimization

**Marcus Rodriguez:** Sarah, I like the phases, but you're missing infrastructure. We need:
- Build system (CMake, obviously)
- Dependency management (vcpkg or Conan)
- CI/CD pipeline
- Testing framework
- Code organization

**Sarah Chen:** You're right. But let's not over-engineer. This isn't a 50-person team. We need:
- CMake for builds
- vcpkg for dependencies (Qt, PDF library, image processing)
- Unit tests for coordinate transformations
- Integration tests for export/import

**Dr. Kenji Tanaka:** Speaking of dependencies, what PDF library? The Python version uses pdf2image which relies on poppler. For C++, we have:
- Poppler (C++ bindings)
- PDFium (Google's PDF renderer)
- MuPDF (lightweight)

I'd recommend Poppler - it's mature, well-maintained, and handles PDF to image conversion reliably.

**Sarah Chen:** Poppler it is. Now, for the coordinate system - this is critical. The three-coordinate system must be bulletproof. I propose we create a dedicated `CoordinateSystem` module that handles all transformations. This is pure math, no UI dependencies.

**Dr. Elena Volkov:** I want to challenge the three-coordinate system. Do we really need canvas coordinates as a stored value? Can't we calculate them on-demand during rendering?

**Sarah Chen:** Technically yes, but performance. If we recalculate on every mouse move during drag operations, we'll have lag. Storing canvas coords and recalculating only on zoom/pan changes is more efficient.

**Dr. Elena Volkov:** Fair enough. But let's profile it. If the calculation is fast enough, we should calculate on-demand for simplicity.

**James Park:** Can we talk about the UI framework decision? Qt is fine, but it's heavy. What about Dear ImGui? It's immediate mode, perfect for tools like this.

**Sarah Chen:** Dear ImGui is great for developer tools, but this is a production application. Users expect native look and feel, proper accessibility, and platform integration. Qt provides that. Also, Qt has excellent PDF support through QtPDF module.

**James Park:** But Qt's styling is... limited. Custom styling requires QML or extensive CSS. For a modern look, we might struggle.

**Dr. Elena Volkov:** James, we can style Qt widgets. It's not as flexible as web, but it's sufficient. And native look is important for user trust and familiarity.

**Marcus Rodriguez:** Can we make a decision? Qt 6.7, CMake, vcpkg, Poppler. Are we agreed?

**All:** Agreed.

**Sarah Chen:** Good. Now let's talk architecture. I propose:

```
OWD/
├── src/
│   ├── core/           # Core business logic
│   │   ├── CoordinateSystem.cpp
│   │   ├── Region.cpp
│   │   ├── Group.cpp
│   │   └── Document.cpp
│   ├── ui/             # UI components
│   │   ├── MainWindow.cpp
│   │   ├── Canvas.cpp
│   │   ├── Toolbar.cpp
│   │   └── SidePanel.cpp
│   ├── export/         # Export/import
│   │   ├── JsonExporter.cpp
│   │   ├── CsvExporter.cpp
│   │   └── MaskGenerator.cpp
│   └── utils/          # Utilities
│       └── PdfLoader.cpp
├── tests/
├── docs/
└── CMakeLists.txt
```

**Dr. Elena Volkov:** I like the separation, but I'd add a `models/` directory for data structures. Keep the data models separate from business logic.

**Sarah Chen:** Good point. Let's add `models/` for Region, Group, Document state structures.

**Marcus Rodriguez:** What about the build structure? I want:
- Debug and Release builds
- Separate build directory
- Install targets
- Package generation

**Sarah Chen:** Standard CMake setup. We'll use modern CMake (3.20+), target-based approach, proper dependency management.

**Dr. Kenji Tanaka:** Before we finalize, I need to address the OCR test functionality. The Python version uses EasyOCR. For C++, we have:
- Tesseract (most common)
- PaddleOCR (C++ bindings)
- Custom integration

I'd suggest making OCR optional - it's not core to the calibration tool. Users can test extraction in their own pipeline.

**Sarah Chen:** Agreed. OCR testing is nice-to-have, not essential. We can add it later if needed.

**James Park:** Can we talk about the visual design? The spec has specific colors, but we should support theming. At minimum, light and dark modes.

**Dr. Elena Volkov:** Absolutely. Theming is essential. But let's implement it properly - not just color swapping, but proper contrast ratios, accessibility compliance.

**Sarah Chen:** Let's add a `themes/` directory for style definitions. We can use Qt's style sheets or QML styling.

**Marcus Rodriguez:** I'm concerned about the export formats. Five formats (JSON, CSV, TXT, XML, YAML) plus mask images. That's a lot of code. Can we use libraries?

**Sarah Chen:** 
- JSON: Qt's QJsonDocument (built-in)
- CSV: Simple, we can write ourselves
- TXT: Simple formatting
- XML: Qt's QXmlStreamWriter (built-in)
- YAML: We'll need a library (yaml-cpp via vcpkg)
- Images: Qt's QImage (built-in)

Most is covered by Qt. YAML needs a library, but it's optional.

**Dr. Elena Volkov:** Let's prioritize. JSON is essential - it's the primary format. CSV is useful for spreadsheets. TXT is human-readable. XML and YAML are nice-to-have.

**Sarah Chen:** Agreed. Phase 4 can start with JSON and CSV, add others incrementally.

**Marcus Rodriguez:** One more thing - the undo/redo system. The spec says 50 levels. That's a lot of state to store. We need efficient serialization.

**Sarah Chen:** We'll use deep copies of the data structures. For 50 levels with potentially 500 regions, that's manageable. We can optimize later if needed.

**Dr. Kenji Tanaka:** The mask generation is interesting. Creating a black image with white rectangles. This should be straightforward with Qt's QImage.

**James Park:** Can we finalize the UI framework decision? Qt with QWidgets or QML?

**Sarah Chen:** QWidgets for the main application. It's more mature, better for complex desktop apps. QML is great for touch interfaces and simpler UIs, but this needs fine-grained control.

**Dr. Elena Volkov:** I agree. QWidgets gives us more control over interactions, which is critical for precise region selection.

**Marcus Rodriguez:** Alright, let's create the project structure. I'll set up:
- CMake configuration
- vcpkg integration
- Basic CI/CD (GitHub Actions or similar)
- Directory structure

**Sarah Chen:** Perfect. Let's break this into concrete implementation steps. I'll create a detailed breakdown.

---

## Final Consensus

### Technology Stack
- **Framework:** Qt 6.7 (QWidgets)
- **Build System:** CMake 3.20+
- **Package Manager:** vcpkg
- **PDF Library:** Poppler (via vcpkg)
- **Image Processing:** Qt QImage
- **Testing:** Qt Test + Catch2 (for unit tests)
- **CI/CD:** GitHub Actions (or GitLab CI)

### Architecture Principles
1. **Separation of Concerns:** Core logic separate from UI
2. **Incremental Development:** Build and test each phase before moving on
3. **Performance First:** Optimize coordinate transformations early
4. **Accessibility:** WCAG 2.1 AA compliance from the start
5. **Cross-Platform:** Test on Windows, macOS, Linux

### Development Phases
1. **Foundation:** Window, PDF loading, basic region creation
2. **Interaction:** Selection, movement, resize, zoom
3. **Organization:** Groups, editing, undo/redo
4. **Export/Import:** All formats, mask generation
5. **Polish:** Shortcuts, help, optimization

---

## Next Steps

The team will now create detailed implementation plans for each phase, broken down into specific tasks with clear acceptance criteria.


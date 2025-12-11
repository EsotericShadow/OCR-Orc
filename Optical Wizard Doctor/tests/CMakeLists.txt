# Tests CMakeLists.txt
# Qt Test framework configuration

if(NOT Qt6Test_FOUND)
    message(WARNING "Qt Test not found - skipping test configuration")
    return()
endif()

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/models
    ${CMAKE_SOURCE_DIR}/src/ui
    ${CMAKE_SOURCE_DIR}/src/utils
)

# Poppler (needed for PdfLoader used by DocumentState)
find_package(PkgConfig REQUIRED)
pkg_check_modules(POPPLER_CPP REQUIRED poppler-cpp)
include_directories(${POPPLER_CPP_INCLUDE_DIRS})
link_directories(${POPPLER_CPP_LIBRARY_DIRS})
link_libraries(${POPPLER_CPP_LIBRARIES})

# Test executables
# 1. Coordinate System Tests
add_executable(test_coordinate_system
    test_coordinate_system.cpp
    ${CMAKE_SOURCE_DIR}/src/core/CoordinateSystem.cpp
)
target_link_libraries(test_coordinate_system
    Qt6::Core
    Qt6::Test
)
add_test(NAME CoordinateSystemTests COMMAND test_coordinate_system)

# 1b. Coordinate System Comprehensive Tests
add_executable(test_coordinate_system_comprehensive
    test_coordinate_system_comprehensive.cpp
    ${CMAKE_SOURCE_DIR}/src/core/CoordinateSystem.cpp
)
target_link_libraries(test_coordinate_system_comprehensive
    Qt6::Core
    Qt6::Test
)
add_test(NAME CoordinateSystemComprehensiveTests COMMAND test_coordinate_system_comprehensive)

# 2. Data Models Tests
add_executable(test_data_models
    test_data_models.cpp
    ${CMAKE_SOURCE_DIR}/src/core/CoordinateSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/models/RegionData.cpp
    ${CMAKE_SOURCE_DIR}/src/models/GroupData.cpp
    ${CMAKE_SOURCE_DIR}/src/models/DocumentState.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/PdfLoader.cpp
)
target_link_libraries(test_data_models
    Qt6::Core
    Qt6::Test
    Qt6::Gui
)
add_test(NAME DataModelsTests COMMAND test_data_models)

# 3. Canvas Module Tests (separate executables for each module)
# Common source files for all canvas tests
set(CANVAS_TEST_SOURCES
    ${CMAKE_SOURCE_DIR}/src/core/CoordinateSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/models/RegionData.cpp
    ${CMAKE_SOURCE_DIR}/src/models/GroupData.cpp
    ${CMAKE_SOURCE_DIR}/src/models/DocumentState.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/PdfLoader.cpp
)

# CanvasCoordinateCache test
add_executable(test_canvas_coordinate_cache
    test_canvas_coordinate_cache.cpp
    ${CANVAS_TEST_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/ui/canvas/core/coordinate/CanvasCoordinateCache.cpp
)
target_link_libraries(test_canvas_coordinate_cache
    Qt6::Core
    Qt6::Test
    Qt6::Gui
)
add_test(NAME CanvasCoordinateCacheTest COMMAND test_canvas_coordinate_cache)

# CanvasZoomController test
add_executable(test_canvas_zoom_controller
    test_canvas_zoom_controller.cpp
    ${CANVAS_TEST_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/ui/canvas/core/coordinate/CanvasCoordinateCache.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/canvas/core/zoom/CanvasZoomController.cpp
)
target_link_libraries(test_canvas_zoom_controller
    Qt6::Core
    Qt6::Test
    Qt6::Gui
)
add_test(NAME CanvasZoomControllerTest COMMAND test_canvas_zoom_controller)

# CanvasRegionOperations test
add_executable(test_canvas_region_operations
    test_canvas_region_operations.cpp
    ${CANVAS_TEST_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/ui/canvas/core/regions/CanvasRegionOperations.cpp
)
target_link_libraries(test_canvas_region_operations
    Qt6::Core
    Qt6::Test
    Qt6::Gui
)
add_test(NAME CanvasRegionOperationsTest COMMAND test_canvas_region_operations)

# CanvasSelectionManager test
add_executable(test_canvas_selection_manager
    test_canvas_selection_manager.cpp
    ${CANVAS_TEST_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/ui/canvas/core/selection/CanvasSelectionManager.cpp
)
target_link_libraries(test_canvas_selection_manager
    Qt6::Core
    Qt6::Test
    Qt6::Gui
)
add_test(NAME CanvasSelectionManagerTest COMMAND test_canvas_selection_manager)

# CanvasHitTester test
add_executable(test_canvas_hit_tester
    test_canvas_hit_tester.cpp
    ${CANVAS_TEST_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/ui/canvas/core/coordinate/CanvasHitTester.cpp
)
target_link_libraries(test_canvas_hit_tester
    Qt6::Core
    Qt6::Test
    Qt6::Gui
)
add_test(NAME CanvasHitTesterTest COMMAND test_canvas_hit_tester)

# CanvasRegionCreator test
add_executable(test_canvas_region_creator
    test_canvas_region_creator.cpp
    ${CANVAS_TEST_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/ui/canvas/core/regions/CanvasRegionCreator.cpp
)
target_link_libraries(test_canvas_region_creator
    Qt6::Core
    Qt6::Test
    Qt6::Gui
)
add_test(NAME CanvasRegionCreatorTest COMMAND test_canvas_region_creator)

# CanvasRotation test
add_executable(test_canvas_rotation
    test_canvas_rotation.cpp
    ${CANVAS_TEST_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/ui/canvas/core/coordinate/CanvasHitTester.cpp
)
target_link_libraries(test_canvas_rotation
    Qt6::Core
    Qt6::Test
    Qt6::Gui
)
add_test(NAME CanvasRotationTest COMMAND test_canvas_rotation)

# 3b. Region Editor Fields Test
add_executable(test_region_editor_fields
    test_region_editor_fields.cpp
    ${CANVAS_TEST_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/ui/components/widgets/SidePanelWidget.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/components/widgets/ToolbarWidget.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/components/widgets/ControlPanelWidget.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/components/widgets/ModeToggleWidget.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/components/icons/IconManager.cpp
)
target_link_libraries(test_region_editor_fields
    Qt6::Core
    Qt6::Test
    Qt6::Gui
    Qt6::Widgets
    Qt6::Svg
)
add_test(NAME RegionEditorFieldsTest COMMAND test_region_editor_fields)

# 4. Export/Import Tests
add_executable(test_json_exporter
    test_json_exporter.cpp
    ${CANVAS_TEST_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/export/JsonExporter.cpp
)
target_link_libraries(test_json_exporter
    Qt6::Core
    Qt6::Test
    Qt6::Gui
)
add_test(NAME JsonExporterTest COMMAND test_json_exporter)

add_executable(test_json_importer
    test_json_importer.cpp
    ${CANVAS_TEST_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/export/JsonImporter.cpp
)
target_link_libraries(test_json_importer
    Qt6::Core
    Qt6::Test
    Qt6::Gui
)
add_test(NAME JsonImporterTest COMMAND test_json_importer)

add_executable(test_csv_exporter
    test_csv_exporter.cpp
    ${CANVAS_TEST_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/export/CsvExporter.cpp
)
target_link_libraries(test_csv_exporter
    Qt6::Core
    Qt6::Test
    Qt6::Gui
)
add_test(NAME CsvExporterTest COMMAND test_csv_exporter)

add_executable(test_mask_generator
    test_mask_generator.cpp
    ${CANVAS_TEST_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/export/MaskGenerator.cpp
    ${CMAKE_SOURCE_DIR}/src/export/JsonExporter.cpp
)
target_link_libraries(test_mask_generator
    Qt6::Core
    Qt6::Test
    Qt6::Gui
)
add_test(NAME MaskGeneratorTest COMMAND test_mask_generator)

# 5. Utility Tests
add_executable(test_input_validator
    test_input_validator.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/InputValidator.cpp
)
target_link_libraries(test_input_validator
    Qt6::Core
    Qt6::Test
)
add_test(NAME InputValidatorTest COMMAND test_input_validator)

# 6. Undo/Redo Tests (if exists)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/test_undo_redo.cpp)
    add_executable(test_undo_redo
        test_undo_redo.cpp
        ${CANVAS_TEST_SOURCES}
    )
    target_link_libraries(test_undo_redo
        Qt6::Core
        Qt6::Test
        Qt6::Gui
    )
    add_test(NAME UndoRedoTests COMMAND test_undo_redo)

# MainWindow Integration Tests
add_executable(test_mainwindow_integration
    test_mainwindow_integration.cpp
    ${CANVAS_TEST_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/ui/MainWindow.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/ThemeManager.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/canvas/Canvas.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/components/widgets/ToolbarWidget.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/components/widgets/ControlPanelWidget.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/components/widgets/SidePanelWidget.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/components/widgets/ModeToggleWidget.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/components/icons/IconManager.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/mainwindow/operations/file/MainWindowFileOperations.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/mainwindow/operations/region/MainWindowRegionOperations.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/mainwindow/operations/group/MainWindowGroupOperations.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/mainwindow/operations/undo/MainWindowUndoRedo.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/mainwindow/operations/selection/MainWindowSelectionOperations.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/mainwindow/ui/MainWindowUIUpdates.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/mainwindow/ui/MainWindowToolbarAdapter.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/mainwindow/dialogs/MainWindowDialogsController.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/mainwindow/adapters/UiDataAdapters.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/mainwindow/adapters/MainWindowUndoRedoAdapter.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/mainwindow/shortcuts/MainWindowKeyboardShortcuts.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/mainwindow/setup/MainWindowUISetup.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/mainwindow/setup/MainWindowWidgetConnections.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/mainwindow/setup/MainWindowCanvasWiring.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/mainwindow/events/MainWindowEventHandlers.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/mainwindow/handlers/actions/MainWindowActionHandlers.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/mainwindow/handlers/regions/MainWindowRegionGroupHandlers.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/mainwindow/handlers/regions/MainWindowRegionEditorHandlers.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/mainwindow/handlers/view/MainWindowViewHandlers.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/components/dialogs/HelpDialog.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/components/dialogs/ExportDialog.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/components/dialogs/PreferencesDialog.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/canvas/Canvas.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/canvas/core/coordinate/CanvasCoordinateCache.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/canvas/core/coordinate/CanvasHitTester.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/canvas/core/rendering/CanvasRenderer.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/canvas/core/zoom/CanvasZoomController.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/canvas/core/selection/CanvasSelectionManager.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/canvas/ui/CanvasUiSync.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/canvas/core/regions/CanvasRegionOperations.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/canvas/core/regions/CanvasRegionCreator.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/canvas/input/CanvasInputHandler.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/canvas/input/CanvasMouseHandlerCommon.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/canvas/input/CanvasMouseHandlerPress.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/canvas/input/CanvasMouseHandlerMove.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/canvas/input/CanvasMouseHandlerRelease.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/canvas/input/CanvasMouseHandlerDoubleClick.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/canvas/input/CanvasKeyboardHandler.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/canvas/input/CanvasWheelHandler.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/canvas/input/CanvasHoverManager.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/canvas/managers/CanvasRegionManager.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/canvas/managers/CanvasRegionCreationManager.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/canvas/managers/CanvasZoomManager.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/canvas/managers/CanvasStateManager.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/canvas/ui/CanvasContextMenuBuilder.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/PdfLoader.cpp
    ${CMAKE_SOURCE_DIR}/src/export/JsonExporter.cpp
    ${CMAKE_SOURCE_DIR}/src/export/JsonImporter.cpp
    ${CMAKE_SOURCE_DIR}/src/export/CsvExporter.cpp
    ${CMAKE_SOURCE_DIR}/src/export/MaskGenerator.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/InputValidator.cpp
)
# Find Poppler for test
find_package(PkgConfig REQUIRED)
pkg_check_modules(POPPLER_CPP REQUIRED poppler-cpp)

target_link_libraries(test_mainwindow_integration
    Qt6::Core
    Qt6::Test
    Qt6::Gui
    Qt6::Widgets
    Qt6::Svg
    ${POPPLER_CPP_LIBRARIES}
)

target_include_directories(test_mainwindow_integration PRIVATE ${POPPLER_CPP_INCLUDE_DIRS})
target_compile_options(test_mainwindow_integration PRIVATE ${POPPLER_CPP_CFLAGS_OTHER})

if(APPLE)
    target_link_directories(test_mainwindow_integration PRIVATE ${POPPLER_CPP_LIBRARY_DIRS})
endif()
add_test(NAME MainWindowIntegrationTests COMMAND test_mainwindow_integration)

# ThemeManager Tests
add_executable(test_theme_manager
    test_theme_manager.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/ThemeManager.cpp
)
target_link_libraries(test_theme_manager
    Qt6::Core
    Qt6::Test
    Qt6::Gui
    Qt6::Widgets
)
add_test(NAME ThemeManagerTests COMMAND test_theme_manager)

# OCR-First Detection Tests
# Note: These tests require Tesseract to be installed
# Find Tesseract for OCR tests
pkg_check_modules(TESSERACT REQUIRED tesseract)

# OcrTextExtractor test
add_executable(test_ocr_text_extractor
    test_ocr_text_extractor.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/OcrTextExtractor.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/ImageConverter.cpp
    ${CMAKE_SOURCE_DIR}/src/core/CoordinateSystem.cpp
)
target_link_libraries(test_ocr_text_extractor
    Qt6::Core
    Qt6::Test
    Qt6::Gui
    ${TESSERACT_LIBRARIES}
    ${OpenCV_LIBS}
)
target_include_directories(test_ocr_text_extractor PRIVATE ${TESSERACT_INCLUDE_DIRS})
add_test(NAME OcrTextExtractorTest COMMAND test_ocr_text_extractor)

# TextRegionRefiner test
add_executable(test_text_region_refiner
    test_text_region_refiner.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/TextRegionRefiner.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/OcrTextExtractor.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/ImageConverter.cpp
    ${CMAKE_SOURCE_DIR}/src/core/CoordinateSystem.cpp
)
target_link_libraries(test_text_region_refiner
    Qt6::Core
    Qt6::Test
    Qt6::Gui
    ${OpenCV_LIBS}
)
add_test(NAME TextRegionRefinerTest COMMAND test_text_region_refiner)

# CheckboxDetector test
add_executable(test_checkbox_detector
    test_checkbox_detector.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/CheckboxDetector.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/OcrTextExtractor.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/ImageConverter.cpp
    ${CMAKE_SOURCE_DIR}/src/core/CoordinateSystem.cpp
)
target_link_libraries(test_checkbox_detector
    Qt6::Core
    Qt6::Test
    Qt6::Gui
    ${OpenCV_LIBS}
)
add_test(NAME CheckboxDetectorTest COMMAND test_checkbox_detector)

# PatternAnalyzer test
add_executable(test_pattern_analyzer
    test_pattern_analyzer.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/PatternAnalyzer.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/CheckboxDetector.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/OcrTextExtractor.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/ImageConverter.cpp
    ${CMAKE_SOURCE_DIR}/src/core/CoordinateSystem.cpp
)
target_link_libraries(test_pattern_analyzer
    Qt6::Core
    Qt6::Test
    Qt6::Gui
    ${OpenCV_LIBS}
)
add_test(NAME PatternAnalyzerTest COMMAND test_pattern_analyzer)

# ConfidenceCalculator test
add_executable(test_confidence_calculator
    test_confidence_calculator.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/ConfidenceCalculator.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/RegionDetector.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/ImageConverter.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/TypeInferencer.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/GroupInferencer.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/SpatialClusterer.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/RegionValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/patterns/PostalCodePatternDetector.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/patterns/NameFieldPatternDetector.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/patterns/NumberSequencePatternDetector.cpp
    ${CMAKE_SOURCE_DIR}/src/core/CoordinateSystem.cpp
)
target_link_libraries(test_confidence_calculator
    Qt6::Core
    Qt6::Test
    Qt6::Gui
    ${OpenCV_LIBS}
)
add_test(NAME ConfidenceCalculatorTest COMMAND test_confidence_calculator)

# OCR-First Integration Test
add_executable(test_ocr_first_integration
    test_ocr_first_integration.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/RegionDetector.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/OcrTextExtractor.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/TextRegionRefiner.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/CheckboxDetector.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/PatternAnalyzer.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/FormFieldDetector.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/ConfidenceCalculator.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/ImageConverter.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/TypeInferencer.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/GroupInferencer.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/SpatialClusterer.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/RegionValidator.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/patterns/PostalCodePatternDetector.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/patterns/NameFieldPatternDetector.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/patterns/NumberSequencePatternDetector.cpp
    ${CMAKE_SOURCE_DIR}/src/core/CoordinateSystem.cpp
)
target_link_libraries(test_ocr_first_integration
    Qt6::Core
    Qt6::Test
    Qt6::Gui
    ${TESSERACT_LIBRARIES}
    ${OpenCV_LIBS}
)
target_include_directories(test_ocr_first_integration PRIVATE ${TESSERACT_INCLUDE_DIRS})
add_test(NAME OcrFirstIntegrationTest COMMAND test_ocr_first_integration)

endif()


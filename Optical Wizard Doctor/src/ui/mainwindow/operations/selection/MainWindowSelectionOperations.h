#ifndef MAIN_WINDOW_SELECTION_OPERATIONS_H
#define MAIN_WINDOW_SELECTION_OPERATIONS_H

#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QSet>
#include <functional>

namespace ocr_orc {

// Forward declarations
class DocumentState;
class Canvas;

/**
 * @brief Handles selection operations for MainWindow
 * 
 * Manages:
 * - Select all regions
 * - Invert selection
 */
class MainWindowSelectionOperations {
public:
    // Callback types
    using GetAllRegionNamesCallback = std::function<QList<QString>()>;
    using GetSelectedRegionsCallback = std::function<QSet<QString>()>;
    using SetSelectedRegionsCallback = std::function<void(const QSet<QString>&)>;
    using UpdateCanvasCallback = std::function<void()>;
    using UpdateRegionListBoxCallback = std::function<void()>;
    
    MainWindowSelectionOperations();
    ~MainWindowSelectionOperations();
    
    /**
     * @brief Select all regions
     * @param documentState Document state
     * @param canvas Canvas widget
     * @param callbacks Callback functions
     */
    void selectAll(DocumentState* documentState,
                   Canvas* canvas,
                   const GetAllRegionNamesCallback& getAllRegionNames,
                   const SetSelectedRegionsCallback& setSelectedRegions,
                   const UpdateCanvasCallback& updateCanvas,
                   const UpdateRegionListBoxCallback& updateRegionListBox);
    
    /**
     * @brief Invert the current selection
     * @param documentState Document state
     * @param canvas Canvas widget
     * @param callbacks Callback functions
     */
    void invertSelection(DocumentState* documentState,
                        Canvas* canvas,
                        const GetAllRegionNamesCallback& getAllRegionNames,
                        const GetSelectedRegionsCallback& getSelectedRegions,
                        const SetSelectedRegionsCallback& setSelectedRegions,
                        const UpdateCanvasCallback& updateCanvas,
                        const UpdateRegionListBoxCallback& updateRegionListBox);
};

} // namespace ocr_orc

#endif // MAIN_WINDOW_SELECTION_OPERATIONS_H


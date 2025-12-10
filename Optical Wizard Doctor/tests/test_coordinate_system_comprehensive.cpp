// Comprehensive test suite for CoordinateSystem
// Tests edge cases, integration scenarios, and prepares for Data Models phase

#include "../src/core/CoordinateSystem.h"
#include <QtCore/QPointF>
#include <QtCore/QRectF>
#include <iostream>
#include <cassert>
#include <cmath>
#include <limits>

using namespace ocr_orc;

const double EPSILON = 0.0001;
const double ROUND_TRIP_EPSILON = 0.001; // Allow more tolerance for round-trip

// Test helper: Check if two doubles are approximately equal
bool approxEqual(double a, double b, double epsilon = EPSILON) {
    return std::abs(a - b) < epsilon;
}

// Test helper: Check if two ImageCoords are approximately equal
bool approxEqual(const ImageCoords& a, const ImageCoords& b, int tolerance = 1) {
    return std::abs(a.x1 - b.x1) <= tolerance &&
           std::abs(a.y1 - b.y1) <= tolerance &&
           std::abs(a.x2 - b.x2) <= tolerance &&
           std::abs(a.y2 - b.y2) <= tolerance;
}

// ============================================================================
// EDGE CASES AND BOUNDARY CONDITIONS
// ============================================================================

void testEdgeCases() {
    std::cout << "\n=== Testing Edge Cases ===\n";
    
    // Test 1: Zero dimensions (division by zero protection)
    std::cout << "  Testing zero dimensions...\n";
    ImageCoords img(100, 100, 200, 200);
    NormalizedCoords norm = CoordinateSystem::imageToNormalized(img, 0, 0);
    assert(norm.x1 == 0.0 && norm.y1 == 0.0 && norm.x2 == 0.0 && norm.y2 == 0.0);
    
    double ratio = CoordinateSystem::calculateAspectRatio(1000, 0);
    assert(ratio == 0.0);
    std::cout << "    ✓ Zero dimensions handled\n";
    
    // Test 2: Very small image (1x1 pixel)
    std::cout << "  Testing very small image (1x1)...\n";
    NormalizedCoords smallNorm(0.5, 0.5, 1.0, 1.0);
    ImageCoords smallImg = CoordinateSystem::normalizedToImage(smallNorm, 1, 1);
    assert(smallImg.x1 == 0 || smallImg.x1 == 1); // Rounding may give 0 or 1
    assert(smallImg.y1 == 0 || smallImg.y1 == 1);
    std::cout << "    ✓ Very small image handled\n";
    
    // Test 3: Very large image (10K x 10K)
    std::cout << "  Testing very large image (10000x10000)...\n";
    NormalizedCoords largeNorm(0.25, 0.25, 0.75, 0.75);
    ImageCoords largeImg = CoordinateSystem::normalizedToImage(largeNorm, 10000, 10000);
    assert(largeImg.x1 == 2500);
    assert(largeImg.y1 == 2500);
    assert(largeImg.x2 == 7500);
    assert(largeImg.y2 == 7500);
    std::cout << "    ✓ Very large image handled\n";
    
    // Test 4: Coordinates at boundaries (0.0 and 1.0)
    std::cout << "  Testing boundary coordinates...\n";
    NormalizedCoords boundary(0.0, 0.0, 1.0, 1.0);
    ImageCoords boundaryImg = CoordinateSystem::normalizedToImage(boundary, 2000, 3000);
    assert(boundaryImg.x1 == 0);
    assert(boundaryImg.y1 == 0);
    assert(boundaryImg.x2 == 2000);
    assert(boundaryImg.y2 == 3000);
    std::cout << "    ✓ Boundary coordinates handled\n";
    
    // Test 5: Very small regions (near-zero size)
    std::cout << "  Testing very small regions...\n";
    NormalizedCoords tiny(0.5, 0.5, 0.5001, 0.5001);
    ImageCoords tinyImg = CoordinateSystem::normalizedToImage(tiny, 2000, 3000);
    // Should still produce valid coordinates (may round to same pixel)
    assert(CoordinateSystem::isValidImage(tinyImg, 2000, 3000) || 
           (tinyImg.x1 == tinyImg.x2 && tinyImg.y1 == tinyImg.y2));
    std::cout << "    ✓ Very small regions handled\n";
    
    // Test 6: Negative scale factor (should handle gracefully)
    std::cout << "  Testing negative scale factor...\n";
    ImageCoords testImg(100, 100, 200, 200);
    QPointF offset(0, 0);
    CanvasCoords canvas = CoordinateSystem::imageToCanvas(testImg, -0.5, offset);
    // Should still produce coordinates (may be negative, which is OK for canvas)
    assert(canvas.x1 < canvas.x2 || canvas.x1 > canvas.x2); // Order may be reversed
    std::cout << "    ✓ Negative scale factor handled\n";
    
    // Test 7: Zero scale factor (division by zero protection)
    std::cout << "  Testing zero scale factor...\n";
    ImageCoords zeroImg = CoordinateSystem::canvasToImage(CanvasCoords(100, 100, 200, 200), 0.0, offset);
    assert(zeroImg.x1 == 0 && zeroImg.y1 == 0 && zeroImg.x2 == 0 && zeroImg.y2 == 0);
    std::cout << "    ✓ Zero scale factor handled\n";
}

// ============================================================================
// ROUND-TRIP ACCURACY TESTS
// ============================================================================

void testRoundTripAccuracy() {
    std::cout << "\n=== Testing Round-Trip Accuracy ===\n";
    
    // Test 1: Normalized → Image → Normalized
    std::cout << "  Testing Normalized → Image → Normalized...\n";
    NormalizedCoords original(0.25, 0.10, 0.35, 0.20);
    ImageCoords img = CoordinateSystem::normalizedToImage(original, 2000, 3000);
    NormalizedCoords roundTrip = CoordinateSystem::imageToNormalized(img, 2000, 3000);
    
    assert(approxEqual(original.x1, roundTrip.x1, ROUND_TRIP_EPSILON));
    assert(approxEqual(original.y1, roundTrip.y1, ROUND_TRIP_EPSILON));
    assert(approxEqual(original.x2, roundTrip.x2, ROUND_TRIP_EPSILON));
    assert(approxEqual(original.y2, roundTrip.y2, ROUND_TRIP_EPSILON));
    std::cout << "    ✓ Round-trip accuracy acceptable\n";
    
    // Test 2: Image → Canvas → Image (with rounding tolerance)
    std::cout << "  Testing Image → Canvas → Image...\n";
    ImageCoords originalImg(500, 300, 700, 600);
    QPointF offset(100.0, 50.0);
    double scale = 0.4;
    CanvasCoords canvas = CoordinateSystem::imageToCanvas(originalImg, scale, offset);
    ImageCoords roundTripImg = CoordinateSystem::canvasToImage(canvas, scale, offset);
    
    assert(approxEqual(originalImg, roundTripImg, 1)); // Allow 1 pixel tolerance
    std::cout << "    ✓ Round-trip with rounding tolerance acceptable\n";
    
    // Test 3: Full cycle: Normalized → Image → Canvas → Image → Normalized
    std::cout << "  Testing full cycle transformation...\n";
    NormalizedCoords start(0.3, 0.15, 0.4, 0.25);
    ImageCoords img1 = CoordinateSystem::normalizedToImage(start, 2000, 3000);
    CanvasCoords canvas1 = CoordinateSystem::imageToCanvas(img1, scale, offset);
    ImageCoords img2 = CoordinateSystem::canvasToImage(canvas1, scale, offset);
    NormalizedCoords end = CoordinateSystem::imageToNormalized(img2, 2000, 3000);
    
    // Should be close (within rounding error)
    assert(approxEqual(start.x1, end.x1, ROUND_TRIP_EPSILON * 2));
    assert(approxEqual(start.y1, end.y1, ROUND_TRIP_EPSILON * 2));
    std::cout << "    ✓ Full cycle accuracy acceptable\n";
}

// ============================================================================
// PRECISION AND FLOATING POINT TESTS
// ============================================================================

void testPrecision() {
    std::cout << "\n=== Testing Precision ===\n";
    
    // Test 1: High precision normalized coordinates
    std::cout << "  Testing high precision coordinates...\n";
    NormalizedCoords precise(0.123456789, 0.987654321, 0.234567890, 0.876543210);
    ImageCoords img = CoordinateSystem::normalizedToImage(precise, 10000, 10000);
    NormalizedCoords back = CoordinateSystem::imageToNormalized(img, 10000, 10000);
    
    // Should maintain reasonable precision
    assert(approxEqual(precise.x1, back.x1, 0.0001)); // 0.01% tolerance
    std::cout << "    ✓ High precision maintained\n";
    
    // Test 2: Very small differences
    std::cout << "  Testing very small coordinate differences...\n";
    NormalizedCoords smallDiff(0.5, 0.5, 0.50001, 0.50001);
    ImageCoords smallImg = CoordinateSystem::normalizedToImage(smallDiff, 2000, 3000);
    // May round to same pixel, which is acceptable
    assert(CoordinateSystem::isValidImage(smallImg, 2000, 3000));
    std::cout << "    ✓ Small differences handled\n";
}

// ============================================================================
// INTEGRATION SCENARIOS (Preparing for Data Models)
// ============================================================================

void testIntegrationScenarios() {
    std::cout << "\n=== Testing Integration Scenarios ===\n";
    
    // Scenario 1: Region Creation Flow (Canvas → Image → Normalized)
    std::cout << "  Testing region creation flow...\n";
    CanvasCoords userInput(150.0, 200.0, 250.0, 300.0);
    QPointF offset(50.0, 100.0);
    double scale = 0.5;
    
    // Step 1: Canvas → Image
    ImageCoords img = CoordinateSystem::canvasToImage(userInput, scale, offset);
    assert(img.x1 >= 0 && img.y1 >= 0);
    
    // Step 2: Image → Normalized
    NormalizedCoords norm = CoordinateSystem::imageToNormalized(img, 2000, 3000);
    assert(CoordinateSystem::isValidNormalized(norm));
    
    // Step 3: Verify we can go back
    ImageCoords verifyImg = CoordinateSystem::normalizedToImage(norm, 2000, 3000);
    assert(approxEqual(img, verifyImg, 1)); // Allow rounding
    std::cout << "    ✓ Region creation flow works\n";
    
    // Scenario 2: Zoom Change Flow (Normalized → Canvas recalculation)
    std::cout << "  Testing zoom change flow...\n";
    NormalizedCoords stored(0.25, 0.10, 0.35, 0.20);
    
    // Old zoom
    double oldScale = 0.4;
    CanvasCoords oldCanvas = CoordinateSystem::normalizedToCanvas(stored, 2000, 3000, oldScale, offset);
    
    // New zoom
    double newScale = 0.8; // Zoom in
    CanvasCoords newCanvas = CoordinateSystem::normalizedToCanvas(stored, 2000, 3000, newScale, offset);
    
    // Canvas coordinates should change proportionally
    assert(std::abs(newCanvas.x1 - offset.x()) > std::abs(oldCanvas.x1 - offset.x()));
    std::cout << "    ✓ Zoom change flow works\n";
    
    // Scenario 3: Region Movement (Delta calculation)
    std::cout << "  Testing region movement delta...\n";
    CanvasCoords start(100.0, 100.0, 200.0, 200.0);
    CanvasCoords end(150.0, 120.0, 250.0, 220.0);
    
    // Calculate delta in canvas space
    double deltaX = end.x1 - start.x1;
    double deltaY = end.y1 - start.y1;
    
    // Convert delta to image space
    double imageDeltaX = deltaX / scale;
    double imageDeltaY = deltaY / scale;
    
    // Convert delta to normalized space
    double normDeltaX = imageDeltaX / 2000.0;
    double normDeltaY = imageDeltaY / 3000.0;
    
    // Verify by transforming start and end
    ImageCoords startImg = CoordinateSystem::canvasToImage(start, scale, offset);
    ImageCoords endImg = CoordinateSystem::canvasToImage(end, scale, offset);
    assert(std::abs((endImg.x1 - startImg.x1) - imageDeltaX) < 1);
    std::cout << "    ✓ Region movement delta calculation works\n";
}

// ============================================================================
// QT TYPE INTEGRATION TESTS
// ============================================================================

void testQtIntegration() {
    std::cout << "\n=== Testing Qt Type Integration ===\n";
    
    // Test 1: QPointF offset usage
    std::cout << "  Testing QPointF offset...\n";
    ImageCoords img(100, 100, 200, 200);
    QPointF offset1(50.0, 75.0);
    QPointF offset2(-25.0, -50.0);
    
    CanvasCoords canvas1 = CoordinateSystem::imageToCanvas(img, 1.0, offset1);
    CanvasCoords canvas2 = CoordinateSystem::imageToCanvas(img, 1.0, offset2);
    
    assert(approxEqual(canvas1.x1 - canvas2.x1, 75.0)); // 50 - (-25)
    assert(approxEqual(canvas1.y1 - canvas2.y1, 125.0)); // 75 - (-50)
    std::cout << "    ✓ QPointF offset works correctly\n";
    
    // Test 2: Conversion to QRectF (for future use in Canvas)
    std::cout << "  Testing QRectF conversion compatibility...\n";
    CanvasCoords canvas(100.0, 200.0, 300.0, 400.0);
    QRectF qrect(canvas.x1, canvas.y1, canvas.x2 - canvas.x1, canvas.y2 - canvas.y1);
    
    assert(approxEqual(qrect.x(), canvas.x1));
    assert(approxEqual(qrect.y(), canvas.y1));
    assert(approxEqual(qrect.width(), canvas.x2 - canvas.x1));
    assert(approxEqual(qrect.height(), canvas.y2 - canvas.y1));
    std::cout << "    ✓ QRectF conversion compatible\n";
}

// ============================================================================
// PERFORMANCE TESTS (Stress Testing)
// ============================================================================

void testPerformance() {
    std::cout << "\n=== Testing Performance (Stress Test) ===\n";
    
    // Test: Transform many coordinates (simulating 100 regions)
    std::cout << "  Testing batch transformations (100 regions)...\n";
    const int numRegions = 100;
    NormalizedCoords regions[numRegions];
    
    // Create test data
    for (int i = 0; i < numRegions; i++) {
        double x = 0.1 + (i % 10) * 0.08;
        double y = 0.1 + (i / 10) * 0.08;
        regions[i] = NormalizedCoords(x, y, x + 0.05, y + 0.05);
    }
    
    // Transform all to canvas (simulating zoom change)
    QPointF offset(100.0, 50.0);
    double scale = 0.5;
    int validCount = 0;
    
    for (int i = 0; i < numRegions; i++) {
        CanvasCoords canvas = CoordinateSystem::normalizedToCanvas(
            regions[i], 2000, 3000, scale, offset);
        if (CoordinateSystem::isValidCanvas(canvas)) {
            validCount++;
        }
    }
    
    assert(validCount == numRegions);
    std::cout << "    ✓ Batch transformation of " << numRegions << " regions successful\n";
}

// ============================================================================
// VALIDATION EDGE CASES
// ============================================================================

void testValidationEdgeCases() {
    std::cout << "\n=== Testing Validation Edge Cases ===\n";
    
    // Test 1: Exactly at boundaries
    std::cout << "  Testing boundary validation...\n";
    NormalizedCoords atBoundary(0.0, 0.0, 1.0, 1.0);
    assert(CoordinateSystem::isValidNormalized(atBoundary));
    
    NormalizedCoords justOver(0.0, 0.0, 1.0001, 1.0);
    assert(!CoordinateSystem::isValidNormalized(justOver));
    std::cout << "    ✓ Boundary validation works\n";
    
    // Test 2: Equal coordinates (x1 == x2 or y1 == y2)
    std::cout << "  Testing equal coordinate validation...\n";
    NormalizedCoords equalX(0.25, 0.10, 0.25, 0.20); // x1 == x2
    assert(!CoordinateSystem::isValidNormalized(equalX));
    
    NormalizedCoords equalY(0.25, 0.10, 0.35, 0.10); // y1 == y2
    assert(!CoordinateSystem::isValidNormalized(equalY));
    std::cout << "    ✓ Equal coordinate validation works\n";
    
    // Test 3: Clamp fixes invalid coordinates
    std::cout << "  Testing clamp fixes invalid coordinates...\n";
    NormalizedCoords invalid(1.5, -0.1, 0.35, 0.20);
    CoordinateSystem::clampToBounds(invalid);
    assert(CoordinateSystem::isValidNormalized(invalid));
    std::cout << "    ✓ Clamp fixes invalid coordinates\n";
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

int main() {
    std::cout << "========================================\n";
    std::cout << "CoordinateSystem Comprehensive Test Suite\n";
    std::cout << "========================================\n";
    
    try {
        testEdgeCases();
        testRoundTripAccuracy();
        testPrecision();
        testIntegrationScenarios();
        testQtIntegration();
        testPerformance();
        testValidationEdgeCases();
        
        std::cout << "\n========================================\n";
        std::cout << "✓ ALL TESTS PASSED!\n";
        std::cout << "========================================\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n✗ TEST FAILED: " << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "\n✗ UNKNOWN TEST FAILURE\n";
        return 1;
    }
}


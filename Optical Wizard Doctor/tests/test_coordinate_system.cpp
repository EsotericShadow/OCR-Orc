// Test file for CoordinateSystem
// This is a placeholder test file - full test framework will be added later

#include "../src/core/CoordinateSystem.h"
#include <QtCore/QPointF>
#include <iostream>
#include <cassert>

using namespace ocr_orc;

// Simple test runner (will be replaced with proper test framework later)
void testNormalizedToImage() {
    std::cout << "Testing normalizedToImage...\n";
    
    // Test case: Normalized (0.25, 0.10, 0.35, 0.20) → Image (500, 300, 700, 600) for 2000x3000 image
    NormalizedCoords norm(0.25, 0.10, 0.35, 0.20);
    ImageCoords img = CoordinateSystem::normalizedToImage(norm, 2000, 3000);
    
    assert(img.x1 == 500);
    assert(img.y1 == 300);
    assert(img.x2 == 700);
    assert(img.y2 == 600);
    
    std::cout << "  ✓ normalizedToImage passed\n";
}

void testImageToNormalized() {
    std::cout << "Testing imageToNormalized...\n";
    
    // Test case: Image (500, 300, 700, 600) → Normalized (0.25, 0.10, 0.35, 0.20) for 2000x3000 image
    ImageCoords img(500, 300, 700, 600);
    NormalizedCoords norm = CoordinateSystem::imageToNormalized(img, 2000, 3000);
    
    const double epsilon = 0.0001;
    assert(std::abs(norm.x1 - 0.25) < epsilon);
    assert(std::abs(norm.y1 - 0.10) < epsilon);
    assert(std::abs(norm.x2 - 0.35) < epsilon);
    assert(std::abs(norm.y2 - 0.20) < epsilon);
    
    std::cout << "  ✓ imageToNormalized passed\n";
}

void testImageToCanvas() {
    std::cout << "Testing imageToCanvas...\n";
    
    // Test case: Image → Canvas with scale factor 0.4 and offset (100, 50)
    ImageCoords img(500, 300, 700, 600);
    QPointF offset(100.0, 50.0);
    CanvasCoords canvas = CoordinateSystem::imageToCanvas(img, 0.4, offset);
    
    const double epsilon = 0.0001;
    assert(std::abs(canvas.x1 - 300.0) < epsilon);  // 500 * 0.4 + 100
    assert(std::abs(canvas.y1 - 170.0) < epsilon); // 300 * 0.4 + 50
    assert(std::abs(canvas.x2 - 380.0) < epsilon); // 700 * 0.4 + 100
    assert(std::abs(canvas.y2 - 290.0) < epsilon); // 600 * 0.4 + 50
    
    std::cout << "  ✓ imageToCanvas passed\n";
}

void testCanvasToImage() {
    std::cout << "Testing canvasToImage...\n";
    
    // Test case: Canvas → Image (reverse of above)
    CanvasCoords canvas(300.0, 170.0, 380.0, 290.0);
    QPointF offset(100.0, 50.0);
    ImageCoords img = CoordinateSystem::canvasToImage(canvas, 0.4, offset);
    
    // Allow for rounding differences
    assert(std::abs(img.x1 - 500) <= 1);
    assert(std::abs(img.y1 - 300) <= 1);
    assert(std::abs(img.x2 - 700) <= 1);
    assert(std::abs(img.y2 - 600) <= 1);
    
    std::cout << "  ✓ canvasToImage passed\n";
}

void testNormalizedToCanvas() {
    std::cout << "Testing normalizedToCanvas...\n";
    
    NormalizedCoords norm(0.25, 0.10, 0.35, 0.20);
    QPointF offset(100.0, 50.0);
    CanvasCoords canvas = CoordinateSystem::normalizedToCanvas(norm, 2000, 3000, 0.4, offset);
    
    const double epsilon = 0.0001;
    assert(std::abs(canvas.x1 - 300.0) < epsilon);
    assert(std::abs(canvas.y1 - 170.0) < epsilon);
    assert(std::abs(canvas.x2 - 380.0) < epsilon);
    assert(std::abs(canvas.y2 - 290.0) < epsilon);
    
    std::cout << "  ✓ normalizedToCanvas passed\n";
}

void testValidation() {
    std::cout << "Testing validation functions...\n";
    
    // Valid normalized coordinates
    NormalizedCoords validNorm(0.25, 0.10, 0.35, 0.20);
    assert(CoordinateSystem::isValidNormalized(validNorm));
    
    // Invalid normalized coordinates (out of range)
    NormalizedCoords invalidNorm(1.5, 0.10, 0.35, 0.20);
    assert(!CoordinateSystem::isValidNormalized(invalidNorm));
    
    // Invalid normalized coordinates (wrong order)
    NormalizedCoords wrongOrder(0.35, 0.10, 0.25, 0.20);
    assert(!CoordinateSystem::isValidNormalized(wrongOrder));
    
    // Valid image coordinates
    ImageCoords validImg(500, 300, 700, 600);
    assert(CoordinateSystem::isValidImage(validImg, 2000, 3000));
    
    // Invalid image coordinates (out of bounds)
    ImageCoords invalidImg(2500, 300, 700, 600);
    assert(!CoordinateSystem::isValidImage(invalidImg, 2000, 3000));
    
    // Valid canvas coordinates
    CanvasCoords validCanvas(100.0, 200.0, 300.0, 400.0);
    assert(CoordinateSystem::isValidCanvas(validCanvas));
    
    // Invalid canvas coordinates (wrong order)
    CanvasCoords invalidCanvas(300.0, 200.0, 100.0, 400.0);
    assert(!CoordinateSystem::isValidCanvas(invalidCanvas));
    
    std::cout << "  ✓ validation functions passed\n";
}

void testClampAndNormalize() {
    std::cout << "Testing clampToBounds and normalizeCoords...\n";
    
    // Test clamping
    NormalizedCoords outOfBounds(1.5, -0.1, 0.35, 0.20);
    CoordinateSystem::clampToBounds(outOfBounds);
    assert(outOfBounds.x1 <= 1.0 && outOfBounds.x1 >= 0.0);
    assert(outOfBounds.y1 <= 1.0 && outOfBounds.y1 >= 0.0);
    
    // Test normalization (swap if needed)
    NormalizedCoords swapped(0.35, 0.20, 0.25, 0.10);
    CoordinateSystem::normalizeCoords(swapped);
    assert(swapped.x1 < swapped.x2);
    assert(swapped.y1 < swapped.y2);
    
    std::cout << "  ✓ clampToBounds and normalizeCoords passed\n";
}

void testAspectRatio() {
    std::cout << "Testing calculateAspectRatio...\n";
    
    double ratio = CoordinateSystem::calculateAspectRatio(2000, 3000);
    const double epsilon = 0.0001;
    assert(std::abs(ratio - 0.6666667) < epsilon);
    
    // Test division by zero
    double zeroRatio = CoordinateSystem::calculateAspectRatio(2000, 0);
    assert(zeroRatio == 0.0);
    
    std::cout << "  ✓ calculateAspectRatio passed\n";
}

int main() {
    std::cout << "Running CoordinateSystem tests...\n\n";
    
    try {
        testNormalizedToImage();
        testImageToNormalized();
        testImageToCanvas();
        testCanvasToImage();
        testNormalizedToCanvas();
        testValidation();
        testClampAndNormalize();
        testAspectRatio();
        
        std::cout << "\n✓ All tests passed!\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n✗ Test failed: " << e.what() << "\n";
        return 1;
    }
}


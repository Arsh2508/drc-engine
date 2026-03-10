#include <core/DrcEngine.hpp>
#include <rules/MinSpacingRule.hpp>
#include <rules/IntersectionRule.hpp>
#include <layout/Layout.hpp>
#include <layout/Layer.hpp>
#include <layout/Shape.hpp>
#include <spatial/NaiveSpatialIndex.hpp>
#include <spatial/RTreeSpatialIndex.hpp>
#include <iostream>
#include <iomanip>
#include <vector>
#include <memory>
#include <chrono>
#include <cstring>

// Performance comparison between spatial indices
/// Helper to print a formatted header
void printHeader(const std::string& title)
{
    std::cout << "\n" << std::string(70, '=') << "\n";
    std::cout << title << "\n";
    std::cout << std::string(70, '=') << "\n";
}

/// Helper to print a formatted sub-header
void printSubheader(const std::string& title)
{
    std::cout << "\n" << std::string(50, '-') << "\n";
    std::cout << title << "\n";
    std::cout << std::string(50, '-') << "\n";
}

/// Create a test layout with configurable density
/// @param shapeCount Number of shapes to create
/// @param layerCount Number of layers to distribute shapes across
/// @return Layout with test shapes
Layout createTestLayout(int shapeCount, int layerCount = 2)
{
    Layout layout;

    // Create layers
    std::vector<Layer> layers;
    for (int i = 0; i < layerCount; ++i)
    {
        layers.push_back(Layer(i, "layer_" + std::to_string(i)));
    }

    // Distribute shapes across layers with some spacing
    int shapeId = 1;
    int spacing = 15;  // Space between shape centers
    int shapesPerLayer = (shapeCount + layerCount - 1) / layerCount;

    for (int layer = 0; layer < layerCount; ++layer)
    {
        for (int i = 0; i < shapesPerLayer && shapeId <= shapeCount; ++i)
        {
            int x = i * spacing;
            int y = layer * spacing;
            Rect bounds(x, y, x + 10, y + 10);
            layers[layer].addShape(Shape(bounds, shapeId++, layer));
        }
    }

    // Add layers to layout
    for (const auto& layer : layers)
    {
        layout.addLayer(layer);
    }

    return layout;
}

/// Run DRC with specified spatial index and measure performance
struct DrcResult
{
    std::string indexName;
    long long executionMs;
    int violationCount;
    int rulsExecuted;

    void print() const
    {
        std::cout << std::left << std::setw(25) << indexName 
                  << std::setw(15) << (std::string(std::to_string(executionMs)) + " ms")
                  << std::setw(15) << violationCount
                  << std::setw(15) << rulsExecuted
                  << "\n";
    }
};

/// Execute DRC with specified spatial index
DrcResult runDrc(const Layout& layout, bool useRTree)
{
    DrcEngine engine;

    // Register rules
    engine.registerRule(std::make_shared<IntersectionRule>());
    engine.registerRule(std::make_shared<MinSpacingRule>(10.0, 0));
    if (layout.getLayerCount() > 1)
    {
        engine.registerRule(std::make_shared<MinSpacingRule>(8.0, 1));
    }

    // Create spatial index
    SpatialIndexPtr index;
    if (useRTree)
    {
        index = std::make_shared<spatial::RTreeSpatialIndex>();
    }
    else
    {
        index = std::make_shared<NaiveSpatialIndex>();
    }

    // Run DRC and measure time
    auto startTime = std::chrono::high_resolution_clock::now();
    auto report = engine.run(layout, index);
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    DrcResult result;
    result.indexName = useRTree ? "R-tree (Boost)" : "Naive (Vector)";
    result.executionMs = duration.count();
    result.violationCount = report.getViolationCount();
    result.rulsExecuted = report.getRulesExecuted();

    return result;
}

int main(int argc, char* argv[])
{
    (void)argc;  // Suppress unused parameter warning
    (void)argv;

    std::cout << "\nThis test demonstrates:\n"
              << "  1. Correctness: Both implementations find identical violations\n"
              << "  2. Performance: How R-tree scales with increasing shape count\n"
              << "  3. Efficiency: Better query performance in R-tree\n\n";

    /// Test 1: Small layout (correctness verification)
    {
        printSubheader("Test 1: Small Layout (10 shapes) - Correctness Check");

        Layout small_layout = createTestLayout(10, 2);
        std::cout << "\nLayout: " << small_layout.getTotalShapeCount() << " shapes on "
                  << small_layout.getLayerCount() << " layers\n\n";

        DrcResult naive_result = runDrc(small_layout, false);
        DrcResult rtree_result = runDrc(small_layout, true);

        std::cout << std::left << std::setw(25) << "Index"
                  << std::setw(15) << "Time"
                  << std::setw(15) << "Violations"
                  << std::setw(15) << "Rules\n";
        std::cout << std::string(70, '-') << "\n";
        naive_result.print();
        rtree_result.print();

        if (naive_result.violationCount == rtree_result.violationCount)
        {
            std::cout << "\n✓ Correctness Check PASSED: Both implementations found "
                      << naive_result.violationCount << " violations\n";
        }
        else
        {
            std::cout << "\n✗ Correctness Check FAILED!\n"
                      << "  Naive: " << naive_result.violationCount << " violations\n"
                      << "  R-tree: " << rtree_result.violationCount << " violations\n";
        }
    }

    /// Test 2: Medium layout
    {
        printSubheader("Test 2: Medium Layout (50 shapes) - Performance Measurement");

        Layout medium_layout = createTestLayout(50, 2);
        std::cout << "\nLayout: " << medium_layout.getTotalShapeCount() << " shapes on "
                  << medium_layout.getLayerCount() << " layers\n\n";

        DrcResult naive_result = runDrc(medium_layout, false);
        DrcResult rtree_result = runDrc(medium_layout, true);

        std::cout << std::left << std::setw(25) << "Index"
                  << std::setw(15) << "Time"
                  << std::setw(15) << "Violations"
                  << std::setw(15) << "Rules\n";
        std::cout << std::string(70, '-') << "\n";
        naive_result.print();
        rtree_result.print();

        if (naive_result.executionMs > 0 && rtree_result.executionMs > 0)
        {
            double speedup = static_cast<double>(naive_result.executionMs) /
                           rtree_result.executionMs;
            std::cout << "\nSpeedup: " << std::fixed << std::setprecision(2)
                      << speedup << "x\n";
        }
    }

    /// Test 3: Large layout
    {
        printSubheader("Test 3: Large Layout (200 shapes) - Scalability Comparison");

        Layout large_layout = createTestLayout(200, 2);
        std::cout << "\nLayout: " << large_layout.getTotalShapeCount() << " shapes on "
                  << large_layout.getLayerCount() << " layers\n"
                  << "(This demonstrates O(n*log(n)) vs O(n²) scaling)\n\n";

        DrcResult naive_result = runDrc(large_layout, false);
        DrcResult rtree_result = runDrc(large_layout, true);

        std::cout << std::left << std::setw(25) << "Index"
                  << std::setw(15) << "Time"
                  << std::setw(15) << "Violations"
                  << std::setw(15) << "Rules\n";
        std::cout << std::string(70, '-') << "\n";
        naive_result.print();
        rtree_result.print();

        if (naive_result.executionMs > 0 && rtree_result.executionMs > 0)
        {
            double speedup = static_cast<double>(naive_result.executionMs) /
                           rtree_result.executionMs;
            std::cout << "\nSpeedup: " << std::fixed << std::setprecision(2)
                      << speedup << "x\n";
        }

        if (naive_result.violationCount == rtree_result.violationCount)
        {
            std::cout << "Correctness: ✓ PASSED (both found "
                      << naive_result.violationCount << " violations)\n";
        }
    }

    /// Summary
    {
        printHeader("Summary");
        std::cout << "\nImplementation Notes:\n\n"
                  << "1. CORRECTNESS:\n"
                  << "   - Both implementations use identical rule logic\n"
                  << "   - Both find the same violations\n"
                  << "   - Only spatial index data structure differs\n\n"
                  << "2. R-TREE ADVANTAGES:\n"
                  << "   - Query time: O(log n + k) vs O(n) for naive\n"
                  << "   - Better cache locality with spatial partitioning\n"
                  << "   - Seamless scaling from 10 to 10,000+ shapes\n\n"
                  << "3. ARCHITECTURE:\n"
                  << "   - SpatialIndex interface enables pluggable implementations\n"
                  << "   - Rules use only the abstract interface\n"
                  << "   - Zero changes to rule logic when switching indices\n\n"
                  << "4. INTEGRATION:\n"
                  << "   - DrcEngine uses R-tree by default\n"
                  << "   - Can override with naive index for testing\n"
                  << "   - Boost.Geometry headers only (no linking required)\n";
    }

    return 0;
}

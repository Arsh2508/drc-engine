#pragma once

#include "SpatialIndex.hpp"
#include <geometry/GeometryUtils.hpp>
#include <algorithm>

/// @brief Naive spatial index using a simple vector.
///
/// This implementation stores all shapes in a linear vector and performs
/// brute-force O(n) queries by checking all shapes against the query rectangle.
///
/// Purpose:
/// - Phase 1 of DRC development: focus on correctness, not optimization
/// - Zero external dependencies (no Boost required yet)
/// - Baseline for measuring performance improvements with R-tree
/// - Suitable for thesis work with small to medium designs
///
/// Performance Characteristics:
/// - insert(): O(1) amortized (vector push_back)
/// - query(): O(n) - checks every shape
/// - insertBatch(): O(n) - no optimization
/// - getAllShapes(): O(1) return time
///
/// Scalability:
/// - Suitable for designs with hundreds to low thousands of shapes
/// - QueryTime = O(n*m) where n=shapes, m=query rectangles
/// - Total DRC time: O(rules * n²) due to pairwise checking
///
/// Future Replacement with Boost R-tree:
/// - Drop-in replacement: RtreeSpatialIndex implements SpatialIndex
/// - Same interface, dramatically better performance: O(log n + k)
/// - No code changes in DrcEngine or DrcRule implementations
///
class NaiveSpatialIndex : public SpatialIndex
{
public:
    /// @brief Construct empty naive index.
    NaiveSpatialIndex() = default;

    /// @brief Insert a shape into the vector.
    /// @param shape Shape to insert
    void insert(const Shape& shape) override;

    /// @brief Query for shapes overlapping the rectangle using brute-force.
    /// @param queryRect Rectangle to query
    /// @return Vector of shapes overlapping queryRect
    std::vector<Shape> query(const Rect& queryRect) const override;

    /// @brief Get all indexed shapes.
    /// @return All shapes in the index
    std::vector<Shape> getAllShapes() const override;

    /// @brief Get shape count.
    /// @return Number of shapes in index
    size_t getShapeCount() const override;

    /// @brief Clear the index.
    void clear() override;

    /// @brief Get bounding box of all indexed shapes.
    /// @return Bounding rectangle encompassing all shapes
    Rect getBounds() const override;

private:
    std::vector<Shape> m_shapes; ///< All shapes stored linearly
};

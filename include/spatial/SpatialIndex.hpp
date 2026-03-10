#pragma once

#include <layout/Shape.hpp>
#include <vector>
#include <memory>

/// @brief Abstract interface for spatial indexing in DRC systems.
///
/// This abstraction decouples DRC rule logic from the spatial indexing implementation.
/// By defining a common interface, we support two key design goals:
///
/// 1. Initial Phase: Use naive O(n²) vector-based method
///    - Simple, no external dependencies
///    - Suitable for thesis work focusing on DRC algorithm design
///
/// 2. Future Phase: Replace with Boost.Geometry R-tree
///    - O(log n) query performance
///    - Drop-in replacement without modifying DRC rules or engine
///    - Enables thesis expansion into spatial optimization
///
/// The interface defines two core operations:
/// - insert(): Add shapes and maintain spatial structure
/// - query(): Return shapes that overlap a query rectangle
///
/// Key design decisions:
/// - No template parameters to keep implementation simple
/// - Virtual functions allow polymorphic behavior
/// - Rule logic depends only on this interface, not concrete types
///
/// Future Boost R-tree integration:
/// ================================
/// When replacing with boost::geometry::index::rtree<Shape, Parameters>:
/// 1. Create new class: class BoostedSpatialIndex : public SpatialIndex
/// 2. Internally wrap: rtree<std::pair<Rect, Shape>> m_index
/// 3. insert() -> calls m_index.insert(...)
/// 4. query() -> calls m_index.query(index::intersects(queryRect), ...)
/// 5. No changes needed to DrcEngine or DrcRule subclasses
///
class SpatialIndex
{
public:
    virtual ~SpatialIndex() = default;

    /// @brief Insert a shape into the spatial index.
    /// The index maintains the spatial structure for efficient querying.
    /// Time complexity: O(1) for naive, O(log n) for R-tree
    /// @param shape Shape to insert
    virtual void insert(const Shape& shape) = 0;

    /// @brief Insert multiple shapes into the spatial index.
    /// Default implementation calls insert() for each shape.
    /// Subclasses may override for batch optimization (e.g., bulk-loading in R-trees).
    /// @param shapes Vector of shapes to insert
    virtual void insertBatch(const std::vector<Shape>& shapes)
    {
        for (const auto& shape : shapes)
        {
            insert(shape);
        }
    }

    /// @brief Query for all shapes that overlap the given rectangle.
    /// Time complexity: O(n) for naive, O(log n + k) for R-tree (k = results)
    /// @param queryRect Query rectangle for spatial search
    /// @return Vector of shapes that overlap queryRect
    virtual std::vector<Shape> query(const Rect& queryRect) const = 0;

    /// @brief Get all indexed shapes (useful for iteration, debugging).
    /// Note: Order and performance depends on implementation.
    /// Time complexity: O(n)
    /// @return Vector of all shapes in the index
    virtual std::vector<Shape> getAllShapes() const = 0;

    /// @brief Get the number of indexed shapes.
    /// Time complexity: O(1) if implemented efficiently
    /// @return Number of shapes in the index
    virtual size_t getShapeCount() const = 0;

    /// @brief Clear all shapes from the index.
    /// Time complexity: O(1) amortized
    virtual void clear() = 0;

    /// @brief Get bounding box of indexed region.
    /// Only meaningful when index is non-empty.
    /// @return Bounding rectangle of all indexed shapes
    virtual Rect getBounds() const = 0;
};

/// @brief Shared pointer for spatial index.
/// Used throughout DRC system to maintain abstraction over concrete type.
using SpatialIndexPtr = std::shared_ptr<SpatialIndex>;

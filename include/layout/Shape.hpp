#pragma once

#include <geometry/Rect.hpp>
#include <string>

/// @brief Represents a geometric shape in the layout with associated metadata.
/// Wraps a bounding rectangle with a unique identifier and layer information.
/// This is the fundamental unit of a layout that DRC checks operate on.
class Shape
{
public:
    using ShapeId = int;

    /// @brief Construct a shape from a rectangle, ID, and layer.
    /// @param rect Bounding box of the shape
    /// @param id Unique identifier for this shape
    /// @param layer Layer number (e.g., 0 for metal1, 1 for metal2, etc.)
    Shape(const Rect& rect, ShapeId id, int layer)
        : m_rect(rect), m_id(id), m_layer(layer)
    {
    }

    /// @brief Get the bounding rectangle of this shape.
    const Rect& getBounds() const { return m_rect; }

    /// @brief Get the unique identifier of this shape.
    ShapeId getId() const { return m_id; }

    /// @brief Get the layer number of this shape.
    int getLayer() const { return m_layer; }

    /// @brief Get a human-readable name for this shape.
    std::string getName() const
    {
        return "Shape_" + std::to_string(m_id) + "_L" + std::to_string(m_layer);
    }

    /// @brief Check equality based on ID and bounding box.
    bool operator==(const Shape& other) const
    {
        return m_id == other.m_id && m_rect == other.m_rect;
    }

    /// @brief Check inequality.
    bool operator!=(const Shape& other) const
    {
        return !(*this == other);
    }

private:
    Rect m_rect;      ///< Bounding box in layout coordinates
    ShapeId m_id;     ///< Unique shape identifier
    int m_layer;      ///< Layer number (physical layer in process)
};

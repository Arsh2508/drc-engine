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

    Shape(const Rect& rect, ShapeId id, int layer)
        : m_rect(rect), m_id(id), m_layer(layer)
    {
    }
    const Rect& getBounds() const { return m_rect; }

    ShapeId getId() const { return m_id; }

    int getLayer() const { return m_layer; }

    std::string getName() const
    {
        return "Shape_" + std::to_string(m_id) + "_L" + std::to_string(m_layer);
    }

    bool operator==(const Shape& other) const
    {
        return m_id == other.m_id && m_rect == other.m_rect;
    }

    bool operator!=(const Shape& other) const
    {
        return !(*this == other);
    }

private:
    Rect m_rect;      
    ShapeId m_id;     
    int m_layer;      
};

#pragma once

#include <geometry/Rect.hpp>
#include <layout/Polygon.hpp>
#include <string>
#include <vector>

/// @brief Represents a geometric shape in the layout with associated metadata.
/// Supports both axis-aligned rectangles and polygonal geometry such as trapezoids.
class Shape
{
public:
    using ShapeId = int;

    Shape(const Rect& rect, ShapeId id, int layer)
        : m_type(ShapeType::Rectangle), m_rect(rect), m_points(), m_id(id), m_layer(layer)
    {
    }

    Shape(const Polygon& polygon, ShapeId id, int layer)
        : m_type(polygon.getType()), m_rect(polygon.getBoundingRect()),
          m_points(polygon.getPoints()), m_id(id), m_layer(layer)
    {
    }

    Shape(ShapeType type,
          const std::vector<Point>& points,
          ShapeId id,
          int layer)
        : m_type(type), m_rect(boundingRectForPoints(points)),
          m_points(points), m_id(id), m_layer(layer)
    {
    }

    ShapeType getType() const { return m_type; }

    const Rect& getBounds() const { return m_rect; }

    const std::vector<Point>& getPoints() const { return m_points; }

    bool hasPoints() const { return !m_points.empty(); }

    long long getArea() const
    {
        if (m_type == ShapeType::Rectangle)
            return m_rect.area();
        return std::llabs(signedPolygonArea(m_points));
    }

    ShapeId getId() const { return m_id; }

    int getLayer() const { return m_layer; }

    std::string getName() const
    {
        return "Shape_" + std::to_string(m_id) + "_L" + std::to_string(m_layer);
    }

    bool operator==(const Shape& other) const
    {
        return m_id == other.m_id && m_layer == other.m_layer &&
               m_type == other.m_type && m_rect == other.m_rect &&
               m_points == other.m_points;
    }

    bool operator!=(const Shape& other) const
    {
        return !(*this == other);
    }

private:
    ShapeType m_type = ShapeType::Rectangle;
    Rect m_rect;                      
    std::vector<Point> m_points;      
    ShapeId m_id;                     
    int m_layer;                      
};

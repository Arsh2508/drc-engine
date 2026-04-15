#pragma once

#include <geometry/Point.hpp>
#include <geometry/Rect.hpp>
#include <vector>
#include <stdexcept>

/// @brief Supported polygon shape categories for layout geometry.
enum class ShapeType
{
    Rectangle,
    Trapezoid,
    Parallelogram,
    Polygon
};

inline long long signedPolygonArea(const std::vector<Point>& points)
{
    long long sum = 0;
    for (size_t i = 0; i < points.size(); ++i)
    {
        const Point& a = points[i];
        const Point& b = points[(i + 1) % points.size()];
        sum += static_cast<long long>(a.x) * b.y - static_cast<long long>(a.y) * b.x;
    }
    return sum;
}

inline Rect boundingRectForPoints(const std::vector<Point>& points)
{
    if (points.empty())
        return Rect();

    int minX = points[0].x;
    int minY = points[0].y;
    int maxX = points[0].x;
    int maxY = points[0].y;

    for (const auto& p : points)
    {
        minX = std::min(minX, p.x);
        minY = std::min(minY, p.y);
        maxX = std::max(maxX, p.x);
        maxY = std::max(maxY, p.y);
    }
    return Rect(minX, minY, maxX, maxY);
}

inline bool areEdgesParallel(const Point& a, const Point& b,
                             const Point& c, const Point& d)
{
    long long dx1 = static_cast<long long>(b.x) - a.x;
    long long dy1 = static_cast<long long>(b.y) - a.y;
    long long dx2 = static_cast<long long>(d.x) - c.x;
    long long dy2 = static_cast<long long>(d.y) - c.y;
    return dx1 * dy2 == dy1 * dx2;
}

inline bool hasDuplicatePoints(const std::vector<Point>& points)
{
    for (size_t i = 0; i < points.size(); ++i)
    {
        for (size_t j = i + 1; j < points.size(); ++j)
        {
            if (points[i] == points[j])
                return true;
        }
    }
    return false;
}

/// @brief Generic polygon geometry used by layout shapes.
class Polygon
{
public:
    Polygon(const std::vector<Point>& points, ShapeType type = ShapeType::Polygon)
        : m_points(points), m_type(type)
    {
        validate();
    }

    const std::vector<Point>& getPoints() const { return m_points; }
    ShapeType getType() const { return m_type; }

    Rect getBoundingRect() const
    {
        return boundingRectForPoints(m_points);
    }

    long long getArea() const
    {
        return std::llabs(signedPolygonArea(m_points));
    }

protected:
    void validate() const
    {
        if (m_points.size() < 3)
            throw std::runtime_error("Polygon must contain at least 3 points");
        if (hasDuplicatePoints(m_points))
            throw std::runtime_error("Polygon contains duplicate points");
        if (signedPolygonArea(m_points) == 0)
            throw std::runtime_error("Polygon must have non-zero area");

        if (m_type == ShapeType::Trapezoid)
            validateTrapezoid();
        else if (m_type == ShapeType::Parallelogram)
            validateParallelogram();
    }

private:
    void validateTrapezoid() const
    {
        if (m_points.size() != 4)
            throw std::runtime_error("Trapezoid must have exactly 4 points");

        bool parallel1 = areEdgesParallel(m_points[0], m_points[1], m_points[2], m_points[3]);
        bool parallel2 = areEdgesParallel(m_points[1], m_points[2], m_points[3], m_points[0]);
        if (!parallel1 && !parallel2)
        {
            throw std::runtime_error(
                "Trapezoid points must form a quadrilateral with at least one pair of opposite parallel edges");
        }
    }

    void validateParallelogram() const
    {
        if (m_points.size() != 4)
            throw std::runtime_error("Parallelogram must have exactly 4 points");

        bool opposite1 = areEdgesParallel(m_points[0], m_points[1], m_points[2], m_points[3]);
        bool opposite2 = areEdgesParallel(m_points[1], m_points[2], m_points[3], m_points[0]);
        if (!opposite1 || !opposite2)
        {
            throw std::runtime_error(
                "Parallelogram points must form a quadrilateral with both pairs of opposite edges parallel");
        }
    }

    std::vector<Point> m_points;
    ShapeType m_type;
};

/// @brief Explicit trapezoid geometry for layout support.
class Trapezoid : public Polygon
{
public:
    explicit Trapezoid(const std::vector<Point>& points)
        : Polygon(points, ShapeType::Trapezoid)
    {
    }
};

/// @brief Explicit parallelogram geometry for layout support.
class Parallelogram : public Polygon
{
public:
    explicit Parallelogram(const std::vector<Point>& points)
        : Polygon(points, ShapeType::Parallelogram)
    {
    }
};

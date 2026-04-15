#pragma once

#include <boost/geometry.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <geometry/Rect.hpp>
#include <geometry/Point.hpp>
#include <layout/Shape.hpp>
#include <vector>

namespace spatial {

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

using BoostPoint = bg::model::point<int, 2, bg::cs::cartesian>;
using BoostBox = bg::model::box<BoostPoint>;
using BoostPolygon = bg::model::polygon<BoostPoint>;

/// @brief Convert project Rect to Boost.Geometry box.
/// Keeps Boost includes confined to spatial module.
inline BoostBox rectToBoostBox(const Rect& r)
{
    BoostPoint minPt(r.min.x, r.min.y);
    BoostPoint maxPt(r.max.x, r.max.y);
    return BoostBox(minPt, maxPt);
}

/// @brief Convert Boost.Geometry box to project Rect.
inline Rect boostBoxToRect(const BoostBox& b)
{
    BoostPoint minPt = b.min_corner();
    BoostPoint maxPt = b.max_corner();
    return Rect(static_cast<int>(bg::get<0>(minPt)),
                static_cast<int>(bg::get<1>(minPt)),
                static_cast<int>(bg::get<0>(maxPt)),
                static_cast<int>(bg::get<1>(maxPt)));
}

/// @brief Convert project points to Boost.Geometry polygon.
inline BoostPolygon pointsToBoostPolygon(const std::vector<Point>& points)
{
    BoostPolygon poly;
    for (const auto& p : points)
    {
        bg::append(poly.outer(), BoostPoint(p.x, p.y));
    }
    bg::correct(poly);
    return poly;
}

/// @brief Convert Shape to Boost.Geometry polygon.
inline BoostPolygon shapeToBoostPolygon(const Shape& shape)
{
    if (shape.hasPoints())
    {
        return pointsToBoostPolygon(shape.getPoints());
    }
    else
    {
        // Rectangle: create polygon from bounds
        const Rect& r = shape.getBounds();
        std::vector<Point> points = {
            Point(r.min.x, r.min.y),
            Point(r.max.x, r.min.y),
            Point(r.max.x, r.max.y),
            Point(r.min.x, r.max.y),
            Point(r.min.x, r.min.y) // close the polygon
        };
        return pointsToBoostPolygon(points);
    }
}

/// @brief Check if two shapes intersect (including touching edges).
inline bool shapesIntersect(const Shape& s1, const Shape& s2)
{
    BoostPolygon p1 = shapeToBoostPolygon(s1);
    BoostPolygon p2 = shapeToBoostPolygon(s2);
    return bg::intersects(p1, p2);
}

/// @brief Check if inner shape is covered by outer shape (including touching boundary).
/// For enclosure candidates, this allows shapes that touch the boundary.
inline bool shapeWithin(const Shape& inner, const Shape& outer)
{
    BoostPolygon p1 = shapeToBoostPolygon(inner);
    BoostPolygon p2 = shapeToBoostPolygon(outer);
    return bg::covered_by(p1, p2);
}

/// @brief Calculate min enclosure distance for inner within outer (axis-aligned for rectangles).
/// Returns the minimum distance from the inner shape boundary to the outer shape boundary.
/// For rectangles, this is equivalent to the minimum margin on any side.
inline double shapeEnclosureDistance(const Shape& inner, const Shape& outer)
{
    // For efficiency, work with bounding rectangles
    // This gives the actual enclosure margin for axis-aligned rectangles
    const Rect& ib = inner.getBounds();
    const Rect& ob = outer.getBounds();
    
    double marginLeft = static_cast<double>(ib.min.x - ob.min.x);
    double marginRight = static_cast<double>(ob.max.x - ib.max.x);
    double marginBottom = static_cast<double>(ib.min.y - ob.min.y);
    double marginTop = static_cast<double>(ob.max.y - ib.max.y);
    
    return std::min({marginLeft, marginRight, marginBottom, marginTop});
}

/// @brief Calculate distance between two shapes (non-enclosure case).
inline double shapesDistance(const Shape& s1, const Shape& s2)
{
    BoostPolygon p1 = shapeToBoostPolygon(s1);
    BoostPolygon p2 = shapeToBoostPolygon(s2);
    return bg::distance(p1, p2);
}

/// @brief Check if two polygons intersect.
inline bool polygonsIntersect(const std::vector<Point>& poly1, const std::vector<Point>& poly2)
{
    BoostPolygon bp1 = pointsToBoostPolygon(poly1);
    BoostPolygon bp2 = pointsToBoostPolygon(poly2);
    return bg::intersects(bp1, bp2);
}

/// @brief Calculate distance between two polygons.
inline double polygonsDistance(const std::vector<Point>& poly1, const std::vector<Point>& poly2)
{
    BoostPolygon bp1 = pointsToBoostPolygon(poly1);
    BoostPolygon bp2 = pointsToBoostPolygon(poly2);
    return bg::distance(bp1, bp2);
}

} // namespace spatial

#pragma once

#include "Point.hpp"
#include "Rect.hpp"
#include <algorithm>

// Geometric utilities for DRC spatial queries
namespace GeometryUtils
{

// Check if rectangles overlap or touch
inline bool rectsOverlap(const Rect& rect1, const Rect& rect2)
{
    return !(rect1.max.x < rect2.min.x ||
             rect1.min.x > rect2.max.x ||
             rect1.max.y < rect2.min.y ||
             rect1.min.y > rect2.max.y);
}

// Check if rectangles strictly overlap (not touching)
inline bool rectsStrictlyOverlap(const Rect& rect1, const Rect& rect2)
{
    return !(rect1.max.x <= rect2.min.x ||
             rect1.min.x >= rect2.max.x ||
             rect1.max.y <= rect2.min.y ||
             rect1.min.y >= rect2.max.y);
}

// Calculate distance between rectangles (0 if overlapping)
inline double rectDistance(const Rect& rect1, const Rect& rect2)
{
    int dx = 0;
    if (rect1.max.x < rect2.min.x)
        dx = rect2.min.x - rect1.max.x;
    else if (rect2.max.x < rect1.min.x)
        dx = rect1.min.x - rect2.max.x;

    int dy = 0;
    if (rect1.max.y < rect2.min.y)
        dy = rect2.min.y - rect1.max.y;
    else if (rect2.max.y < rect1.min.y)
        dy = rect1.min.y - rect2.max.y;

    return std::sqrt(static_cast<double>(dx * dx + dy * dy));
}

inline Rect getUnion(const Rect& rect1, const Rect& rect2)
{
    return Rect(
        std::min(rect1.min.x, rect2.min.x),
        std::min(rect1.min.y, rect2.min.y),
        std::max(rect1.max.x, rect2.max.x),
        std::max(rect1.max.y, rect2.max.y)
    );
}

inline Rect getIntersection(const Rect& rect1, const Rect& rect2)
{
    int minX = std::max(rect1.min.x, rect2.min.x);
    int minY = std::max(rect1.min.y, rect2.min.y);
    int maxX = std::min(rect1.max.x, rect2.max.x);
    int maxY = std::min(rect1.max.y, rect2.max.y);

    return Rect(minX, minY, maxX, maxY);
}

inline bool contains(const Rect& container, const Rect& contained)
{
    return container.min.x <= contained.min.x &&
           container.min.y <= contained.min.y &&
           container.max.x >= contained.max.x &&
           container.max.y >= contained.max.y;
}

} // namespace GeometryUtils

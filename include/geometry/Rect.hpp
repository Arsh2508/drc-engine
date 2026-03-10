#pragma once

#include "Point.hpp"
#include <cmath>

// Rectangular bounding box with min/max corners
struct Rect
{
    Point min;  // Bottom-left corner
    Point max;  // Top-right corner

    Rect() : min(0, 0), max(0, 0) {}

    Rect(int minX, int minY, int maxX, int maxY)
        : min(minX, minY), max(maxX, maxY)
    {
    }

    Rect(const Point& minPt, const Point& maxPt)
        : min(minPt), max(maxPt)
    {
    }

    int width() const
    {
        return max.x - min.x;
    }

    int height() const
    {
        return max.y - min.y;
    }

    long long area() const
    {
        return static_cast<long long>(width()) * height();
    }

    Point center() const
    {
        return Point((min.x + max.x) / 2, (min.y + max.y) / 2);
    }

    bool contains(const Point& p) const
    {
        return p.x >= min.x && p.x <= max.x && p.y >= min.y && p.y <= max.y;
    }

    bool operator==(const Rect& other) const
    {
        return min == other.min && max == other.max;
    }

    // Expand rectangle for spatial queries (efficient with R-tree)
    Rect expanded(double distance) const
    {
        // use floor/ceil to ensure the expanded box fully covers the desired
        // floating‑point margin; avoids losing candidates due to integer truncation
        double nx = static_cast<double>(min.x) - distance;
        double ny = static_cast<double>(min.y) - distance;
        double xx = static_cast<double>(max.x) + distance;
        double xy = static_cast<double>(max.y) + distance;

        return Rect(
            static_cast<int>(std::floor(nx)),
            static_cast<int>(std::floor(ny)),
            static_cast<int>(std::ceil(xx)),
            static_cast<int>(std::ceil(xy))
        );
    }
};
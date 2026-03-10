#pragma once

#include <cmath>

// 2D point in layout space
struct Point
{
    int x;
    int y;

    Point(int coordX = 0, int coordY = 0) : x{coordX}, y{coordY} {}

    double distanceTo(const Point& other) const
    {
        double dx = x - other.x;
        double dy = y - other.y;
        return std::sqrt(dx * dx + dy * dy);
    }

    bool operator==(const Point& other) const
    {
        return x == other.x && y == other.y;
    }

    bool operator!=(const Point& other) const
    {
        return !(*this == other);
    }
};

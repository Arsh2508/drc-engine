#pragma once

#include <boost/geometry.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <geometry/Rect.hpp>

namespace spatial {

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

using BoostPoint = bg::model::point<int, 2, bg::cs::cartesian>;
using BoostBox = bg::model::box<BoostPoint>;

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

} // namespace spatial

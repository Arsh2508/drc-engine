#pragma once

#include "SpatialIndex.hpp"
#include "BoostGeometryAdapters.hpp"
#include <geometry/GeometryUtils.hpp>
#include <boost/geometry.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <vector>

namespace spatial {

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

/// @brief Spatial index backed by Boost.Geometry R-tree.
/// Stores pairs of (BoostBox, const Shape*). The pointers reference
/// shapes owned by the `Layout` container; therefore callers must
/// ensure the layout outlives the index (true for DrcEngine run-time).
class RTreeSpatialIndex : public SpatialIndex
{
public:
    using Value = std::pair<BoostBox, const Shape*>;
    using RTree = bgi::rtree<Value, bgi::quadratic<16>>;

    RTreeSpatialIndex() = default;

    void insert(const Shape& shape) override;

    std::vector<Shape> query(const Rect& queryRect) const override;

    std::vector<Shape> getAllShapes() const override;

    size_t getShapeCount() const override;

    void clear() override;

    Rect getBounds() const override;

private:
    RTree m_rtree;
};

} // namespace spatial

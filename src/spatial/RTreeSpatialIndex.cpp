#include <spatial/RTreeSpatialIndex.hpp>

#include <geometry/GeometryUtils.hpp>

#include <boost/geometry.hpp>
#include <boost/geometry/index/rtree.hpp>

#include <iostream>
#include <iterator>
#include <utility>

namespace spatial {

void RTreeSpatialIndex::insert(const Shape& shape)
{
    BoostBox box = rectToBoostBox(shape.getBounds());
    // debug: log inserted shape details to help verify index contents
    std::cout << "[RTree] inserting shape id=" << shape.getId()
              << " layer=" << shape.getLayer()
              << " bounds=(" << shape.getBounds().min.x << "," << shape.getBounds().min.y
              << ")-((" << shape.getBounds().max.x << "," << shape.getBounds().max.y << "))\n";
    m_rtree.insert(std::make_pair(box, &shape));
}

std::vector<Shape> RTreeSpatialIndex::query(const Rect& queryRect) const
{
    BoostBox qbox = rectToBoostBox(queryRect);
    std::vector<Value> results;
    m_rtree.query(bgi::intersects(qbox), std::back_inserter(results));

    // debug: log number of candidates returned for this query rectangle
    std::cout << "[RTree] query rect=(" << queryRect.min.x << "," << queryRect.min.y
              << ")-(" << queryRect.max.x << "," << queryRect.max.y << ")";
    std::cout << " returned " << results.size() << " candidate(s)\n";

    std::vector<Shape> out;
    out.reserve(results.size());
    for (const auto& v : results)
    {
        if (v.second)
            out.push_back(*v.second);
    }
    return out;
}

std::vector<Shape> RTreeSpatialIndex::getAllShapes() const
{
    std::vector<Shape> out;
    out.reserve(m_rtree.size());
    for (const auto& v : m_rtree)
    {
        if (v.second)
            out.push_back(*v.second);
    }
    return out;
}

size_t RTreeSpatialIndex::getShapeCount() const
{
    return m_rtree.size();
}

void RTreeSpatialIndex::clear()
{
    m_rtree.clear();
}

Rect RTreeSpatialIndex::getBounds() const
{
    if (m_rtree.empty())
        return Rect();

    auto it = m_rtree.begin();
    Rect bounds = boostBoxToRect(it->first);
    ++it;
    for (; it != m_rtree.end(); ++it)
    {
        bounds = GeometryUtils::getUnion(bounds, boostBoxToRect(it->first));
    }
    return bounds;
}

} // namespace spatial


#include <spatial/NaiveSpatialIndex.hpp>

#include <geometry/GeometryUtils.hpp>

void NaiveSpatialIndex::insert(const Shape& shape)
{
    m_shapes.push_back(shape);
}

std::vector<Shape> NaiveSpatialIndex::query(const Rect& queryRect) const
{
    std::vector<Shape> results;

    // Brute-force: check every shape
    for (const auto& shape : m_shapes)
    {
        if (GeometryUtils::rectsOverlap(queryRect, shape.getBounds()))
        {
            results.push_back(shape);
        }
    }

    return results;
}

std::vector<Shape> NaiveSpatialIndex::getAllShapes() const
{
    return m_shapes;
}

size_t NaiveSpatialIndex::getShapeCount() const
{
    return m_shapes.size();
}

void NaiveSpatialIndex::clear()
{
    m_shapes.clear();
}

Rect NaiveSpatialIndex::getBounds() const
{
    if (m_shapes.empty())
        return Rect();

    Rect bounds = m_shapes[0].getBounds();
    for (size_t i = 1; i < m_shapes.size(); ++i)
    {
        bounds = GeometryUtils::getUnion(bounds, m_shapes[i].getBounds());
    }
    return bounds;
}


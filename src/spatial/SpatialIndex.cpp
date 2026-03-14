#include <spatial/SpatialIndex.hpp>

void SpatialIndex::insertBatch(const std::vector<Shape>& shapes)
{
    for (const auto& shape : shapes)
    {
        insert(shape);
    }
}


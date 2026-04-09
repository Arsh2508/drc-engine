#pragma once

#include "SpatialIndex.hpp"
#include <geometry/GeometryUtils.hpp>
#include <algorithm>

/// @brief Naive spatial index using a simple vector.

class NaiveSpatialIndex : public SpatialIndex
{
public:
    NaiveSpatialIndex() = default;

    void insert(const Shape& shape) override;

    std::vector<Shape> query(const Rect& queryRect) const override;

    std::vector<Shape> getAllShapes() const override;

    size_t getShapeCount() const override;

    void clear() override;

    Rect getBounds() const override;

private:
    std::vector<Shape> m_shapes;
};

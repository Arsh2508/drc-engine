#pragma once

#include <layout/Shape.hpp>
#include <vector>
#include <memory>

/// @brief Abstract interface for spatial indexing in DRC systems.

class SpatialIndex
{
public:
    virtual ~SpatialIndex() = default;

    virtual void insert(const Shape& shape) = 0;

    virtual void insertBatch(const std::vector<Shape>& shapes);

    virtual std::vector<Shape> query(const Rect& queryRect) const = 0;

    virtual std::vector<Shape> getAllShapes() const = 0;

    virtual size_t getShapeCount() const = 0;

    virtual void clear() = 0;

    virtual Rect getBounds() const = 0;
};

using SpatialIndexPtr = std::shared_ptr<SpatialIndex>;

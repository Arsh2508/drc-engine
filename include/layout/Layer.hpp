#pragma once

#include "Shape.hpp"
#include <vector>
#include <string>

/// @brief Represents a physical layer in the design (e.g., metal1, metal2, poly).
/// Contains all shapes on this layer and metadata about the layer.
class Layer
{
public:
    using LayerId = int;

    Layer() : m_id(-1), m_name("") {}

    Layer(LayerId layerId, const std::string& layerName)
        : m_id(layerId), m_name(layerName)
    {
    }

    LayerId getId() const { return m_id; }

    const std::string& getName() const { return m_name; }

    void addShape(const Shape& shape)
    {
        m_shapes.push_back(shape);
    }

    void addShapes(const std::vector<Shape>& shapes)
    {
        m_shapes.insert(m_shapes.end(), shapes.begin(), shapes.end());
    }

    const std::vector<Shape>& getShapes() const { return m_shapes; }

    size_t getShapeCount() const { return m_shapes.size(); }

    void clear() { m_shapes.clear(); }

private:
    LayerId m_id;                
    std::string m_name;          
    std::vector<Shape> m_shapes; 
};

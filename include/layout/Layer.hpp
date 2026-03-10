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

    /// @brief Default construct an empty layer.
    Layer() : m_id(-1), m_name("") {}

    /// @brief Construct a layer with given layer number and name.
    /// @param layerId Physical layer number
    /// @param layerName Human-readable name (e.g., "metal1")
    Layer(LayerId layerId, const std::string& layerName)
        : m_id(layerId), m_name(layerName)
    {
    }

    /// @brief Get the layer identifier.
    LayerId getId() const { return m_id; }

    /// @brief Get the layer name.
    const std::string& getName() const { return m_name; }

    /// @brief Add a shape to this layer.
    /// @param shape Shape to add
    void addShape(const Shape& shape)
    {
        m_shapes.push_back(shape);
    }

    /// @brief Add multiple shapes to this layer.
    /// @param shapes Vector of shapes to add
    void addShapes(const std::vector<Shape>& shapes)
    {
        m_shapes.insert(m_shapes.end(), shapes.begin(), shapes.end());
    }

    /// @brief Get all shapes on this layer.
    const std::vector<Shape>& getShapes() const { return m_shapes; }

    /// @brief Get the number of shapes on this layer.
    size_t getShapeCount() const { return m_shapes.size(); }

    /// @brief Clear all shapes from this layer.
    void clear() { m_shapes.clear(); }

private:
    LayerId m_id;                ///< Layer identifier
    std::string m_name;          ///< Layer name (e.g., "metal1")
    std::vector<Shape> m_shapes; ///< All shapes on this layer
};

#pragma once

#include "Layer.hpp"
#include <geometry/Rect.hpp>
#include <map>
#include <memory>
#include <unordered_map>
#include <string>
#include <vector>

/// @brief Represents the entire design layout containing multiple layers.
/// Acts as the top-level data structure that the DRC engine operates on.
/// Provides convenient access to layers and all shapes in the design.
class Layout
{
public:
    /// @brief Create an empty layout.
    Layout() = default;

    /// @brief Per-layer rule configuration.
    struct LayerRuleConfig {
        double minWidth = 0.0;
        double minHeight = 0.0;
        double minArea = 0.0;
    };

    /// @brief Add a layer to the layout.
    /// If a layer with the same ID already exists, it will be replaced.
    /// @param layer Layer to add
    void addLayer(const Layer& layer);

    /// @brief Set per-layer rule configuration by layer name.
    void setLayerRuleConfig(const std::string& layerName, const LayerRuleConfig& cfg);

    /// @brief Get per-layer rule configuration. Returns nullptr if not present.
    const LayerRuleConfig* getLayerRuleConfig(const std::string& layerName) const;

    /// @brief Get layer id for a given layer name. Returns -1 if not found.
    Layer::LayerId getLayerIdByName(const std::string& layerName) const;

    /// @brief Get a layer by ID.
    /// @param layerId Layer identifier
    /// @return Pointer to the layer, or nullptr if not found
    Layer* getLayer(Layer::LayerId layerId);

    /// @brief Get a layer by ID (const version).
    const Layer* getLayer(Layer::LayerId layerId) const;

    /// @brief Check if a layer exists.
    /// @param layerId Layer identifier
    /// @return true if layer exists, false otherwise
    bool hasLayer(Layer::LayerId layerId) const;

    /// @brief Get all layers in the layout.
    const std::map<Layer::LayerId, Layer>& getLayers() const { return m_layers; }

    /// @brief Collect all shapes from all layers.
    /// Useful for spatial indexing where all shapes need to be indexed together.
    /// @return Vector of all shapes in the layout
    std::vector<Shape> getAllShapes() const;

    /// @brief Collect all shapes on a specific layer.
    /// @param layerId Layer identifier
    /// @return Vector of shapes on the specified layer
    std::vector<Shape> getShapesByLayer(Layer::LayerId layerId) const;

    /// @brief Get the total number of layers.
    size_t getLayerCount() const { return m_layers.size(); }

    /// @brief Get the total number of shapes across all layers.
    size_t getTotalShapeCount() const;

    /// @brief Clear all layers and shapes.
    void clear();

    /// @brief Get the overall bounding box of the entire layout.
    /// @return Bounding rectangle encompassing all shapes
    Rect getBoundingBox() const;

private:
    std::map<Layer::LayerId, Layer> m_layers; ///< All layers in the design
    std::unordered_map<std::string, LayerRuleConfig> m_layerConfigs; ///< Layer name -> rules
    std::unordered_map<std::string, Layer::LayerId> m_nameToId; ///< Layer name -> id
};

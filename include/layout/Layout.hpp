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
    Layout() = default;

    struct LayerRuleConfig {
        double minWidth = 0.0;
        double minHeight = 0.0;
        double minArea = 0.0;
    };

    /// If a layer with the same ID already exists, it will be replaced.
    void addLayer(const Layer& layer);

    void setLayerRuleConfig(const std::string& layerName, const LayerRuleConfig& cfg);

    const LayerRuleConfig* getLayerRuleConfig(const std::string& layerName) const;

    Layer::LayerId getLayerIdByName(const std::string& layerName) const;

    Layer* getLayer(Layer::LayerId layerId);

    const Layer* getLayer(Layer::LayerId layerId) const;

    bool hasLayer(Layer::LayerId layerId) const;

    const std::map<Layer::LayerId, Layer>& getLayers() const { return m_layers; }

    std::vector<Shape> getAllShapes() const;

    std::vector<Shape> getShapesByLayer(Layer::LayerId layerId) const;

    size_t getLayerCount() const { return m_layers.size(); }

    size_t getTotalShapeCount() const;

    void clear();

    Rect getBoundingBox() const;

private:
    std::map<Layer::LayerId, Layer> m_layers; 
    std::unordered_map<std::string, LayerRuleConfig> m_layerConfigs; ///< Layer name -> rules
    std::unordered_map<std::string, Layer::LayerId> m_nameToId; ///< Layer name -> id
};

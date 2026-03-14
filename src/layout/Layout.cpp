#include <layout/Layout.hpp>

#include <geometry/GeometryUtils.hpp>

#include <map>
#include <string>
#include <unordered_map>
#include <vector>

void Layout::addLayer(const Layer& layer)
{
    m_layers[layer.getId()] = layer;
    // maintain name -> id mapping
    m_nameToId[layer.getName()] = layer.getId();
}

void Layout::setLayerRuleConfig(const std::string& layerName, const LayerRuleConfig& cfg)
{
    m_layerConfigs[layerName] = cfg;
}

const Layout::LayerRuleConfig* Layout::getLayerRuleConfig(const std::string& layerName) const
{
    auto it = m_layerConfigs.find(layerName);
    if (it != m_layerConfigs.end())
        return &it->second;
    return nullptr;
}

Layer::LayerId Layout::getLayerIdByName(const std::string& layerName) const
{
    auto it = m_nameToId.find(layerName);
    if (it != m_nameToId.end())
        return it->second;
    return static_cast<Layer::LayerId>(-1);
}

Layer* Layout::getLayer(Layer::LayerId layerId)
{
    auto it = m_layers.find(layerId);
    if (it != m_layers.end())
        return &it->second;
    return nullptr;
}

const Layer* Layout::getLayer(Layer::LayerId layerId) const
{
    auto it = m_layers.find(layerId);
    if (it != m_layers.end())
        return &it->second;
    return nullptr;
}

bool Layout::hasLayer(Layer::LayerId layerId) const
{
    return m_layers.find(layerId) != m_layers.end();
}

std::vector<Shape> Layout::getAllShapes() const
{
    std::vector<Shape> allShapes;
    for (const auto& [layerId, layer] : m_layers)
    {
        (void)layerId;
        const auto& shapes = layer.getShapes();
        allShapes.insert(allShapes.end(), shapes.begin(), shapes.end());
    }
    return allShapes;
}

std::vector<Shape> Layout::getShapesByLayer(Layer::LayerId layerId) const
{
    const Layer* layer = getLayer(layerId);
    if (layer)
        return std::vector<Shape>(layer->getShapes());
    return {};
}

size_t Layout::getTotalShapeCount() const
{
    size_t count = 0;
    for (const auto& [layerId, layer] : m_layers)
    {
        (void)layerId;
        count += layer.getShapeCount();
    }
    return count;
}

void Layout::clear()
{
    m_layers.clear();
}

Rect Layout::getBoundingBox() const
{
    if (m_layers.empty())
        return Rect();

    Rect bbox;
    bool first = true;

    for (const auto& [layerId, layer] : m_layers)
    {
        (void)layerId;
        for (const auto& shape : layer.getShapes())
        {
            if (first)
            {
                bbox = shape.getBounds();
                first = false;
            }
            else
            {
                bbox = GeometryUtils::getUnion(bbox, shape.getBounds());
            }
        }
    }

    return bbox;
}

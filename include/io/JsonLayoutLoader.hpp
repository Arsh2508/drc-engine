#pragma once

#include "LayoutLoader.hpp"
#include <drc/DrcRule.hpp>
#include <geometry/Rect.hpp>
#include <layout/Layer.hpp>
#include <layout/Layout.hpp>
#include <layout/Shape.hpp>
#include <nlohmann/json.hpp>
#include <vector>
#include <memory>
#include <string>
#include <utility>

using json = nlohmann::json;

// Loads layout from JSON format
class JsonLayoutLoader : public LayoutLoader
{
public:
    std::shared_ptr<Layout> load(const std::string& filename) override;

    std::pair<std::shared_ptr<Layout>, std::vector<DrcRulePtr>>
    loadWithRules(const std::string& filename);

    std::string getDescription() const override;

private:
    void parseLayer(const json& layerJson,
                    Layout& layout,
                    int defaultLayerId);

    Shape parseShape(const json& shapeJson, int layerId, size_t defaultShapeId);

    Rect parseRect(const json& rectJson);
};

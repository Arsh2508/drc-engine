#include <io/RulesJsonLoader.hpp>

#include <rules/ContainmentRule.hpp>
#include <rules/EnclosureRule.hpp>
#include <rules/IntersectionRule.hpp>
#include <rules/MinAreaRule.hpp>
#include <rules/MinSpacingRule.hpp>
#include <rules/MinWidthRule.hpp>

#include <fstream>
#include <stdexcept>

std::vector<DrcRulePtr> RulesJsonLoader::load(const std::string& filename,
                                              const std::shared_ptr<Layout>& layout)
{
    if (!layout)
    {
        throw std::runtime_error("RulesJsonLoader requires a loaded Layout instance");
    }

    std::ifstream file(filename);
    if (!file.is_open())
    {
        throw std::runtime_error("Could not open rules file: " + filename);
    }

    json rulesJson;
    try
    {
        file >> rulesJson;
    }
    catch (const json::exception& e)
    {
        throw std::runtime_error("Failed to parse JSON: " + std::string(e.what()));
    }

    file.close();

    const json* root = &rulesJson;
    if (rulesJson.contains("rules") && rulesJson["rules"].is_object())
    {
        root = &rulesJson["rules"];
    }

    if (!root->is_object())
    {
        throw std::runtime_error("Rules JSON must contain an object at the top level or inside 'rules'");
    }

    std::vector<DrcRulePtr> rules;

    if (root->contains("layers") && (*root)["layers"].is_object())
    {
        parseLayerRuleConfig((*root)["layers"], *layout);
    }

    if (root->contains("intersection"))
    {
        parseIntersection((*root)["intersection"], rules);
    }

    if (root->contains("minSpacing"))
    {
        parseMinSpacing((*root)["minSpacing"], layout, rules);
    }

    if (root->contains("enclosure"))
    {
        parseEnclosure((*root)["enclosure"], rules);
    }

    if (root->contains("containment"))
    {
        parseContainment((*root)["containment"], rules);
    }

    if (!layout->getLayers().empty())
    {
        rules.push_back(std::make_shared<MinWidthRule>());
        rules.push_back(std::make_shared<MinAreaRule>());
    }

    return rules;
}

std::string RulesJsonLoader::getDescription() const
{
    return "JSON Rules Loader (nlohmann/json)";
}

void RulesJsonLoader::parseLayerRuleConfig(const json& layersJson, Layout& layout)
{
    if (!layersJson.is_object())
    {
        throw std::runtime_error("'layers' section in rules JSON must be an object");
    }

    for (auto it = layersJson.begin(); it != layersJson.end(); ++it)
    {
        const std::string layerName = it.key();
        const json& layerJson = it.value();

        if (!layerJson.is_object())
        {
            throw std::runtime_error("Layer rule config for '" + layerName + "' must be an object");
        }

        auto layerId = layout.getLayerIdByName(layerName);
        if (layerId < 0 || !layout.hasLayer(layerId))
        {
            throw std::runtime_error("Rules file references unknown layer: " + layerName);
        }

        Layout::LayerRuleConfig cfg;
        if (layerJson.contains("minWidth"))
        {
            if (!layerJson["minWidth"].is_number())
                throw std::runtime_error("Layer '" + layerName + "' field 'minWidth' must be numeric");
            cfg.minWidth = layerJson["minWidth"].get<double>();
        }
        if (layerJson.contains("minHeight"))
        {
            if (!layerJson["minHeight"].is_number())
                throw std::runtime_error("Layer '" + layerName + "' field 'minHeight' must be numeric");
            cfg.minHeight = layerJson["minHeight"].get<double>();
        }
        if (layerJson.contains("minArea"))
        {
            if (!layerJson["minArea"].is_number())
                throw std::runtime_error("Layer '" + layerName + "' field 'minArea' must be numeric");
            cfg.minArea = layerJson["minArea"].get<double>();
        }

        layout.setLayerRuleConfig(layerName, cfg);
    }
}

void RulesJsonLoader::parseIntersection(const json& intersectionJson,
                                        std::vector<DrcRulePtr>& rules) const
{
    if (intersectionJson.is_boolean())
    {
        if (intersectionJson.get<bool>())
        {
            rules.push_back(std::make_shared<IntersectionRule>());
        }
        return;
    }

    if (intersectionJson.is_array() || intersectionJson.is_object())
    {
        rules.push_back(std::make_shared<IntersectionRule>());
        return;
    }

    throw std::runtime_error("Unsupported format for 'intersection' rule in rules JSON");
}

void RulesJsonLoader::parseMinSpacing(const json& minSpacingJson,
                                      const std::shared_ptr<Layout>& layout,
                                      std::vector<DrcRulePtr>& rules) const
{
    if (!minSpacingJson.is_array())
    {
        throw std::runtime_error("'minSpacing' field must be an array");
    }

    for (const auto& item : minSpacingJson)
    {
        if (!item.is_object() || !item.contains("layer") || !item.contains("minDist"))
        {
            throw std::runtime_error("Each minSpacing entry must include 'layer' and 'minDist'");
        }

        const std::string layerName = item["layer"].get<std::string>();
        if (!item["minDist"].is_number())
        {
            throw std::runtime_error("minSpacing 'minDist' must be numeric");
        }

        const double minDist = item["minDist"].get<double>();
        const int layerId = layout->getLayerIdByName(layerName);
        if (layerId < 0 || !layout->hasLayer(layerId))
        {
            throw std::runtime_error("minSpacing references unknown layer: " + layerName);
        }

        rules.push_back(std::make_shared<MinSpacingRule>(minDist, layerId));
    }
}

void RulesJsonLoader::parseEnclosure(const json& enclosureJson,
                                     std::vector<DrcRulePtr>& rules) const
{
    if (!enclosureJson.is_array())
    {
        throw std::runtime_error("'enclosure' field must be an array");
    }

    for (const auto& item : enclosureJson)
    {
        if (!item.is_object() || !item.contains("inner") || !item.contains("outer"))
        {
            throw std::runtime_error("Each enclosure entry must include 'inner' and 'outer'");
        }

        const std::string inner = item["inner"].get<std::string>();
        const std::string outer = item["outer"].get<std::string>();
        double margin = 0.0;
        if (item.contains("margin"))
        {
            if (!item["margin"].is_number())
                throw std::runtime_error("enclosure 'margin' must be numeric");
            margin = item["margin"].get<double>();
        }

        rules.push_back(std::make_shared<EnclosureRule>(inner, outer, margin));
    }
}

void RulesJsonLoader::parseContainment(const json& containmentJson,
                                       std::vector<DrcRulePtr>& rules) const
{
    if (!containmentJson.is_array())
    {
        throw std::runtime_error("'containment' field must be an array");
    }

    for (const auto& item : containmentJson)
    {
        if (!item.is_object() || !item.contains("layerA") || !item.contains("layerB"))
        {
            throw std::runtime_error("Each containment entry must include 'layerA' and 'layerB'");
        }

        const std::string layerA = item["layerA"].get<std::string>();
        const std::string layerB = item["layerB"].get<std::string>();
        rules.push_back(std::make_shared<ContainmentRule>(layerA, layerB));
    }
}

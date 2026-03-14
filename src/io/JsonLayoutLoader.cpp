#include <io/JsonLayoutLoader.hpp>

#include <rules/ContainmentRule.hpp>
#include <rules/EnclosureRule.hpp>
#include <rules/IntersectionRule.hpp>
#include <rules/MinAreaRule.hpp>
#include <rules/MinSpacingRule.hpp>
#include <rules/MinWidthRule.hpp>

#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

std::shared_ptr<Layout> JsonLayoutLoader::load(const std::string& filename)
{
    // Check file exists and is readable
    std::ifstream file(filename);
    if (!file.is_open())
    {
        throw std::runtime_error("Could not open file: " + filename);
    }

    // Parse JSON
    json layoutJson;
    try
    {
        file >> layoutJson;
    }
    catch (const json::exception& e)
    {
        throw std::runtime_error("Failed to parse JSON: " + std::string(e.what()));
    }

    file.close();

    // Validate top-level structure
    if (!layoutJson.contains("layers"))
    {
        throw std::runtime_error("JSON missing required 'layers' field");
    }

    auto layout = std::make_shared<Layout>();

    // Parse layers
    // Backwards-compatible parsing: if "layers" is an array (old format)
    if (layoutJson["layers"].is_array())
    {
        for (size_t layerIdx = 0; layerIdx < layoutJson["layers"].size(); ++layerIdx)
        {
            const auto& layerJson = layoutJson["layers"][layerIdx];
            parseLayer(layerJson, *layout, static_cast<int>(layerIdx));
        }
    }
    else if (layoutJson["layers"].is_object())
    {
        // New format: object mapping layer name -> properties (minWidth, shapes omitted)
        int nextId = 0;
        for (auto it = layoutJson["layers"].begin(); it != layoutJson["layers"].end(); ++it)
        {
            const std::string layerName = it.key();
            const auto& props = it.value();

            Layer layer(nextId, layerName);

            // If shapes are provided inside this object, parse them
            if (props.contains("shapes") && props["shapes"].is_array())
            {
                for (size_t si = 0; si < props["shapes"].size(); ++si)
                {
                    Shape s = parseShape(props["shapes"][si], nextId, si);
                    layer.addShape(s);
                }
            }

            // Parse per-layer rule config if present
            Layout::LayerRuleConfig cfg;
            if (props.contains("minWidth"))
                cfg.minWidth = props["minWidth"].get<double>();
            if (props.contains("minHeight"))
                cfg.minHeight = props["minHeight"].get<double>();
            if (props.contains("minArea"))
                cfg.minArea = props["minArea"].get<double>();

            layout->addLayer(layer);
            layout->setLayerRuleConfig(layerName, cfg);

            ++nextId;
        }
    }
    else
    {
        throw std::runtime_error("Unsupported 'layers' format in JSON");
    }

    return layout;
}

std::pair<std::shared_ptr<Layout>, std::vector<DrcRulePtr>>
JsonLayoutLoader::loadWithRules(const std::string& filename)
{
    // Read file and parse JSON first (reuse existing load error checks)
    std::ifstream file(filename);
    if (!file.is_open())
    {
        throw std::runtime_error("Could not open file: " + filename);
    }

    json layoutJson;
    try
    {
        file >> layoutJson;
    }
    catch (const json::exception& e)
    {
        throw std::runtime_error(std::string("Failed to parse JSON: ") + e.what());
    }
    file.close();

    auto layout = std::make_shared<Layout>();
    std::vector<DrcRulePtr> rules;

    // Parse layers (reuse logic from load)
    if (!layoutJson.contains("layers"))
        throw std::runtime_error("JSON missing required 'layers' field");

    if (layoutJson["layers"].is_array())
    {
        for (size_t layerIdx = 0; layerIdx < layoutJson["layers"].size(); ++layerIdx)
        {
            const auto& layerJson = layoutJson["layers"][layerIdx];
            parseLayer(layerJson, *layout, static_cast<int>(layerIdx));
        }
    }
    else if (layoutJson["layers"].is_object())
    {
        int nextId = 0;
        for (auto it = layoutJson["layers"].begin(); it != layoutJson["layers"].end(); ++it)
        {
            const std::string layerName = it.key();
            const auto& props = it.value();

            Layer layer(nextId, layerName);

            if (props.contains("shapes") && props["shapes"].is_array())
            {
                for (size_t si = 0; si < props["shapes"].size(); ++si)
                {
                    Shape s = parseShape(props["shapes"][si], nextId, si);
                    layer.addShape(s);
                }
            }

            Layout::LayerRuleConfig cfg;
            if (props.contains("minWidth"))
                cfg.minWidth = props["minWidth"].get<double>();
            if (props.contains("minHeight"))
                cfg.minHeight = props["minHeight"].get<double>();
            if (props.contains("minArea"))
                cfg.minArea = props["minArea"].get<double>();

            layout->addLayer(layer);
            layout->setLayerRuleConfig(layerName, cfg);

            ++nextId;
        }
    }
    else
    {
        throw std::runtime_error("Unsupported 'layers' format in JSON");
    }

    // If top-level shapes array exists (alternate format), parse shapes into layers by name
    if (layoutJson.contains("shapes") && layoutJson["shapes"].is_array())
    {
        for (const auto& sj : layoutJson["shapes"])
        {
            // Expected: id, layer (name), x1,y1,x2,y2
            if (!sj.contains("layer") || !sj.contains("rect"))
                continue;
            std::string lname = sj["layer"].get<std::string>();
            auto lid = layout->getLayerIdByName(lname);
            if (lid < 0)
                continue;
            Shape s = parseShape(sj, lid, 0);
            Layer* layerPtr = layout->getLayer(lid);
            if (layerPtr)
                layerPtr->addShape(s);
        }
    }

    // Parse rules object if present
    if (layoutJson.contains("rules") && layoutJson["rules"].is_object())
    {
        const auto& rulesJson = layoutJson["rules"];

        // Intersection rules (boolean flag or empty array indicates desire)
        if (rulesJson.contains("intersection"))
        {
            // if empty array or object, still register default IntersectionRule
            if (rulesJson["intersection"].is_array() && !rulesJson["intersection"].empty())
            {
                // future: parse parameters per-layer
            }
            rules.push_back(std::make_shared<IntersectionRule>());
        }

        // Min-spacing rules
        if (rulesJson.contains("minSpacing") && rulesJson["minSpacing"].is_array())
        {
            for (const auto& ms : rulesJson["minSpacing"])
            {
                if (!ms.contains("layer") || !ms.contains("minDist"))
                    continue;
                std::string lname = ms["layer"].get<std::string>();
                double minDist = ms["minDist"].get<double>();
                int lid = layout->getLayerIdByName(lname);
                if (lid >= 0)
                {
                    rules.push_back(std::make_shared<MinSpacingRule>(minDist, lid));
                }
            }
        }

        // Enclosure rules
        if (rulesJson.contains("enclosure") && rulesJson["enclosure"].is_array())
        {
            for (const auto& ej : rulesJson["enclosure"])
            {
                std::string inner = ej["inner"].get<std::string>();
                std::string outer = ej["outer"].get<std::string>();
                double margin = ej.contains("margin") ? ej["margin"].get<double>() : 0.0;
                rules.push_back(std::make_shared<EnclosureRule>(inner, outer, margin));
            }
        }

        // Containment rules
        if (rulesJson.contains("containment") && rulesJson["containment"].is_array())
        {
            for (const auto& cj : rulesJson["containment"])
            {
                std::string a = cj["layerA"].get<std::string>();
                std::string b = cj["layerB"].get<std::string>();
                rules.push_back(std::make_shared<ContainmentRule>(a, b));
            }
        }
    }

    // Always add min-size/area rules if any layer configs exist
    if (!layout->getLayers().empty())
    {
        rules.push_back(std::make_shared<MinWidthRule>());
        rules.push_back(std::make_shared<MinAreaRule>());
    }

    return {layout, rules};
}

std::string JsonLayoutLoader::getDescription() const
{
    return "JSON Layout Loader (nlohmann/json)";
}

void JsonLayoutLoader::parseLayer(const json& layerJson,
                                 Layout& layout,
                                 int defaultLayerId)
{
    // Extract layer ID and name
    int layerId = defaultLayerId;
    if (layerJson.contains("id"))
    {
        if (!layerJson["id"].is_number_integer())
        {
            throw std::runtime_error("Layer 'id' must be an integer");
        }
        layerId = layerJson["id"];
    }

    std::string layerName = "layer_" + std::to_string(layerId);
    if (layerJson.contains("name"))
    {
        if (!layerJson["name"].is_string())
        {
            throw std::runtime_error("Layer 'name' must be a string");
        }
        layerName = layerJson["name"];
    }

    // Create layer
    Layer layer(layerId, layerName);

    // Parse shapes if present
    if (layerJson.contains("shapes"))
    {
        if (!layerJson["shapes"].is_array())
        {
            throw std::runtime_error(
                "Layer '" + layerName + "' has 'shapes' that is not an array");
        }

        for (size_t shapeIdx = 0; shapeIdx < layerJson["shapes"].size(); ++shapeIdx)
        {
            const auto& shapeJson = layerJson["shapes"][shapeIdx];
            Shape shape = parseShape(shapeJson, layerId, shapeIdx);
            layer.addShape(shape);
        }
    }

    // look for optional per-layer rule configuration
    Layout::LayerRuleConfig cfg;
    if (layerJson.contains("minWidth"))
    {
        if (!layerJson["minWidth"].is_number())
            throw std::runtime_error("Layer 'minWidth' must be numeric");
        cfg.minWidth = layerJson["minWidth"].get<double>();
    }
    if (layerJson.contains("minHeight"))
    {
        if (!layerJson["minHeight"].is_number())
            throw std::runtime_error("Layer 'minHeight' must be numeric");
        cfg.minHeight = layerJson["minHeight"].get<double>();
    }
    if (layerJson.contains("minArea"))
    {
        if (!layerJson["minArea"].is_number())
            throw std::runtime_error("Layer 'minArea' must be numeric");
        cfg.minArea = layerJson["minArea"].get<double>();
    }
    if (cfg.minWidth > 0.0 || cfg.minHeight > 0.0 || cfg.minArea > 0.0)
    {
        layout.setLayerRuleConfig(layerName, cfg);
    }

    layout.addLayer(layer);
}

Shape JsonLayoutLoader::parseShape(const json& shapeJson,
                                  int layerId,
                                  size_t defaultShapeId)
{
    // Extract shape ID
    int shapeId = static_cast<int>(defaultShapeId);
    if (shapeJson.contains("id"))
    {
        if (!shapeJson["id"].is_number_integer())
        {
            throw std::runtime_error("Shape 'id' must be an integer");
        }
        shapeId = shapeJson["id"];
    }

    // Parse rectangle
    if (!shapeJson.contains("rect"))
    {
        throw std::runtime_error(
            "Shape " + std::to_string(shapeId) + " missing required 'rect' field");
    }

    Rect rect = parseRect(shapeJson["rect"]);

    return Shape(rect, shapeId, layerId);
}

Rect JsonLayoutLoader::parseRect(const json& rectJson)
{
    // Validate required fields
    const char* requiredFields[] = {"x1", "y1", "x2", "y2"};
    for (const char* field : requiredFields)
    {
        if (!rectJson.contains(field))
        {
            throw std::runtime_error(
                std::string("Rectangle missing required field: ") + field);
        }
        if (!rectJson[field].is_number_integer())
        {
            throw std::runtime_error(
                std::string("Rectangle field '") + field + "' must be an integer");
        }
    }

    int x1 = rectJson["x1"];
    int y1 = rectJson["y1"];
    int x2 = rectJson["x2"];
    int y2 = rectJson["y2"];

    // Basic validation
    if (x1 >= x2 || y1 >= y2)
    {
        throw std::runtime_error("Rectangle bounds invalid: min must be less than max");
    }

    return Rect(x1, y1, x2, y2);
}


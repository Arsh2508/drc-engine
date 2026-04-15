#include <io/JsonLayoutLoader.hpp>
#include <io/RulesJsonLoader.hpp>

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
    if (!layoutJson.contains("layers") && !layoutJson.contains("objects"))
    {
        throw std::runtime_error("JSON missing required 'layers' or 'objects' field");
    }

    auto layout = std::make_shared<Layout>();

    // Parse objects format (new format)
    if (layoutJson.contains("objects"))
    {
        if (!layoutJson["objects"].is_array())
        {
            throw std::runtime_error("'objects' field must be an array");
        }

        std::map<std::string, std::vector<Shape>> layerShapes;
        int nextShapeId = 0;

        for (const auto& objJson : layoutJson["objects"])
        {
            if (!objJson.contains("type") || !objJson.contains("layer"))
            {
                throw std::runtime_error("Object missing required 'type' or 'layer' field");
            }

            std::string layerName = objJson["layer"];
            Shape shape = parseShape(objJson, 0, nextShapeId++);
            layerShapes[layerName].push_back(shape);
        }

        // Create layers
        int layerId = 0;
        for (const auto& [layerName, shapes] : layerShapes)
        {
            Layer layer(layerId++, layerName);
            for (const auto& shape : shapes)
            {
                layer.addShape(shape);
            }
            layout->addLayer(layer);

            // Set default layer rule config for objects format
            Layout::LayerRuleConfig cfg;
            cfg.minWidth = 5.0;   // Default minimum width
            cfg.minHeight = 5.0;  // Default minimum height
            cfg.minArea = 25.0;   // Default minimum area (5x5)
            layout->setLayerRuleConfig(layerName, cfg);
        }
    }
    // Parse layers format (existing format)
    else if (layoutJson.contains("layers"))
    {
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
    }

    return layout;
}

std::pair<std::shared_ptr<Layout>, std::vector<DrcRulePtr>>
JsonLayoutLoader::loadWithRules(const std::string& filename)
{
    auto layout = load(filename);
    RulesJsonLoader rulesLoader;
    auto rules = rulesLoader.load(filename, layout);
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

    std::string shapeType = "rectangle";
    if (shapeJson.contains("type"))
    {
        if (!shapeJson["type"].is_string())
        {
            throw std::runtime_error("Shape 'type' must be a string");
        }
        shapeType = shapeJson["type"].get<std::string>();
    }

    if (shapeType == "rectangle")
    {
        if (shapeJson.contains("rect"))
        {
            Rect rect = parseRect(shapeJson["rect"]);
            return Shape(rect, shapeId, layerId);
        }
        else if (shapeJson.contains("x") && shapeJson.contains("y") &&
                 shapeJson.contains("width") && shapeJson.contains("height"))
        {
            int x = shapeJson["x"];
            int y = shapeJson["y"];
            int width = shapeJson["width"];
            int height = shapeJson["height"];
            Rect rect(x, y, x + width, y + height);
            return Shape(rect, shapeId, layerId);
        }
        else
        {
            throw std::runtime_error(
                "Rectangle shape " + std::to_string(shapeId) + " missing required 'rect' or 'x','y','width','height' fields");
        }
    }

    if (!shapeJson.contains("points"))
    {
        throw std::runtime_error(
            "Shape " + std::to_string(shapeId) + " of type '" + shapeType + "' missing required 'points' field");
    }

    std::vector<Point> points = parsePoints(shapeJson["points"]);
    if (shapeType == "trapezoid")
    {
        return Shape(Trapezoid(points), shapeId, layerId);
    }
    else if (shapeType == "parallelogram")
    {
        return Shape(Parallelogram(points), shapeId, layerId);
    }
    else if (shapeType == "polygon")
    {
        return Shape(Polygon(points, ShapeType::Polygon), shapeId, layerId);
    }
    else
    {
        throw std::runtime_error(
            "Unsupported shape type: '" + shapeType + "' for shape " + std::to_string(shapeId));
    }
}

std::vector<Point> JsonLayoutLoader::parsePoints(const json& pointsJson)
{
    if (!pointsJson.is_array() || pointsJson.empty())
    {
        throw std::runtime_error("Shape 'points' must be a non-empty array");
    }

    std::vector<Point> points;
    points.reserve(pointsJson.size());

    for (size_t i = 0; i < pointsJson.size(); ++i)
    {
        const auto& pointJson = pointsJson[i];
        if (!pointJson.is_object())
        {
            throw std::runtime_error("Each point must be an object with 'x' and 'y'");
        }
        if (!pointJson.contains("x") || !pointJson.contains("y"))
        {
            throw std::runtime_error("Point must contain both 'x' and 'y'");
        }
        if (!pointJson["x"].is_number_integer() || !pointJson["y"].is_number_integer())
        {
            throw std::runtime_error("Point coordinates must be integers");
        }
        points.emplace_back(pointJson["x"].get<int>(), pointJson["y"].get<int>());
    }

    return points;
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


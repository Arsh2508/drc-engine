#include <core/DrcEngine.hpp>
#include <io/JsonLayoutLoader.hpp>
#include <io/RulesJsonLoader.hpp>
#include <rules/MinSpacingRule.hpp>
#include <rules/IntersectionRule.hpp>
#include <layout/Layout.hpp>
#include <layout/Layer.hpp>
#include <layout/Shape.hpp>
#include <iostream>
#include <iomanip>
#include <memory>
#include <cstring>

// CLI demonstration of the DRC system
/// Helper to print a formatted header.
void printHeader(const std::string& title)
{
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << title << "\n";
    std::cout << std::string(60, '=') << "\n";
}

/// Helper to print a formatted sub-header.
void printSubheader(const std::string& title)
{
    std::cout << "\n" << std::string(40, '-') << "\n";
    std::cout << title << "\n";
    std::cout << std::string(40, '-') << "\n";
}

/// Create a sample layout for demonstration.
/// Returns a layout with shapes on different layers for testing.
Layout createSampleLayout()
{
    Layout layout;

    // Create metal1 layer (layer ID = 0)
    Layer metal1(0, "metal1");

    // Add some shapes to metal1 layer
    // Shape format: Rect(minX, minY, maxX, maxY), ID, Layer
    metal1.addShape(Shape(Rect(0, 0, 10, 10), 1, 0));     // Shape 1
    metal1.addShape(Shape(Rect(20, 0, 30, 10), 2, 0));    // Shape 2 (gap of 10)
    metal1.addShape(Shape(Rect(35, 0, 45, 10), 3, 0));    // Shape 3 (gap of 5 - violation!)
    metal1.addShape(Shape(Rect(0, 20, 10, 30), 4, 0));    // Shape 4
    metal1.addShape(Shape(Rect(15, 20, 25, 30), 5, 0));   // Shape 5 (gap of 5 - violation!)

    layout.addLayer(metal1);

    // Create metal2 layer (layer ID = 1)
    Layer metal2(1, "metal2");

    // Add shapes to metal2 layer
    metal2.addShape(Shape(Rect(5, 5, 15, 15), 6, 1));     // Shape 6
    metal2.addShape(Shape(Rect(30, 5, 40, 15), 7, 1));    // Shape 7 (gap of 15)
    metal2.addShape(Shape(Rect(50, 5, 60, 15), 8, 1));    // Shape 8 (gap of 10)
    metal2.addShape(Shape(Rect(0, 40, 10, 50), 9, 1));    // Shape 9
    metal2.addShape(Shape(Rect(12, 40, 22, 50), 10, 1));  // Shape 10 (gap of 2 - violation!)

    layout.addLayer(metal2);

    return layout;
}

/// Print usage information.
void printUsage(const char* programName)
{
    std::cout << "Usage: " << programName << " [layout.json] [--rules rules.json]\n\n";
    std::cout << "If a layout JSON file is provided, loads layout from that file.\n";
    std::cout << "If --rules is provided, loads rules from the separate rules JSON file.\n";
    std::cout << "Otherwise, runs demo with a sample hardcoded layout.\n\n";
    std::cout << "Layout JSON Format:\n";
    std::cout << "{\n";
    std::cout << "  \"layers\": [\n";
    std::cout << "    {\n";
    std::cout << "      \"name\": \"metal1\",\n";
    std::cout << "      \"id\": 0,\n";
    std::cout << "      \"shapes\": [\n";
    std::cout << "        {\n";
    std::cout << "          \"id\": 1,\n";
    std::cout << "          \"rect\": { \"x1\": 0, \"y1\": 0, \"x2\": 10, \"y2\": 10 }\n";
    std::cout << "        }\n";
    std::cout << "      ]\n";
    std::cout << "    }\n";
    std::cout << "  ]\n";
    std::cout << "}\n\n";
    std::cout << "Rules JSON Format:\n";
    std::cout << "{\n";
    std::cout << "  \"intersection\": true,\n";
    std::cout << "  \"minSpacing\": [\n";
    std::cout << "    { \"layer\": \"metal1\", \"minDist\": 10.0 }\n";
    std::cout << "  ],\n";
    std::cout << "  \"layers\": {\n";
    std::cout << "    \"metal1\": { \"minWidth\": 1000.0, \"minArea\": 100000.0 }\n";
    std::cout << "  }\n";
    std::cout << "}\n";
}

/// Main DRC demonstration.
int main(int argc, char* argv[])
{
    try
    {
        printHeader("DRC System Demonstration");

        std::cout << "\nInitializing DRC System...\n";

        // Step 1: Load or create layout
        std::shared_ptr<Layout> layout;
        std::vector<DrcRulePtr> parsedRules;
        // Create DRC Engine (rules will be registered below)
        auto engine = std::make_shared<DrcEngine>();

        std::string layoutFilename;
        std::string rulesFilename;
        for (int i = 1; i < argc; ++i)
        {
            if (std::strcmp(argv[i], "--rules") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::cerr << "Missing argument for --rules\n";
                    printUsage(argv[0]);
                    return 1;
                }
                rulesFilename = argv[++i];
                continue;
            }

            if (std::strcmp(argv[i], "--help") == 0 || std::strcmp(argv[i], "-h") == 0)
            {
                printUsage(argv[0]);
                return 0;
            }

            if (layoutFilename.empty())
            {
                layoutFilename = argv[i];
            }
            else
            {
                std::cerr << "Unexpected argument: " << argv[i] << "\n";
                printUsage(argv[0]);
                return 1;
            }
        }

        if (!layoutFilename.empty())
        {
            JsonLayoutLoader loader;
            RulesJsonLoader ruleLoader;
            std::cout << "Loading layout from file: " << layoutFilename << "\n";
            std::cout << "Layout Loader: " << loader.getDescription() << "\n";

            try
            {
                layout = loader.load(layoutFilename);
                if (!rulesFilename.empty())
                {
                    std::cout << "Loading rules from file: " << rulesFilename << "\n";
                    parsedRules = ruleLoader.load(rulesFilename, layout);
                }
                else
                {
                    auto pair = loader.loadWithRules(layoutFilename);
                    layout = pair.first;
                    parsedRules = pair.second;
                }

                if (!parsedRules.empty())
                {
                    std::cout << "Registering rules from JSON...\n";
                    for (const auto& r : parsedRules)
                    {
                        engine->registerRule(r);
                        std::cout << "  Added rule: " << r->getDescription() << "\n";
                    }
                }

                std::cout << "Layout loaded successfully.\n";
            }
            catch (const std::exception& e)
            {
                std::cerr << "Error loading layout or rules: " << e.what() << "\n";
                printUsage(argv[0]);
                return 1;
            }
        }
        else
        {
            // Create sample layout
            std::cout << "No layout file specified. Creating sample layout...\n";
            layout = std::make_shared<Layout>(createSampleLayout());
        }

        std::cout << "  Layers: " << layout->getLayerCount() << "\n";
        std::cout << "  Total Shapes: " << layout->getTotalShapeCount() << "\n";

        // Print layout summary
        for (const auto& [layerId, layer] : layout->getLayers())
        {
            std::cout << "  - " << layer.getName() << ": "
                      << layer.getShapeCount() << " shapes\n";
        }

        // If rules came from JSON we may still want to add some common defaults
        bool needIntersection = true;
        bool needSpacing = true;
        if (engine->getRuleCount() == 0)
        {
            // no rules at all -> behave exactly like GUI fallback
            std::cout << "\nCreating default DRC rules...\n";
            auto irule = std::make_shared<IntersectionRule>();
            engine->registerRule(irule);
            std::cout << "  Rule: " << irule->getDescription() << "\n";
            engine->registerRule(std::make_shared<MinSpacingRule>(10.0, 0));
            std::cout << "  Rule: " << std::make_shared<MinSpacingRule>(10.0, 0)->getDescription() << "\n";
            engine->registerRule(std::make_shared<MinSpacingRule>(5.0, 1));
            std::cout << "  Rule: " << std::make_shared<MinSpacingRule>(5.0, 1)->getDescription() << "\n";
            needIntersection = needSpacing = false; // nothing more needed
        }
        else
        {
            // check existing rules for these types
            for (const auto& r : engine->getRules())
            {
                if (!r)
                    continue;
                if (dynamic_cast<IntersectionRule*>(r.get()))
                    needIntersection = false;
                if (dynamic_cast<MinSpacingRule*>(r.get()))
                    needSpacing = false;
            }
            if (needIntersection)
            {
                auto irule = std::make_shared<IntersectionRule>();
                engine->registerRule(irule);
                std::cout << "  (added missing default) Rule: " << irule->getDescription() << "\n";
            }
            if (needSpacing)
            {
                auto sp1 = std::make_shared<MinSpacingRule>(10.0, 0);
                engine->registerRule(sp1);
                std::cout << "  (added missing default) Rule: " << sp1->getDescription() << "\n";
                auto sp2 = std::make_shared<MinSpacingRule>(5.0, 1);
                engine->registerRule(sp2);
                std::cout << "  (added missing default) Rule: " << sp2->getDescription() << "\n";
            }
        }

        std::cout << "\nRegistered " << engine->getRuleCount() << " rules\n";

        // Step 4: Execute DRC
        printSubheader("Executing DRC");
        std::cout << "Running DRC check...\n";
        DrcReport report = engine->run(*layout);

        // Step 5: Print Results
        printSubheader("DRC Results");

        std::cout << report.getSummary();

        // show violations count per rule for extra debugging
        if (!report.getRuleStatistics().empty())
        {
            std::cout << "\nRule breakdown:\n";
            for (const auto& [rule, stats] : report.getRuleStatistics())
            {
                std::cout << "  " << rule << ": " << stats.first << " violations\n";
            }
        }

        // Print detailed violations
        if (report.getViolationCount() > 0)
        {
            printSubheader("Detailed Violations");

            for (const auto& violation : report.getViolations())
            {
                std::cout << "\n" << violation.toString() << "\n";
                std::cout << "  Shape 1: " << violation.getShape1().getName() << " at "
                          << violation.getShape1().getBounds().min.x << ","
                          << violation.getShape1().getBounds().min.y << "\n";
                std::cout << "  Shape 2: " << violation.getShape2().getName() << " at "
                          << violation.getShape2().getBounds().min.x << ","
                          << violation.getShape2().getBounds().min.y << "\n";
            }
        }

        printHeader("DRC Demonstration Complete");
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}

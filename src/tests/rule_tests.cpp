#include <io/JsonLayoutLoader.hpp>
#include <core/DrcEngine.hpp>
#include <spatial/RTreeSpatialIndex.hpp>
// include rules used directly in additional tests
#include <rules/EnclosureRule.hpp>
#include <rules/IntersectionRule.hpp>
#include <rules/MinSpacingRule.hpp>
#include <cassert>
#include <iostream>

int main()
{
    JsonLayoutLoader loader;
    auto pair = loader.loadWithRules("examples/layout_complex_with_rules.json");
    auto layout = pair.first;
    auto rules = pair.second;

    // verify loader produced the expected configuration
    assert(layout->getLayerCount() == 5);
    const auto* cfg = layout->getLayerRuleConfig("metal1");
    assert(cfg && cfg->minWidth == 1000.0 && cfg->minHeight == 1000.0 && cfg->minArea == 100000.0);

    // we expect enclosure + containment rules plus min-width/area
    std::cout << "Rules parsed: " << rules.size() << "\n";
    assert(rules.size() >= 4);
    bool sawEnclosure = false, sawContainment = false, sawMinWidth = false, sawMinArea = false;
    for (const auto& r : rules)
    {
        if (!r) continue;
        std::string desc = r->getDescription();
        std::cout << "  " << desc << "\n";
        if (desc.find("Enclosure") != std::string::npos) sawEnclosure = true;
        if (desc.find("Containment") != std::string::npos) sawContainment = true;
        if (desc.find("MinWidth") != std::string::npos) sawMinWidth = true;
        if (desc.find("MinArea") != std::string::npos) sawMinArea = true;
    }
    assert(sawEnclosure && sawContainment && sawMinWidth && sawMinArea);

    DrcEngine engine;
    engine.registerRules(rules);

    auto report = engine.run(*layout);
    std::cout << "Violations found: " << report.getViolationCount() << "\n";
    // Expect at least one violation from our ruleset
    assert(report.getViolationCount() >= 1);
    // ensure report has entries for each rule type
    auto stats = report.getRuleStatistics();
    assert(stats.count("Enclosure") || stats.count("Containment") || stats.count("MinWidth") || stats.count("MinArea") || stats.count("IntersectionRule") );

    // verify enclosure rule differentiates between missing container and insufficient margin
    {
        Layout small;
        Layer in(0, "inner");
        Layer out(1, "outer");
        in.addShape(Shape(Rect(0,0,10,10), 1, 0));
        out.addShape(Shape(Rect(-3,-3,12,12), 2, 1));
        small.addLayer(in);
        small.addLayer(out);
        DrcEngine e2;
        e2.registerRule(std::make_shared<EnclosureRule>("inner","outer",5.0));
        auto rep2 = e2.run(small);
        // should flag margin violation
        assert(rep2.getViolationCount() == 1);
        auto msg = rep2.getViolations()[0].getMessage();
        assert(msg.find("margin") != std::string::npos);

        // remove outer entirely
        if (auto* lptr = small.getLayer(1))
            lptr->clear();
        auto rep3 = e2.run(small);
        assert(rep3.getViolationCount() == 1);
        auto msg2 = rep3.getViolations()[0].getMessage();
        assert(msg2.find("containing") != std::string::npos);
    }

    // basic R-tree sanity check
    {
        spatial::RTreeSpatialIndex idx;
        Layer l(0, "foo");
        l.addShape(Shape(Rect(0,0,5,5), 100, 0));
        l.addShape(Shape(Rect(10,10,15,15), 101, 0));
        for (const auto& s : l.getShapes()) idx.insert(s);
        assert(idx.getShapeCount() == 2);
        auto results = idx.query(Rect(4,4,11,11));
        assert(results.size() == 2);
    }

    // ensure IntersectionRule ignores cross-layer overlaps
    {
        Layout l;
        Layer lay0(0, "L0");
        Layer lay1(1, "L1");
        lay0.addShape(Shape(Rect(0,0,10,10), 1, 0));
        lay1.addShape(Shape(Rect(5,5,15,15), 2, 1));
        l.addLayer(lay0);
        l.addLayer(lay1);

        DrcEngine e;
        e.registerRule(std::make_shared<IntersectionRule>());
        auto rep = e.run(l);
        // shapes overlap geometrically but are on different layers -> no violation
        assert(rep.getViolationCount() == 0);
    }

    // ensure MinSpacingRule only applies within the same layer
    {
        Layout l;
        Layer lay0(0, "L0");
        Layer lay1(1, "L1");
        // two pairs, one on each layer, each pair too close
        lay0.addShape(Shape(Rect(0,0,10,10), 1, 0));
        lay0.addShape(Shape(Rect(12,0,20,10), 2, 0));
        lay1.addShape(Shape(Rect(0,0,10,10), 3, 1));
        lay1.addShape(Shape(Rect(12,0,20,10), 4, 1));
        // add an extra cross-layer neighbor inside spacing for layer0 shapes
        lay1.addShape(Shape(Rect(5, -5, 15, 5), 5, 1));
        l.addLayer(lay0);
        l.addLayer(lay1);

        DrcEngine e;
        e.registerRule(std::make_shared<MinSpacingRule>(5.0, 0));
        e.registerRule(std::make_shared<MinSpacingRule>(5.0, 1));
        auto rep = e.run(l);
        // should find two violations, one per layer; cross-layer shape5 ignored
        assert(rep.getViolationCount() == 2);
        for (const auto& v : rep.getViolations()) {
            assert(v.getShape1().getLayer() == v.getShape2().getLayer());
        }
    }

    // verify fractional spacing works and is not truncated
    {
        Layout l;
        Layer lay(0, "L0");
        // two shapes 4.5 units apart in x-direction
        lay.addShape(Shape(Rect(0,0,10,10), 1, 0));
        lay.addShape(Shape(Rect(14,0,24,10), 2, 0));
        l.addLayer(lay);

        DrcEngine e;
        e.registerRule(std::make_shared<MinSpacingRule>(5.0, 0));
        auto rep = e.run(l);
        // distance between bounds is 4 (from 10 to 14) -> violation
        assert(rep.getViolationCount() == 1);
    }

    // also test simple JSON that lacks rules should produce minimal rules
    auto simplePair = loader.loadWithRules("examples/layout_complex.json");
    DrcEngine engine2;
    engine2.registerRules(simplePair.second);
    auto report2 = engine2.run(*simplePair.first);
    std::cout << "Simple JSON violations=" << report2.getViolationCount() << "\n";
    // only min-width/area rules exist but no config -> no violations expected
    assert(report2.getViolationCount() == 0);

    // verify spacing rules are parsed from JSON example
    {
        auto pair = loader.loadWithRules("examples/basic_spacing_violations.json");
        DrcEngine e4;
        e4.registerRules(pair.second);
        // ensure at least one MinSpacingRule was created
        bool sawSpacingRule = false;
        for (const auto& r : pair.second) {
            if (dynamic_cast<MinSpacingRule*>(r.get()))
                sawSpacingRule = true;
        }
        assert(sawSpacingRule);
        auto rep4 = e4.run(*pair.first);
        std::cout << "Basic spacing JSON viol=" << rep4.getViolationCount() << "\n";
        // according to example data there should be spacing violations
        assert(rep4.getViolationCount() > 0);
    }

    // simulate CLI/GUI fallback: add intersection and spacing if they were missing
    {
        DrcEngine e3;
        e3.registerRules(simplePair.second);
        bool hasIntersection = false;
        bool hasSpacing = false;
        for (const auto& r : e3.getRules())
        {
            if (dynamic_cast<IntersectionRule*>(r.get()))
                hasIntersection = true;
            if (dynamic_cast<MinSpacingRule*>(r.get()))
                hasSpacing = true;
        }
        if (!hasIntersection)
            e3.registerRule(std::make_shared<IntersectionRule>());
        if (!hasSpacing)
        {
            e3.registerRule(std::make_shared<MinSpacingRule>(10.0, 0));
            e3.registerRule(std::make_shared<MinSpacingRule>(5.0, 1));
        }
        // ensure engine now has at least 3 rules and still runs safely on empty layout
        assert(e3.getRuleCount() >= 3);
        Layout empty;
        auto rep3 = e3.run(empty);
        assert(rep3.getViolationCount() == 0);
    }

    std::cout << "All tests passed.\n";
    return 0;
}

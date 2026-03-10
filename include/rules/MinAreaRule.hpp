#pragma once

#include <drc/DrcRule.hpp>
#include <layout/Layout.hpp>
#include <vector>
#include <string>

class MinAreaRule : public DrcRule
{
public:
    MinAreaRule() : DrcRule("MinArea") {}

    std::vector<DrcViolation> check(const DrcContext& context) const override
    {
        std::vector<DrcViolation> violations;
        int vid = 0;

        const Layout& layout = context.getLayout();

        for (const auto& [layerId, layer] : layout.getLayers())
        {
            const auto cfgPtr = layout.getLayerRuleConfig(layer.getName());
            if (!cfgPtr)
                continue;

            double minArea = cfgPtr->minArea;

            for (const auto& shape : layer.getShapes())
            {
                double area = static_cast<double>(shape.getBounds().area());
                if (area + 1e-9 < minArea)
                {
                    std::string msg = "Area too small: " + std::to_string(static_cast<int>(area));
                    violations.push_back(makeViolation(DrcViolation::ViolationType::MinArea,
                                                      vid++, shape, shape, msg));
                }
            }
        }

        return violations;
    }

    std::string getDescription() const override
    {
        return "MinArea per-layer rules";
    }
};

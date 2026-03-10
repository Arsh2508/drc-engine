#pragma once

#include <drc/DrcRule.hpp>
#include <layout/Layout.hpp>
#include <vector>
#include <string>

class MinWidthRule : public DrcRule
{
public:
    MinWidthRule() : DrcRule("MinWidth") {}

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

            double minW = cfgPtr->minWidth;
            double minH = cfgPtr->minHeight;

            for (const auto& shape : layer.getShapes())
            {
                // compute width/height using absolute difference to guard against
                // any inverted coordinates (should not happen, but helps debugging)
                double w = std::abs(static_cast<double>(shape.getBounds().width()));
                double h = std::abs(static_cast<double>(shape.getBounds().height()));

                // debug logging for each shape
                std::cout << "[MinWidthRule] shape=" << shape.getId()
                          << " width=" << w << " height=" << h
                          << " minW=" << minW << " minH=" << minH << std::endl;

                if (w + 1e-9 < minW || h + 1e-9 < minH)
                {
                    std::string msg = "Shape too small: w=" + std::to_string(w) +
                                      " h=" + std::to_string(h) +
                                      " required (" + std::to_string(minW) + "," + std::to_string(minH) + ")";
                    violations.push_back(makeViolation(DrcViolation::ViolationType::MinWidth,
                                                      vid++, shape, shape, msg));
                }
            }
        }

        return violations;
    }

    std::string getDescription() const override
    {
        return "MinWidth/Height per-layer rules";
    }
};

#pragma once

#include <drc/DrcRule.hpp>
#include <geometry/GeometryUtils.hpp>

// No intersection rule for same-layer shapes
class IntersectionRule : public DrcRule
{
public:
    IntersectionRule()
        : DrcRule("IntersectionRule")
    {
    }

    std::vector<DrcViolation> check(const DrcContext& context) const override
    {
        std::vector<DrcViolation> violations;
        int violationId = 0;

        const Layout& layout = context.getLayout();
        const SpatialIndex& index = context.getSpatialIndex();

        // Get all shapes
        const auto allShapes = layout.getAllShapes();

        if (allShapes.empty())
            return violations;

        // Check each shape for intersections
        for (const auto& shape1 : allShapes)
        {
            const Rect& bounds1 = shape1.getBounds();

            // Query spatial index for overlapping shapes
            auto candidateShapes = index.query(bounds1);

            // Check each candidate
            for (const auto& shape2 : candidateShapes)
            {
                // Skip self
                if (shape1.getId() == shape2.getId())
                    continue;

                // Same layer only
                if (shape1.getLayer() != shape2.getLayer())
                    continue;

                // Avoid duplicate reports
                if (shape1.getId() > shape2.getId())
                    continue;

                const Rect& bounds2 = shape2.getBounds();

                // Check for intersection
                if (GeometryUtils::rectsOverlap(bounds1, bounds2))
                {
                    // Strict intersection (not just touching)
                    if (GeometryUtils::rectsStrictlyOverlap(bounds1, bounds2))
                    {
                        std::string message =
                            "Shapes " + std::to_string(shape1.getId()) +
                            " and " + std::to_string(shape2.getId()) +
                            " intersect";

                        violations.push_back(
                            makeViolation(
                                DrcViolation::ViolationType::Other,
                                violationId++,
                                shape1,
                                shape2,
                                message
                            )
                        );
                    }
                }
            }
        }

        return violations;
    }

    std::string getDescription() const override
    {
        return "IntersectionRule: Detects shape intersections";
    }
};

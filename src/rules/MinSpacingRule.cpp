#include <rules/MinSpacingRule.hpp>

#include <geometry/GeometryUtils.hpp>
#include <spatial/BoostGeometryAdapters.hpp>

#include <boost/geometry.hpp>

#include <string>
#include <vector>

MinSpacingRule::MinSpacingRule(double minSpacing, int layer)
    : DrcRule("MinSpacing"), m_minSpacing(minSpacing), m_layer(layer)
{
}

std::vector<DrcViolation> MinSpacingRule::check(const DrcContext& context) const
{
    std::vector<DrcViolation> violations;
    int violationId = 0;

    const Layout& layout = context.getLayout();
    const SpatialIndex& index = context.getSpatialIndex();

    // Get shapes on target layer
    const auto shapes = layout.getShapesByLayer(m_layer);

    if (shapes.empty())
        return violations;

    // Check spacing for each shape
    for (const auto& shape1 : shapes)
    {
        const Rect& bounds1 = shape1.getBounds();

        // Expand query box for spatial search
        Rect expandedBox = bounds1.expanded(m_minSpacing);

        // Find nearby shapes
        auto nearbyShapes = index.query(expandedBox);

        // Check each nearby shape for violations
        for (const auto& shape2 : nearbyShapes)
        {
            // Same layer only
            if (shape2.getLayer() != m_layer)
                continue;

            // Skip self
            if (shape1.getId() == shape2.getId())
                continue;

            // Avoid duplicate reports
            if (shape1.getId() > shape2.getId())
                continue;

            const Rect& bounds2 = shape2.getBounds();

            // Skip intersecting shapes (handled by IntersectionRule)
            if (GeometryUtils::rectsOverlap(bounds1, bounds2))
                continue;

            // Calculate distance
            spatial::BoostBox box1 = spatial::rectToBoostBox(bounds1);
            spatial::BoostBox box2 = spatial::rectToBoostBox(bounds2);
            double distance = spatial::bg::distance(box1, box2);

            // Check for violation
            bool violation = (distance < m_minSpacing - 1e-9);
            if (violation)
            {
                std::string message =
                    "Spacing between shape " + std::to_string(shape1.getId()) +
                    " and shape " + std::to_string(shape2.getId()) +
                    " is " + std::to_string(distance) +
                    ", minimum required: " + std::to_string(m_minSpacing);

                violations.push_back(
                    makeViolation(
                        DrcViolation::ViolationType::MinSpacing,
                        violationId++,
                        shape1,
                        shape2,
                        message));
            }
        }
    }

    return violations;
}

std::string MinSpacingRule::getDescription() const
{
    return "MinSpacing: " + std::to_string(m_minSpacing) +
           " units on layer " + std::to_string(m_layer);
}


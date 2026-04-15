#include <rules/IntersectionRule.hpp>

#include <geometry/GeometryUtils.hpp>

#include <string>
#include <vector>

IntersectionRule::IntersectionRule()
    : DrcRule("IntersectionRule")
{
}

std::vector<DrcViolation> IntersectionRule::check(const DrcContext& context) const
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
                bool shapesIntersect = false;
                if (shape1.hasPoints() || shape2.hasPoints())
                {
                    // Get points for shape1
                    std::vector<Point> points1 = shape1.hasPoints() ? shape1.getPoints() :
                        std::vector<Point>{{bounds1.min.x, bounds1.min.y}, {bounds1.max.x, bounds1.min.y},
                                          {bounds1.max.x, bounds1.max.y}, {bounds1.min.x, bounds1.max.y}};

                    // Get points for shape2
                    std::vector<Point> points2 = shape2.hasPoints() ? shape2.getPoints() :
                        std::vector<Point>{{bounds2.min.x, bounds2.min.y}, {bounds2.max.x, bounds2.min.y},
                                          {bounds2.max.x, bounds2.max.y}, {bounds2.min.x, bounds2.max.y}};

                    // Use polygon intersection
                    shapesIntersect = GeometryUtils::polygonsIntersect(points1, points2);
                }
                else
                {
                    // Use bounding box intersection for rectangles
                    shapesIntersect = GeometryUtils::rectsStrictlyOverlap(bounds1, bounds2);
                }

                if (shapesIntersect)
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
                            message));
                }
            }
        }
    }

    return violations;
}

std::string IntersectionRule::getDescription() const
{
    return "IntersectionRule: Detects shape intersections";
}


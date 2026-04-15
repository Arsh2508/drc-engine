#include <rules/EnclosureRule.hpp>

#include <spatial/BoostGeometryAdapters.hpp>

#include <boost/geometry.hpp>

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>

namespace bg = boost::geometry;

EnclosureRule::EnclosureRule(const std::string& innerLayerName,
                             const std::string& outerLayerName,
                             double minMargin)
    : DrcRule("Enclosure"),
      m_inner(innerLayerName),
      m_outer(outerLayerName),
      m_minMargin(minMargin)
{
}

namespace {

/// @brief Helper function to check strict containment with margin.
/// 
/// Enclosure is defined as: outer must fully contain (inner expanded by margin).
/// This is NOT intersection or touching edges - it's strict containment with margin.
/// 
/// Algorithm:
/// 1. Expand inner shape by margin in all directions
/// 2. Check if outer shape fully contains the expanded inner
/// 
/// @param outer The outer (enclosing) shape
/// @param inner The inner (to-be-enclosed) shape
/// @param margin The minimum margin requirement
/// @return true if outer strictly contains (inner expanded by margin)
inline bool containsWithMargin(const Shape& outer, const Shape& inner, double margin)
{
    // Convert shapes to Boost polygons for robust geometry operations
    auto outerPoly = spatial::shapeToBoostPolygon(outer);
    
    // For the inner shape, we need to expand it by the margin first
    // Then check if outer contains the expanded inner
    const Rect& innerBounds = inner.getBounds();
    
    // Expand inner bounds by margin
    // Using floor/ceil ensures we expand conservatively
    int expandedMinX = static_cast<int>(std::floor(static_cast<double>(innerBounds.min.x) - margin));
    int expandedMinY = static_cast<int>(std::floor(static_cast<double>(innerBounds.min.y) - margin));
    int expandedMaxX = static_cast<int>(std::ceil(static_cast<double>(innerBounds.max.x) + margin));
    int expandedMaxY = static_cast<int>(std::ceil(static_cast<double>(innerBounds.max.y) + margin));
    
    // Create expanded rectangle as a Boost polygon
    std::vector<spatial::BoostPoint> expandedPoints = {
        spatial::BoostPoint(expandedMinX, expandedMinY),
        spatial::BoostPoint(expandedMaxX, expandedMinY),
        spatial::BoostPoint(expandedMaxX, expandedMaxY),
        spatial::BoostPoint(expandedMinX, expandedMaxY),
        spatial::BoostPoint(expandedMinX, expandedMinY)
    };
    
    spatial::BoostPolygon expandedPoly;
    for (const auto& pt : expandedPoints)
    {
        bg::append(expandedPoly.outer(), pt);
    }
    bg::correct(expandedPoly);
    
    // Check if outer polygon contains the expanded inner polygon
    // Uses the "within" predicate: every point of expanded inner must be inside or on boundary of outer
    // For strict enclosure with margin, this is correct:
    // - If expanded_inner is within outer, then the margin requirement is met
    // - The expanded_inner is constructed to require the margin
    
    return bg::within(expandedPoly, outerPoly);
}

} // anonymous namespace

std::vector<DrcViolation> EnclosureRule::check(const DrcContext& context) const
{
    std::vector<DrcViolation> violations;
    int vid = 0;

    const Layout& layout = context.getLayout();
    const SpatialIndex& index = context.getSpatialIndex();

    auto innerId = layout.getLayerIdByName(m_inner);
    auto outerId = layout.getLayerIdByName(m_outer);
    
    if (innerId < 0 || outerId < 0)
        return violations; // Nothing to check (invalid layer names)

    auto innerShapes = layout.getShapesByLayer(innerId);

    for (const auto& inner : innerShapes)
    {
        const Rect& innerBounds = inner.getBounds();
        
        // Query for candidate outer shapes
        // Use expanded query box to find all shapes that could potentially enclose this inner shape
        // Expand by margin to catch shapes just outside the inner boundary
        Rect queryBox = innerBounds.expanded(m_minMargin);
        auto candidates = index.query(queryBox);

        bool foundValidEnclosure = false;
        bool foundContainingShape = false;

        for (const auto& candidate : candidates)
        {
            // Filter to only outer layer shapes
            if (candidate.getLayer() != outerId)
                continue;

            // Check if this candidate outer shape contains the inner shape (at all, any margin)
            if (containsWithMargin(candidate, inner, 0.0))
            {
                foundContainingShape = true;
            }

            // Check if this candidate outer shape contains the inner shape with the required margin
            if (containsWithMargin(candidate, inner, m_minMargin))
            {
                foundValidEnclosure = true;
                break; // Found valid enclosure, no need to check more candidates
            }
        }

        // Report violation if no valid enclosure found
        if (!foundValidEnclosure)
        {
            std::string msg;
            if (!foundContainingShape)
            {
                // No outer shape contains this inner shape at all
                msg = "No containing shape from layer '" + m_outer + 
                      "' found for inner shape from layer '" + m_inner + "'";
            }
            else
            {
                // Outer shapes exist and contain inner, but margin is insufficient
                msg = "Enclosure margin requirement of " + std::to_string(m_minMargin) + 
                      " not met: inner shape from layer '" + m_inner + 
                      "' is not sufficiently enclosed by layer '" + m_outer + "'";
            }
            
            violations.push_back(makeViolation(
                DrcViolation::ViolationType::EnclosureError,
                vid++,
                inner,
                inner,
                msg));
        }
    }

    return violations;
}

std::string EnclosureRule::getDescription() const
{
    return "Enclosure: " + m_inner + " in " + m_outer +
           " margin=" + std::to_string(m_minMargin);
}


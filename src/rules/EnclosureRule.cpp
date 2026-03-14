#include <rules/EnclosureRule.hpp>

#include <spatial/BoostGeometryAdapters.hpp>

#include <boost/geometry.hpp>

#include <algorithm>
#include <string>
#include <vector>

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

std::vector<DrcViolation> EnclosureRule::check(const DrcContext& context) const
{
    std::vector<DrcViolation> violations;
    int vid = 0;

    const Layout& layout = context.getLayout();
    const SpatialIndex& index = context.getSpatialIndex();

    auto innerId = layout.getLayerIdByName(m_inner);
    auto outerId = layout.getLayerIdByName(m_outer);
    if (innerId < 0 || outerId < 0)
        return violations; // Nothing to check

    auto innerShapes = layout.getShapesByLayer(innerId);

    for (const auto& inner : innerShapes)
    {
        const Rect& ib = inner.getBounds();
        // Query candidates using inner bounds (outer must at least intersect/contain).
        // NOTE: the query rectangle is not expanded by margin here because any
        // outer shape that contains the inner box will necessarily intersect it.
        auto candidates = index.query(ib);

        bool foundContaining = false;
        bool foundValidOuter = false;
        for (const auto& cand : candidates)
        {
            if (cand.getLayer() != outerId)
                continue;

            // Use boost boxes for within test
            auto innerBox = spatial::rectToBoostBox(ib);
            auto outerBox = spatial::rectToBoostBox(cand.getBounds());

            if (!bg::within(innerBox, outerBox))
                continue;

            // we have an outer shape that actually contains the inner one
            foundContaining = true;

            // Compute minimum margin between inner and outer boundaries (axis-aligned)
            double marginLeft = static_cast<double>(ib.min.x - cand.getBounds().min.x);
            double marginRight = static_cast<double>(cand.getBounds().max.x - ib.max.x);
            double marginBottom = static_cast<double>(ib.min.y - cand.getBounds().min.y);
            double marginTop = static_cast<double>(cand.getBounds().max.y - ib.max.y);

            double minFound = std::min({marginLeft, marginRight, marginBottom, marginTop});

            if (minFound + 1e-9 >= m_minMargin)
            {
                foundValidOuter = true;
                break; // no need to check more candidates
            }
        }

        if (!foundContaining)
        {
            // no outer shape at all contained this inner shape
            std::string msg = "No containing outer layer found (" + m_outer + ")";
            violations.push_back(makeViolation(DrcViolation::ViolationType::EnclosureError,
                                               vid++,
                                               inner,
                                               inner,
                                               msg));
        }
        else if (!foundValidOuter)
        {
            // there was at least one container but none satisfied the margin
            std::string msg = "Enclosure margin < " + std::to_string(m_minMargin);
            violations.push_back(makeViolation(DrcViolation::ViolationType::EnclosureError,
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


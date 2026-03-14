#include <rules/ContainmentRule.hpp>

#include <spatial/BoostGeometryAdapters.hpp>

#include <boost/geometry.hpp>

#include <string>
#include <vector>

namespace bg = boost::geometry;

ContainmentRule::ContainmentRule(const std::string& layerA, const std::string& layerB)
    : DrcRule("Containment"), m_a(layerA), m_b(layerB)
{
}

std::vector<DrcViolation> ContainmentRule::check(const DrcContext& context) const
{
    std::vector<DrcViolation> violations;
    int vid = 0;

    const Layout& layout = context.getLayout();
    const SpatialIndex& index = context.getSpatialIndex();

    auto aId = layout.getLayerIdByName(m_a);
    auto bId = layout.getLayerIdByName(m_b);
    if (aId < 0 || bId < 0)
        return violations;

    auto shapesA = layout.getShapesByLayer(aId);
    for (const auto& a : shapesA)
    {
        auto candidates = index.query(a.getBounds());
        for (const auto& cand : candidates)
        {
            if (cand.getLayer() != bId)
                continue;

            auto boxA = spatial::rectToBoostBox(a.getBounds());
            auto boxB = spatial::rectToBoostBox(cand.getBounds());

            if (bg::within(boxA, boxB))
            {
                std::string msg = m_a + " contained inside " + m_b;
                violations.push_back(makeViolation(DrcViolation::ViolationType::Other,
                                                   vid++,
                                                   a,
                                                   cand,
                                                   msg));
            }
        }
    }

    return violations;
}

std::string ContainmentRule::getDescription() const
{
    return "Containment: " + m_a + " not in " + m_b;
}


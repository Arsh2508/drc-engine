#pragma once

#include <drc/DrcRule.hpp>
#include <string>

// Minimum spacing rule for same-layer shapes
class MinSpacingRule : public DrcRule
{
public:
    MinSpacingRule(double minSpacing, int layer);

    std::vector<DrcViolation> check(const DrcContext& context) const override;

    std::string getDescription() const override;

private:
    double m_minSpacing; // Minimum spacing requirement
    int m_layer;         // Layer to check
};

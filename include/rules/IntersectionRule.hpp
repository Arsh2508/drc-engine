#pragma once

#include <drc/DrcRule.hpp>
#include <string>

// No intersection rule for same-layer shapes
class IntersectionRule : public DrcRule
{
public:
    IntersectionRule();

    std::vector<DrcViolation> check(const DrcContext& context) const override;

    std::string getDescription() const override;
};

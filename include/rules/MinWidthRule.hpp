#pragma once

#include <drc/DrcRule.hpp>
#include <string>

class MinWidthRule : public DrcRule
{
public:
    MinWidthRule();

    std::vector<DrcViolation> check(const DrcContext& context) const override;

    std::string getDescription() const override;
};

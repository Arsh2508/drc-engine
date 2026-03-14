#pragma once

#include <drc/DrcRule.hpp>
#include <string>

class MinAreaRule : public DrcRule
{
public:
    MinAreaRule();

    std::vector<DrcViolation> check(const DrcContext& context) const override;

    std::string getDescription() const override;
};

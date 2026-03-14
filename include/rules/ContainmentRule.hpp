#pragma once

#include <drc/DrcRule.hpp>
#include <string>

class ContainmentRule : public DrcRule
{
public:
    ContainmentRule(const std::string& layerA, const std::string& layerB);

    std::vector<DrcViolation> check(const DrcContext& context) const override;

    std::string getDescription() const override;

private:
    std::string m_a;
    std::string m_b;
};

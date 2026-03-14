#pragma once

#include <drc/DrcRule.hpp>
#include <string>

class EnclosureRule : public DrcRule
{
public:
    EnclosureRule(const std::string& innerLayerName,
                  const std::string& outerLayerName,
                  double minMargin);

    std::vector<DrcViolation> check(const DrcContext& context) const override;

    std::string getDescription() const override;

private:
    std::string m_inner;
    std::string m_outer;
    double m_minMargin;
};

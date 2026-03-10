#pragma once

#include "DrcContext.hpp"
#include "DrcViolation.hpp"
#include <vector>
#include <string>
#include <memory>

// Abstract base class for DRC rules
class DrcRule
{
public:
    explicit DrcRule(const std::string& name) : m_name(name) {}

    virtual ~DrcRule() = default;

    virtual std::vector<DrcViolation> check(const DrcContext& context) const = 0;

    const std::string& getName() const { return m_name; }

    virtual std::string getDescription() const { return m_name; }

protected:
    DrcViolation makeViolation(DrcViolation::ViolationType type, int violationIndex,
                               const Shape& shape1, const Shape& shape2,
                               const std::string& message) const
    {
        return DrcViolation(violationIndex, type, m_name, shape1, shape2, message);
    }

private:
    std::string m_name; // Name of this rule
};

using DrcRulePtr = std::shared_ptr<DrcRule>;

#pragma once

#include <layout/Shape.hpp>
#include <string>

class DrcViolation
{
public:
    using ViolationId = int;

    enum class ViolationType
    {
        MinSpacing,       
        MinWidth,         
        MinArea,          
        DensityViolation, 
        EnclosureError,   
        Other             
    };

    DrcViolation(ViolationId violationId, ViolationType violationType,
                 const std::string& ruleName, const Shape& shape1,
                 const Shape& shape2, const std::string& message)
        : m_id(violationId), m_type(violationType), m_ruleName(ruleName),
          m_shape1(shape1), m_shape2(shape2), m_message(message)
    {
    }

    ViolationId getId() const { return m_id; }

    ViolationType getType() const { return m_type; }

    const std::string& getRuleName() const { return m_ruleName; }

    const Shape& getShape1() const { return m_shape1; }

    const Shape& getShape2() const { return m_shape2; }

    const std::string& getMessage() const { return m_message; }

    std::string getTypeString() const
    {
        switch (m_type)
        {
        case ViolationType::MinSpacing:
            return "MinSpacing";
        case ViolationType::MinWidth:
            return "MinWidth";
        case ViolationType::MinArea:
            return "MinArea";
        case ViolationType::DensityViolation:
            return "DensityViolation";
        case ViolationType::EnclosureError:
            return "EnclosureError";
        default:
            return "Other";
        }
    }

    /// @brief Get a formatted string representation of the violation.
    std::string toString() const
    {
        return "Violation #" + std::to_string(m_id) + " [" + getTypeString() +
               "] Rule: " + m_ruleName + " - " + m_message;
    }

private:
    ViolationId m_id;
    ViolationType m_type;
    std::string m_ruleName;
    Shape m_shape1;
    Shape m_shape2;
    std::string m_message;
};

#pragma once

#include <layout/Shape.hpp>
#include <string>

/// @brief Represents a single design rule violation found during DRC.
/// Contains detailed information about the violation for debugging and reporting.
class DrcViolation
{
public:
    using ViolationId = int;

    /// @brief Types of DRC violations.
    enum class ViolationType
    {
        MinSpacing,       ///< Minimum spacing violation
        MinWidth,         ///< Minimum width violation
        MinArea,          ///< Minimum area violation
        DensityViolation, ///< Density checking violation
        EnclosureError,   ///< Enclosure rule violation
        Other             ///< Other/custom violation types
    };

    /// @brief Construct a DRC violation.
    /// @param violationId Unique violation identifier
    /// @param violationType Type of violation
    /// @param ruleName Name of the rule that detected this violation
    /// @param shape1 Primary shape involved in violation
    /// @param shape2 Secondary shape involved in violation (if applicable)
    /// @param message Detailed violation message
    DrcViolation(ViolationId violationId, ViolationType violationType,
                 const std::string& ruleName, const Shape& shape1,
                 const Shape& shape2, const std::string& message)
        : m_id(violationId), m_type(violationType), m_ruleName(ruleName),
          m_shape1(shape1), m_shape2(shape2), m_message(message)
    {
    }

    /// @brief Get violation ID.
    ViolationId getId() const { return m_id; }

    /// @brief Get violation type.
    ViolationType getType() const { return m_type; }

    /// @brief Get rule name that detected this violation.
    const std::string& getRuleName() const { return m_ruleName; }

    /// @brief Get primary shape involved.
    const Shape& getShape1() const { return m_shape1; }

    /// @brief Get secondary shape involved.
    const Shape& getShape2() const { return m_shape2; }

    /// @brief Get violation message.
    const std::string& getMessage() const { return m_message; }

    /// @brief Get violation type as string.
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
    ViolationId m_id;        ///< Unique violation ID
    ViolationType m_type;    ///< Type of violation
    std::string m_ruleName;  ///< Name of the rule that failed
    Shape m_shape1;          ///< Primary shape involved
    Shape m_shape2;          ///< Secondary shape involved (may be duplicate if not applicable)
    std::string m_message;   ///< Detailed violation message
};

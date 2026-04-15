#pragma once

#include <drc/DrcRule.hpp>
#include <string>

/// @brief Enclosure rule: inner layer shapes must be enclosed by outer layer shapes with margin.
///
/// Definition: A shape A (inner) is enclosed by shape B (outer) with margin d if:
///   - B completely contains (A expanded by d in all directions)
///
/// Algorithm:
///  1. For each inner shape on innerLayer:
///     - Expand it by margin d in all directions
///     - Query spatial index for candidate outer shapes
///     - Check strict containment: outer must fully contain the expanded inner
///  2. Report violations for inner shapes that have NO valid enclosure
///
/// Key Properties:
///  - Simple intersection is NOT enclosure
///  - Touching edges is NOT sufficient (requires margin)
///  - If multiple outer shapes exist, only ONE valid enclosure is required
///  - Performance: Uses spatial index to avoid O(N²) comparisons
///
/// Violations reported as:
///  - "No containing shape found" if no outer shape contains the inner shape at all
///  - "Margin requirement not met" if outer shapes exist but margin is insufficient
///
/// Internal helpers:
///  - containsWithMargin(outer, inner, margin): Checks if outer strictly contains (inner expanded by margin)
///    Uses Boost.Geometry for robust polygon containment checking
///
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

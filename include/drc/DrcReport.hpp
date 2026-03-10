#pragma once

#include "DrcViolation.hpp"
#include <vector>
#include <map>
#include <chrono>

/// @brief Container for all DRC violations found in a design.
/// Also provides high-level statistics and summary information about the DRC run.
class DrcReport
{
public:
    /// @brief Construct an empty DRC report.
    DrcReport()
        : m_violationCount(0), m_durationMs(0),
          m_shapesChecked(0), m_rulesExecuted(0)
    {
    }

    /// @brief Add a violation to the report.
    /// @param violation DRC violation to add
    void addViolation(const DrcViolation& violation)
    {
        m_violations.push_back(violation);
        m_violationCount++;

        // Update statistics by rule
        auto& ruleStats = m_ruleStatistics[violation.getRuleName()];
        ruleStats.first++;  // Count
        ruleStats.second = std::max(ruleStats.second, static_cast<int>(violation.getId()));
    }

    /// @brief Add multiple violations to the report.
    /// @param violations Vector of violations to add
    void addViolations(const std::vector<DrcViolation>& violations)
    {
        for (const auto& violation : violations)
        {
            addViolation(violation);
        }
    }

    /// @brief Get all violations.
    const std::vector<DrcViolation>& getViolations() const { return m_violations; }

    /// @brief Get violations for a specific rule.
    /// @param ruleName Name of the rule to filter by
    /// @return Vector of violations from the specified rule
    std::vector<DrcViolation> getViolationsByRule(const std::string& ruleName) const
    {
        std::vector<DrcViolation> result;
        for (const auto& violation : m_violations)
        {
            if (violation.getRuleName() == ruleName)
                result.push_back(violation);
        }
        return result;
    }

    /// @brief Get violations of a specific type.
    /// @param type Type of violations to filter by
    /// @return Vector of violations of the specified type
    std::vector<DrcViolation> getViolationsByType(DrcViolation::ViolationType type) const
    {
        std::vector<DrcViolation> result;
        for (const auto& violation : m_violations)
        {
            if (violation.getType() == type)
                result.push_back(violation);
        }
        return result;
    }

    /// @brief Get total number of violations.
    size_t getViolationCount() const { return m_violationCount; }

    /// @brief Check if design passed DRC (no violations).
    bool passed() const { return m_violationCount == 0; }

    /// @brief Set the DRC runtime in milliseconds.
    /// @param ms Duration of DRC run in milliseconds
    void setDurationMs(long long ms) { m_durationMs = ms; }

    /// @brief Get DRC runtime in milliseconds.
    long long getDurationMs() const { return m_durationMs; }

    /// @brief Set number of shapes that were checked.
    /// @param count Number of shapes
    void setShapesChecked(size_t count) { m_shapesChecked = count; }

    /// @brief Get number of shapes that were checked.
    size_t getShapesChecked() const { return m_shapesChecked; }

    /// @brief Set number of rules that were executed.
    /// @param count Number of rules
    void setRulesExecuted(size_t count) { m_rulesExecuted = count; }

    /// @brief Get number of rules that were executed.
    size_t getRulesExecuted() const { return m_rulesExecuted; }

    /// @brief Get statistics by rule name.
    /// Returns map of rule name to (violation count, max violation ID)
    const std::map<std::string, std::pair<int, int>>& getRuleStatistics() const
    {
        return m_ruleStatistics;
    }

    /// @brief Get a formatted summary string of the report.
    std::string getSummary() const
    {
        std::string summary =
            "DRC Report Summary\n"
            "==================\n"
            "Violations: " + std::to_string(m_violationCount) + "\n"
            "Shapes Checked: " + std::to_string(m_shapesChecked) + "\n"
            "Rules Executed: " + std::to_string(m_rulesExecuted) + "\n"
            "Duration: " + std::to_string(m_durationMs) + " ms\n"
            "Status: " + (passed() ? "PASS" : "FAIL") + "\n";

        if (!m_ruleStatistics.empty())
        {
            summary += "\nViolations by Rule:\n";
            for (const auto& [ruleName, stats] : m_ruleStatistics)
            {
                summary += "  " + ruleName + ": " + std::to_string(stats.first) + "\n";
            }
        }

        return summary;
    }

    /// @brief Clear all violations and reset statistics.
    void clear()
    {
        m_violations.clear();
        m_violationCount = 0;
        m_durationMs = 0;
        m_shapesChecked = 0;
        m_rulesExecuted = 0;
        m_ruleStatistics.clear();
    }

private:
    std::vector<DrcViolation> m_violations; ///< All violations found
    size_t m_violationCount;               ///< Total violation count
    long long m_durationMs;                ///< DRC runtime in milliseconds
    size_t m_shapesChecked;                ///< Number of shapes checked
    size_t m_rulesExecuted;                ///< Number of rules executed
    
    /// Maps rule names to (violation_count, max_violation_id)
    std::map<std::string, std::pair<int, int>> m_ruleStatistics;
};

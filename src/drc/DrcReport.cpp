#include <drc/DrcReport.hpp>

#include <algorithm>
#include <map>
#include <string>
#include <utility>
#include <vector>

DrcReport::DrcReport()
    : m_violationCount(0),
      m_durationMs(0),
      m_shapesChecked(0),
      m_rulesExecuted(0)
{
}

void DrcReport::addViolation(const DrcViolation& violation)
{
    m_violations.push_back(violation);
    m_violationCount++;

    // Update statistics by rule
    auto& ruleStats = m_ruleStatistics[violation.getRuleName()];
    ruleStats.first++; // Count
    ruleStats.second = std::max(ruleStats.second, static_cast<int>(violation.getId()));
}

void DrcReport::addViolations(const std::vector<DrcViolation>& violations)
{
    for (const auto& violation : violations)
    {
        addViolation(violation);
    }
}

std::vector<DrcViolation> DrcReport::getViolationsByRule(const std::string& ruleName) const
{
    std::vector<DrcViolation> result;
    for (const auto& violation : m_violations)
    {
        if (violation.getRuleName() == ruleName)
            result.push_back(violation);
    }
    return result;
}

std::vector<DrcViolation> DrcReport::getViolationsByType(DrcViolation::ViolationType type) const
{
    std::vector<DrcViolation> result;
    for (const auto& violation : m_violations)
    {
        if (violation.getType() == type)
            result.push_back(violation);
    }
    return result;
}

const std::map<std::string, std::pair<int, int>>& DrcReport::getRuleStatistics() const
{
    return m_ruleStatistics;
}

std::string DrcReport::getSummary() const
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

void DrcReport::clear()
{
    m_violations.clear();
    m_violationCount = 0;
    m_durationMs = 0;
    m_shapesChecked = 0;
    m_rulesExecuted = 0;
    m_ruleStatistics.clear();
}


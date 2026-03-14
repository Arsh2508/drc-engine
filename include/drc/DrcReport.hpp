#pragma once

#include "DrcViolation.hpp"
#include <map>
#include <string>
#include <utility>
#include <vector>


class DrcReport
{
public:
    DrcReport();

    void addViolation(const DrcViolation& violation);

    void addViolations(const std::vector<DrcViolation>& violations);

    const std::vector<DrcViolation>& getViolations() const { return m_violations; }

    std::vector<DrcViolation> getViolationsByRule(const std::string& ruleName) const;

    std::vector<DrcViolation> getViolationsByType(DrcViolation::ViolationType type) const;

    size_t getViolationCount() const { return m_violationCount; }

    bool passed() const { return m_violationCount == 0; }

    void setDurationMs(long long ms) { m_durationMs = ms; }

    long long getDurationMs() const { return m_durationMs; }

    void setShapesChecked(size_t count) { m_shapesChecked = count; }

    size_t getShapesChecked() const { return m_shapesChecked; }

    void setRulesExecuted(size_t count) { m_rulesExecuted = count; }

    size_t getRulesExecuted() const { return m_rulesExecuted; }

    const std::map<std::string, std::pair<int, int>>& getRuleStatistics() const;

    std::string getSummary() const;

    void clear();

private:
    std::vector<DrcViolation> m_violations; 
    size_t m_violationCount;               
    long long m_durationMs;                
    size_t m_shapesChecked;                 
    size_t m_rulesExecuted;                
    
    /// Maps rule names to (violation_count, max_violation_id)
    std::map<std::string, std::pair<int, int>> m_ruleStatistics;
};

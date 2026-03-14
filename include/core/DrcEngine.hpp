#pragma once

#include <drc/DrcRule.hpp>
#include <drc/DrcReport.hpp>
#include <drc/DrcContext.hpp>
#include <layout/Layout.hpp>
#include <spatial/SpatialIndex.hpp>
#include <spatial/RTreeSpatialIndex.hpp>
#include <vector>
#include <memory>
#include <chrono>

// Core DRC execution engine
class DrcEngine
{
public:
    DrcEngine() = default;

    // Register rule (executed in insertion order)
    void registerRule(DrcRulePtr rule);

    void registerRules(const std::vector<DrcRulePtr>& rules);

    const std::vector<DrcRulePtr>& getRules() const;

    size_t getRuleCount() const;

    void clearRules();

    // Execute DRC on the given layout
    DrcReport run(const Layout& layout, SpatialIndexPtr spatialIndex = nullptr);
    
private:
    std::vector<DrcRulePtr> m_rules; // Registered rules
};

using DrcEnginePtr = std::shared_ptr<DrcEngine>;

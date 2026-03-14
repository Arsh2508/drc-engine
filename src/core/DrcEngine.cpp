#include <core/DrcEngine.hpp>

#include <drc/DrcContext.hpp>
#include <drc/DrcReport.hpp>
#include <drc/DrcRule.hpp>
#include <layout/Layout.hpp>
#include <spatial/RTreeSpatialIndex.hpp>

#include <chrono>
#include <iostream>
#include <memory>
#include <vector>

void DrcEngine::registerRule(DrcRulePtr rule) {
    if(rule) {
        m_rules.push_back(rule);
    }
}

void DrcEngine::registerRules(const std::vector<DrcRulePtr>& rules) {
    m_rules.insert(m_rules.end(), rules.begin(), rules.end());
}

const std::vector<DrcRulePtr>& DrcEngine::getRules() const {
    return m_rules;
}

size_t DrcEngine::getRuleCount() const {
    return m_rules.size();
}

void DrcEngine::clearRules() {
    m_rules.clear();
}

DrcReport DrcEngine::run(const Layout& layout, SpatialIndexPtr spatialIndex) {
        DrcReport report;

        auto startTime = std::chrono::high_resolution_clock::now();

        // Use R-tree spatial index by default
        if (!spatialIndex)
        {
            spatialIndex = std::make_shared<spatial::RTreeSpatialIndex>();
        }

        // Insert shapes into R-tree for fast spatial queries
        for (const auto& [layerId, layer] : layout.getLayers())
        {
            for (const auto& shape : layer.getShapes())
            {
                spatialIndex->insert(shape);
            }
        }

        // Create execution context
        auto context = std::make_shared<DrcContext>(layout, spatialIndex);

        // Execute each rule
        for (const auto& rule : m_rules)
        {
            if (!rule)
                continue;

            // temporary debug logging: announce rule execution
            std::cout << "[DRCEngine] executing rule: " << rule->getDescription() << std::endl;

            // Execute rule and collect violations
            auto violations = rule->check(*context);

            // debug log number of violations found
            std::cout << "[DRCEngine] rule '" << rule->getDescription() << "' produced "
                      << violations.size() << " violation(s)" << std::endl;

            report.addViolations(violations);
        }

        // Record statistics
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                endTime - startTime);

        report.setShapesChecked(layout.getTotalShapeCount());
        report.setRulesExecuted(m_rules.size());
        report.setDurationMs(duration.count());

        return report;
}

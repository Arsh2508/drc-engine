#pragma once

#include <drc/DrcRule.hpp>
#include <layout/Layout.hpp>
#include <memory>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Loads DRC rules from a dedicated JSON file.
class RulesJsonLoader
{
public:
    std::vector<DrcRulePtr> load(const std::string& filename,
                                 const std::shared_ptr<Layout>& layout);

    std::string getDescription() const;

private:
    void parseLayerRuleConfig(const json& layersJson, Layout& layout);
    void parseIntersection(const json& intersectionJson,
                           std::vector<DrcRulePtr>& rules) const;
    void parseMinSpacing(const json& minSpacingJson,
                         const std::shared_ptr<Layout>& layout,
                         std::vector<DrcRulePtr>& rules) const;
    void parseEnclosure(const json& enclosureJson,
                        std::vector<DrcRulePtr>& rules) const;
    void parseContainment(const json& containmentJson,
                          std::vector<DrcRulePtr>& rules) const;
};

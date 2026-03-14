#pragma once

#include <layout/Layout.hpp>
#include <spatial/SpatialIndex.hpp>
#include <memory>

/// @brief Context information passed to DRC rules during execution.
///
/// DrcContext encapsulates all information that a rule needs to:
/// 1. Access the layout being checked
/// 2. Query shapes spatially
/// 3. Report violations
///
/// Key Design:
/// - Rules receive const reference to DrcContext
/// - Rules access layout and shapes through this context
/// - Rules use spatial index for efficient queries
/// - Separates rule logic from engine management
///
/// Benefits:
/// - Rules are decoupled from engine implementation
/// - Easy to extend context with additional services (logging, cache, etc.)
/// - Thread-safe (immutable during rule execution)
///
class DrcContext
{
public:
    /// @brief Construct DRC context with layout and spatial index.
    /// @param layout Reference to the layout being checked (non-owning)
    /// @param spatialIndex Shared pointer to spatial index (owned)
    DrcContext(const Layout& layout, SpatialIndexPtr spatialIndex)
        : m_layout(layout), m_spatialIndex(spatialIndex)
    {
    }

    const Layout& getLayout() const { return m_layout; }

    const SpatialIndex& getSpatialIndex() const { return *m_spatialIndex; }

    /// @brief Get mutable spatial index reference (for internal engine use).
    SpatialIndex& getSpatialIndexMutable() { return *m_spatialIndex; }

    /// @brief Get the spatial index shared pointer (rarely needed).
    SpatialIndexPtr getSpatialIndexPtr() const { return m_spatialIndex; }

private:
    const Layout& m_layout;    ///< Reference to layout (non-owning)
    SpatialIndexPtr m_spatialIndex; ///< Spatial index for queries (owned)
};

using DrcContextPtr = std::shared_ptr<DrcContext>;

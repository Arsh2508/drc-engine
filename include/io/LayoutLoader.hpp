#pragma once

#include <layout/Layout.hpp>
#include <memory>
#include <string>
#include <stdexcept>

/// @brief Abstract base class for layout file loading.
/// 
/// Defines the interface for loading layout data from various file formats.
/// Implementations handle format-specific parsing logic (JSON, GDS, LEF/DEF, etc.).
class LayoutLoader
{
public:
    virtual ~LayoutLoader() = default;

    /// @brief Load a layout from a file.
    /// 
    /// Implementations must:
    /// - Validate file existence and readability
    /// - Parse format-specific content
    /// - Create Layout object with layers and shapes
    /// - Throw exception on errors (malformed data, etc.)
    ///
    /// @param filename Path to the layout file
    /// @return Shared pointer to the loaded Layout
    /// @throws std::runtime_error if loading fails
    virtual std::shared_ptr<Layout> load(const std::string& filename) = 0;

    /// @brief Get a human-readable description of the loader.
    /// @return Description string
    virtual std::string getDescription() const = 0;
};

/// Convenience alias for shared pointer to LayoutLoader
using LayoutLoaderPtr = std::shared_ptr<LayoutLoader>;


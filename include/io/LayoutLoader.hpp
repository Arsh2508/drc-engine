#pragma once

#include <layout/Layout.hpp>
#include <memory>
#include <string>
#include <stdexcept>

/// @brief Abstract base class for layout file loading.

class LayoutLoader
{
public:
    virtual ~LayoutLoader() = default;

    virtual std::shared_ptr<Layout> load(const std::string& filename) = 0;
    virtual std::string getDescription() const = 0;
};

using LayoutLoaderPtr = std::shared_ptr<LayoutLoader>;


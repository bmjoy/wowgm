#pragma once

#ifdef _DEBUG
# define ENABLE_VALIDATION_LAYERS
#endif

#include <vector>

namespace wowgm::graphics::details
{
    extern const std::vector<const char*> requiredDeviceExtensions;
    extern const std::vector<const char*> requiredValidationLayers;
}

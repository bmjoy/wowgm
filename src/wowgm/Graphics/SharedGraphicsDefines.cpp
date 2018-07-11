#include "SharedGraphicsDefines.hpp"

namespace wowgm::graphics::details
{
    const std::vector<const char*> requiredValidationLayers = {
        "VK_LAYER_LUNARG_standard_validation"
    };


    const std::vector<const char*> requiredDeviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
}

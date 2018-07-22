#include "SharedGraphicsDefines.hpp"

namespace wowgm::graphics::details
{
    const std::array<const char*, 1> ValidationLayers = {
        "VK_LAYER_LUNARG_standard_validation"
    };


    const std::array<const char*, 1> DeviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
}

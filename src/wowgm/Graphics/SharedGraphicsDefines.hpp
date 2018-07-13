#pragma once

#ifdef _DEBUG
# define ENABLE_VALIDATION_LAYERS
#endif

#include <vector>
#include <vulkan/vulkan.h>
#include <type_traits>

namespace wowgm::graphics::details
{
    extern const std::vector<const char*> requiredDeviceExtensions;
    extern const std::vector<const char*> requiredValidationLayers;

    template <typename T>
    struct optional_bit : std::false_type {
        static const constexpr VkDynamicState bit_value = VK_DYNAMIC_STATE_MAX_ENUM;
    };

    template <>
    struct optional_bit<VkPipelineColorBlendStateCreateInfo> : std::true_type {
        static const constexpr VkDynamicState bit_value = VK_DYNAMIC_STATE_BLEND_CONSTANTS;
    };

    template <>
    struct optional_bit<VkPipelineViewportStateCreateInfo> : std::true_type {
        static const constexpr VkDynamicState bit_value = VkDynamicState(VK_DYNAMIC_STATE_VIEWPORT | VK_DYNAMIC_STATE_SCISSOR);
    };
}

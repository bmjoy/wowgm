#pragma once

#include <VEZ.h>

#include "Core/Swapchain.h"
#include "Core/CommandBuffer.h"
#include "Core/Pipeline.h"
#include "Core/Framebuffer.h"

namespace vez
{
    namespace traits
    {
        template <typename T>
        struct vulkan_type {
            constexpr static const VkObjectType type = VK_OBJECT_TYPE_UNKNOWN;
            constexpr static const VkDebugReportObjectTypeEXT type_ext = VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT;
        };

#define VULKAN_TYPE_TRAIT(TYPE, ENUM, ENUM_EXT)                                    \
    template <> struct vulkan_type<TYPE> {                                         \
        constexpr static const VkObjectType type = ENUM;                           \
        constexpr static const VkDebugReportObjectTypeEXT type_ext = ENUM_EXT;     \
    }

        VULKAN_TYPE_TRAIT(VkInstance,                    VK_OBJECT_TYPE_INSTANCE,                       VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT);
        VULKAN_TYPE_TRAIT(VkPhysicalDevice,              VK_OBJECT_TYPE_PHYSICAL_DEVICE,                VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT);
        VULKAN_TYPE_TRAIT(VkDevice,                      VK_OBJECT_TYPE_DEVICE,                         VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT);
        VULKAN_TYPE_TRAIT(VkQueue,                       VK_OBJECT_TYPE_QUEUE,                          VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT);
        VULKAN_TYPE_TRAIT(VkSemaphore,                   VK_OBJECT_TYPE_SEMAPHORE,                      VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT);
        VULKAN_TYPE_TRAIT(VkCommandBuffer,               VK_OBJECT_TYPE_COMMAND_BUFFER,                 VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT);
        VULKAN_TYPE_TRAIT(VkFence,                       VK_OBJECT_TYPE_FENCE,                          VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT);
        VULKAN_TYPE_TRAIT(VkDeviceMemory,                VK_OBJECT_TYPE_DEVICE_MEMORY,                  VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT);
        VULKAN_TYPE_TRAIT(VkBuffer,                      VK_OBJECT_TYPE_BUFFER,                         VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT);
        VULKAN_TYPE_TRAIT(VkImage,                       VK_OBJECT_TYPE_IMAGE,                          VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT);
        VULKAN_TYPE_TRAIT(VkEvent,                       VK_OBJECT_TYPE_EVENT,                          VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT);
        VULKAN_TYPE_TRAIT(VkQueryPool,                   VK_OBJECT_TYPE_QUERY_POOL,                     VK_DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT);
        VULKAN_TYPE_TRAIT(VkBufferView,                  VK_OBJECT_TYPE_BUFFER_VIEW,                    VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT);
        VULKAN_TYPE_TRAIT(VkImageView,                   VK_OBJECT_TYPE_IMAGE_VIEW,                     VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT);
        VULKAN_TYPE_TRAIT(VkShaderModule,                VK_OBJECT_TYPE_SHADER_MODULE,                  VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT);
        VULKAN_TYPE_TRAIT(VkPipelineCache,               VK_OBJECT_TYPE_PIPELINE_CACHE,                 VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_CACHE_EXT);
        VULKAN_TYPE_TRAIT(VkPipelineLayout,              VK_OBJECT_TYPE_PIPELINE_LAYOUT,                VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT);
        VULKAN_TYPE_TRAIT(VkRenderPass,                  VK_OBJECT_TYPE_RENDER_PASS,                    VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT);
        VULKAN_TYPE_TRAIT(VkPipeline,                    VK_OBJECT_TYPE_PIPELINE,                       VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT);
        VULKAN_TYPE_TRAIT(VkDescriptorSetLayout,         VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT,          VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT);
        VULKAN_TYPE_TRAIT(VkSampler,                     VK_OBJECT_TYPE_SAMPLER,                        VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT);
        VULKAN_TYPE_TRAIT(VkDescriptorPool,              VK_OBJECT_TYPE_DESCRIPTOR_POOL,                VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT);
        VULKAN_TYPE_TRAIT(VkDescriptorSet,               VK_OBJECT_TYPE_DESCRIPTOR_SET,                 VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT);
        VULKAN_TYPE_TRAIT(VkFramebuffer,                 VK_OBJECT_TYPE_FRAMEBUFFER,                    VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT);
        VULKAN_TYPE_TRAIT(VkCommandPool,                 VK_OBJECT_TYPE_COMMAND_POOL,                   VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT);
        VULKAN_TYPE_TRAIT(VkSurfaceKHR,                  VK_OBJECT_TYPE_SURFACE_KHR,                    VK_DEBUG_REPORT_OBJECT_TYPE_SURFACE_KHR_EXT);
        VULKAN_TYPE_TRAIT(VkSwapchainKHR,                VK_OBJECT_TYPE_SWAPCHAIN_KHR,                  VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT);
        VULKAN_TYPE_TRAIT(VkDebugReportCallbackEXT,      VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT,      VK_DEBUG_REPORT_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT_EXT);
        VULKAN_TYPE_TRAIT(VkDisplayKHR,                  VK_OBJECT_TYPE_DISPLAY_KHR,                    VK_DEBUG_REPORT_OBJECT_TYPE_DISPLAY_KHR_EXT);
        VULKAN_TYPE_TRAIT(VkDisplayModeKHR,              VK_OBJECT_TYPE_DISPLAY_MODE_KHR,               VK_DEBUG_REPORT_OBJECT_TYPE_DISPLAY_MODE_KHR_EXT);
        VULKAN_TYPE_TRAIT(VkObjectTableNVX,              VK_OBJECT_TYPE_OBJECT_TABLE_NVX,               VK_DEBUG_REPORT_OBJECT_TYPE_OBJECT_TABLE_NVX_EXT);
        VULKAN_TYPE_TRAIT(VkIndirectCommandsLayoutNVX,   VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NVX,   VK_DEBUG_REPORT_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NVX_EXT);
        VULKAN_TYPE_TRAIT(VkDescriptorUpdateTemplateKHR, VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_KHR, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_KHR_EXT);
        VULKAN_TYPE_TRAIT(VkSamplerYcbcrConversionKHR,   VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION_KHR,   VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION_KHR_EXT);

#undef VULKAN_TYPE_TRAIT

        template <typename T>
        struct vez_type
        {
            using vulkan_type = std::nullptr_t; // Vk<...>
            using underlying_type = std::nullptr_t; // vez::<...>
        };

#define VEZ_TO_VULKAN(VEZ, VULKAN, VEZTYPE)   \
    template <> struct vez_type<VEZ> {        \
        using vulkan_type = VULKAN;           \
        using underlying_type = VEZTYPE;      \
    }

        VEZ_TO_VULKAN(VezSwapchain, VkSwapchainKHR, vez::Swapchain);
        VEZ_TO_VULKAN(VezCommandBuffer, VkCommandBuffer, vez::CommandBuffer);
        VEZ_TO_VULKAN(VezPipeline, VkPipeline, vez::Pipeline);
        VEZ_TO_VULKAN(VezFramebuffer, VkFramebuffer, vez::Framebuffer);

#undef VEZ_TO_VULKAN
    }

    namespace debug
    {
        void initializeFunctionPointers(VkInstance instance);

        bool initializeFunctionCallback(VkInstance instance, PFN_vkDebugUtilsMessengerCallbackEXT messengerHandler, PFN_vkDebugReportCallbackEXT fallbackMessageHandler = nullptr);

        void destroyDebugCallbacks(VkInstance instance);

        bool beginLabel(VkQueue queue, const char* label);
        bool insertLabel(VkQueue queue, const char* label);
        bool endLabel(VkQueue queue);

        bool beginLabel(VkCommandBuffer commandBuffer, const char* label);
        bool insertLabel(VkCommandBuffer commandBuffer, const char* label);
        bool endLabel(VkCommandBuffer commandBuffer);

        bool beginLabel(VezCommandBuffer commandBuffer, const char* label);
        bool insertLabel(VezCommandBuffer commandBuffer, const char* label);
        bool endLabel(VezCommandBuffer commandBuffer);

        template <typename T>
        bool setObjectName(VkDevice device, T object, const char* objectName);
    }
}


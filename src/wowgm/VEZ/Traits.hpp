#pragma once

#include <vulkan/vulkan.hpp>
#include <type_traits>

namespace vez
{
    template <typename T>
    struct is_vulkan_type : std::false_type {} ;

    template <> struct is_vulkan_type<VkInstance> : std::true_type { };
    template <> struct is_vulkan_type<VkPhysicalDevice> : std::true_type { };
    template <> struct is_vulkan_type<VkDevice> : std::true_type { };
    template <> struct is_vulkan_type<VkQueue> : std::true_type { };
    template <> struct is_vulkan_type<VkSemaphore> : std::true_type { };
    template <> struct is_vulkan_type<VkCommandBuffer> : std::true_type { };
    template <> struct is_vulkan_type<VkFence> : std::true_type { };
    template <> struct is_vulkan_type<VkDeviceMemory> : std::true_type { };
    template <> struct is_vulkan_type<VkBuffer> : std::true_type { };
    template <> struct is_vulkan_type<VkImage> : std::true_type { };
    template <> struct is_vulkan_type<VkEvent> : std::true_type { };
    template <> struct is_vulkan_type<VkQueryPool> : std::true_type { };
    template <> struct is_vulkan_type<VkBufferView> : std::true_type { };
    template <> struct is_vulkan_type<VkImageView> : std::true_type { };
    template <> struct is_vulkan_type<VkShaderModule> : std::true_type { };
    template <> struct is_vulkan_type<VkPipelineCache> : std::true_type { };
    template <> struct is_vulkan_type<VkPipelineLayout> : std::true_type { };
    template <> struct is_vulkan_type<VkRenderPass> : std::true_type { };
    template <> struct is_vulkan_type<VkPipeline> : std::true_type { };
    template <> struct is_vulkan_type<VkDescriptorSetLayout> : std::true_type { };
    template <> struct is_vulkan_type<VkSampler> : std::true_type { };
    template <> struct is_vulkan_type<VkDescriptorPool> : std::true_type { };
    template <> struct is_vulkan_type<VkDescriptorSet> : std::true_type { };
    template <> struct is_vulkan_type<VkFramebuffer> : std::true_type { };
    template <> struct is_vulkan_type<VkCommandPool> : std::true_type { };
    template <> struct is_vulkan_type<VkSurfaceKHR> : std::true_type { };
    template <> struct is_vulkan_type<VkSwapchainKHR> : std::true_type { };
    template <> struct is_vulkan_type<VkDebugReportCallbackEXT> : std::true_type { };
    template <> struct is_vulkan_type<VkDisplayKHR> : std::true_type { };
    template <> struct is_vulkan_type<VkDisplayModeKHR> : std::true_type { };
    template <> struct is_vulkan_type<VkObjectTableNVX> : std::true_type { };
    template <> struct is_vulkan_type<VkIndirectCommandsLayoutNVX> : std::true_type { };
    template <> struct is_vulkan_type<VkDescriptorUpdateTemplateKHR> : std::true_type { };
    template <> struct is_vulkan_type<VkSamplerYcbcrConversionKHR> : std::true_type { };

    template <typename T>
    struct is_vez_type
    {
    private:
        typedef char yes[1];
        typedef char no[2];

        template <typename C> static yes& test(decltype(&C::GetHandle));
        template <typename C> static no& test(...);

    public:
        enum { value = sizeof(test<T>(0)) == sizeof(yes) };
    };

    template <typename T>
    struct object_type;

    template <typename T>
    struct object_type_ext;

#define VULKAN_OBJECT_TYPE(OBJECT, TYPE, TYPE_EXT)                          \
    template <> struct object_type<OBJECT> {                                \
        constexpr static const VkObjectType value = TYPE;                   \
    };                                                                      \
                                                                            \
    template <> struct object_type_ext<OBJECT> {                            \
        constexpr static const VkDebugReportObjectTypeEXT value = TYPE_EXT; \
    }

    VULKAN_OBJECT_TYPE(VkInstance,                    VK_OBJECT_TYPE_INSTANCE,                       VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT);
    VULKAN_OBJECT_TYPE(VkPhysicalDevice,              VK_OBJECT_TYPE_PHYSICAL_DEVICE,                VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT);
    VULKAN_OBJECT_TYPE(VkDevice,                      VK_OBJECT_TYPE_DEVICE,                         VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT);
    VULKAN_OBJECT_TYPE(VkQueue,                       VK_OBJECT_TYPE_QUEUE,                          VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT);
    VULKAN_OBJECT_TYPE(VkSemaphore,                   VK_OBJECT_TYPE_SEMAPHORE,                      VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT);
    VULKAN_OBJECT_TYPE(VkCommandBuffer,               VK_OBJECT_TYPE_COMMAND_BUFFER,                 VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT);
    VULKAN_OBJECT_TYPE(VkFence,                       VK_OBJECT_TYPE_FENCE,                          VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT);
    VULKAN_OBJECT_TYPE(VkDeviceMemory,                VK_OBJECT_TYPE_DEVICE_MEMORY,                  VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT);
    VULKAN_OBJECT_TYPE(VkBuffer,                      VK_OBJECT_TYPE_BUFFER,                         VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT);
    VULKAN_OBJECT_TYPE(VkImage,                       VK_OBJECT_TYPE_IMAGE,                          VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT);
    VULKAN_OBJECT_TYPE(VkEvent,                       VK_OBJECT_TYPE_EVENT,                          VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT);
    VULKAN_OBJECT_TYPE(VkQueryPool,                   VK_OBJECT_TYPE_QUERY_POOL,                     VK_DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT);
    VULKAN_OBJECT_TYPE(VkBufferView,                  VK_OBJECT_TYPE_BUFFER_VIEW,                    VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT);
    VULKAN_OBJECT_TYPE(VkImageView,                   VK_OBJECT_TYPE_IMAGE_VIEW,                     VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT);
    VULKAN_OBJECT_TYPE(VkShaderModule,                VK_OBJECT_TYPE_SHADER_MODULE,                  VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT);
    VULKAN_OBJECT_TYPE(VkPipelineCache,               VK_OBJECT_TYPE_PIPELINE_CACHE,                 VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_CACHE_EXT);
    VULKAN_OBJECT_TYPE(VkPipelineLayout,              VK_OBJECT_TYPE_PIPELINE_LAYOUT,                VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT);
    VULKAN_OBJECT_TYPE(VkRenderPass,                  VK_OBJECT_TYPE_RENDER_PASS,                    VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT);
    VULKAN_OBJECT_TYPE(VkPipeline,                    VK_OBJECT_TYPE_PIPELINE,                       VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT);
    VULKAN_OBJECT_TYPE(VkDescriptorSetLayout,         VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT,          VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT);
    VULKAN_OBJECT_TYPE(VkSampler,                     VK_OBJECT_TYPE_SAMPLER,                        VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT);
    VULKAN_OBJECT_TYPE(VkDescriptorPool,              VK_OBJECT_TYPE_DESCRIPTOR_POOL,                VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT);
    VULKAN_OBJECT_TYPE(VkDescriptorSet,               VK_OBJECT_TYPE_DESCRIPTOR_SET,                 VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT);
    VULKAN_OBJECT_TYPE(VkFramebuffer,                 VK_OBJECT_TYPE_FRAMEBUFFER,                    VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT);
    VULKAN_OBJECT_TYPE(VkCommandPool,                 VK_OBJECT_TYPE_COMMAND_POOL,                   VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT);
    VULKAN_OBJECT_TYPE(VkSurfaceKHR,                  VK_OBJECT_TYPE_SURFACE_KHR,                    VK_DEBUG_REPORT_OBJECT_TYPE_SURFACE_KHR_EXT);
    VULKAN_OBJECT_TYPE(VkSwapchainKHR,                VK_OBJECT_TYPE_SWAPCHAIN_KHR,                  VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT);
    VULKAN_OBJECT_TYPE(VkDebugReportCallbackEXT,      VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT,      VK_DEBUG_REPORT_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT_EXT);
    VULKAN_OBJECT_TYPE(VkDisplayKHR,                  VK_OBJECT_TYPE_DISPLAY_KHR,                    VK_DEBUG_REPORT_OBJECT_TYPE_DISPLAY_KHR_EXT);
    VULKAN_OBJECT_TYPE(VkDisplayModeKHR,              VK_OBJECT_TYPE_DISPLAY_MODE_KHR,               VK_DEBUG_REPORT_OBJECT_TYPE_DISPLAY_MODE_KHR_EXT);
    VULKAN_OBJECT_TYPE(VkObjectTableNVX,              VK_OBJECT_TYPE_OBJECT_TABLE_NVX,               VK_DEBUG_REPORT_OBJECT_TYPE_OBJECT_TABLE_NVX_EXT);
    VULKAN_OBJECT_TYPE(VkIndirectCommandsLayoutNVX,   VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NVX,   VK_DEBUG_REPORT_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NVX_EXT);
    VULKAN_OBJECT_TYPE(VkDescriptorUpdateTemplateKHR, VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_KHR, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_KHR_EXT);
    VULKAN_OBJECT_TYPE(VkSamplerYcbcrConversionKHR,   VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION_KHR,   VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION_KHR_EXT);

#undef VULKAN_OBJECT_TYPE

    template <typename T>
    struct vez_traits
    {
        static_assert(is_vez_type<T>::value);

        using handle_type = decltype(std::declval<T>().GetHandle());
        constexpr static const VkObjectType object_type = object_type<handle_type>::value;
        constexpr static const VkDebugReportObjectTypeEXT object_type_ext = object_type_ext<handle_type>::value;

        constexpr static bool is_vulkan_type_handle = is_vulkan_type<handle_type>::value;
    };

}

namespace std
{
    template <typename T>
    struct is_data_container : public is_same<decltype(declval<T>().data()), typename add_const<typename T::element_type>::type*> { };
}

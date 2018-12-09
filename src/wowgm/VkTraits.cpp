#include "VkTraits.hpp"


#include <mutex>
#include <thread>


namespace vez
{
    namespace debug
    {
        namespace
        {
            static PFN_vkQueueEndDebugUtilsLabelEXT vkQueueEndDebugUtilsLabelEXT;
            static PFN_vkQueueBeginDebugUtilsLabelEXT vkQueueBeginDebugUtilsLabelEXT;
            static PFN_vkQueueInsertDebugUtilsLabelEXT vkQueueInsertDebugUtilsLabelEXT;

            static PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabelEXT;
            static PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabelEXT;
            static PFN_vkCmdInsertDebugUtilsLabelEXT vkCmdInsertDebugUtilsLabelEXT;

            static PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT;

            static PFN_vkDebugMarkerSetObjectNameEXT vkDebugMarkerSetObjectNameEXT;

            static VkDebugUtilsMessengerEXT callbackSite;
            static VkDebugReportCallbackEXT fallbackCallbackSite;
        }

        void initializeFunctionPointers(VkInstance instance)
        {
#if _DEBUG
            static std::once_flag once_flag;
            std::call_once(once_flag, [](VkInstance _instance) -> void {

                vkQueueEndDebugUtilsLabelEXT = PFN_vkQueueEndDebugUtilsLabelEXT(vkGetInstanceProcAddr(_instance, "vkQueueEndDebugUtilsLabelEXT"));
                vkCmdEndDebugUtilsLabelEXT = PFN_vkCmdEndDebugUtilsLabelEXT(vkGetInstanceProcAddr(_instance, "vkCmdEndDebugUtilsLabelEXT"));

                vkQueueBeginDebugUtilsLabelEXT = PFN_vkQueueBeginDebugUtilsLabelEXT(vkGetInstanceProcAddr(_instance, "vkQueueBeginDebugUtilsLabelEXT"));
                vkCmdBeginDebugUtilsLabelEXT = PFN_vkCmdBeginDebugUtilsLabelEXT(vkGetInstanceProcAddr(_instance, "vkCmdBeginDebugUtilsLabelEXT"));

                vkQueueInsertDebugUtilsLabelEXT = PFN_vkQueueInsertDebugUtilsLabelEXT(vkGetInstanceProcAddr(_instance, "vkQueueInsertDebugUtilsLabelEXT"));
                vkCmdInsertDebugUtilsLabelEXT = PFN_vkCmdInsertDebugUtilsLabelEXT(vkGetInstanceProcAddr(_instance, "vkCmdInsertDebugUtilsLabelEXT"));

                vkSetDebugUtilsObjectNameEXT = PFN_vkSetDebugUtilsObjectNameEXT(vkGetInstanceProcAddr(_instance, "vkSetDebugUtilsObjectNameEXT"));
                if (vkSetDebugUtilsObjectNameEXT == nullptr)
                    vkDebugMarkerSetObjectNameEXT = PFN_vkDebugMarkerSetObjectNameEXT(vkGetInstanceProcAddr(_instance, "vkDebugMarkerSetObjectNameEXT"));
            }, instance);
#endif
        }

        bool initializeFunctionCallback(VkInstance instance, PFN_vkDebugUtilsMessengerCallbackEXT messengerHandler, PFN_vkDebugReportCallbackEXT fallbackMessageHandler)
        {
#if _DEBUG
            auto debugUtilsCallbackInstaller = PFN_vkCreateDebugUtilsMessengerEXT(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
            if (debugUtilsCallbackInstaller != nullptr && messengerHandler != nullptr)
            {
                VkDebugUtilsMessengerCreateInfoEXT createInfo{};
                createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
                createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
                createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
                createInfo.pfnUserCallback = messengerHandler;
                createInfo.pUserData = nullptr; // Optional

                return debugUtilsCallbackInstaller(instance, &createInfo, nullptr, &callbackSite) == VK_SUCCESS;
            }
            else if (fallbackMessageHandler != nullptr)
            {
                VkDebugReportCallbackCreateInfoEXT createInfo{};
                createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
                createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
                createInfo.pfnCallback = fallbackMessageHandler;

                auto debugReportCallbackInstaller = PFN_vkCreateDebugReportCallbackEXT(vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT"));
                if (debugReportCallbackInstaller != nullptr)
                    return debugReportCallbackInstaller(instance, &createInfo, nullptr, &fallbackCallbackSite) == VK_SUCCESS;

                return false;
            }

            return false;
#endif
            return true;
        }

        void destroyDebugCallbacks(VkInstance instance)
        {
#if _DEBUG
            if (callbackSite != nullptr)
            {
                auto debugDestroyUtilsMessenger = PFN_vkDestroyDebugUtilsMessengerEXT(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
                if (debugDestroyUtilsMessenger != nullptr)
                    debugDestroyUtilsMessenger(instance, callbackSite, nullptr);
            }
            else if (fallbackCallbackSite != nullptr)
            {
                auto destroyDebugReportCallback = PFN_vkDestroyDebugReportCallbackEXT(vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT"));
                if (destroyDebugReportCallback != nullptr)
                    destroyDebugReportCallback(instance, fallbackCallbackSite, nullptr);
            }
#endif
        }

        bool beginLabel(VkQueue queue, const char* label)
        {
#if _DEBUG
            if (vkQueueBeginDebugUtilsLabelEXT != nullptr)
            {
                VkDebugUtilsLabelEXT labelInfo{};
                labelInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
                labelInfo.color[0] = 1.0f;
                labelInfo.color[1] = 1.0f;
                labelInfo.color[2] = 1.0f;
                labelInfo.color[3] = 1.0f;
                labelInfo.pLabelName = label;

                vkQueueBeginDebugUtilsLabelEXT(queue, &labelInfo);
                return true;
            }
            return false;
#endif
            return true;
        }

        bool insertLabel(VkQueue queue, const char* label)
        {
#if _DEBUG
            if (vkQueueInsertDebugUtilsLabelEXT != nullptr)
            {
                VkDebugUtilsLabelEXT labelInfo{};
                labelInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
                labelInfo.color[0] = 1.0f;
                labelInfo.color[1] = 1.0f;
                labelInfo.color[2] = 1.0f;
                labelInfo.color[3] = 1.0f;
                labelInfo.pLabelName = label;

                vkQueueInsertDebugUtilsLabelEXT(queue, &labelInfo);
                return true;
            }
            return false;
#endif
            return true;
        }

        bool endLabel(VkQueue queue)
        {
#if _DEBUG
            if (vkQueueEndDebugUtilsLabelEXT != nullptr)
            {
                vkQueueEndDebugUtilsLabelEXT(queue);
                return true;
            }
            return false;
#endif
            return true;
        }

        bool beginLabel(VkCommandBuffer commandBuffer, const char* label)
        {
#if _DEBUG
            if (vkCmdBeginDebugUtilsLabelEXT != nullptr)
            {
                VkDebugUtilsLabelEXT labelInfo{};
                labelInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
                labelInfo.color[0] = 1.0f;
                labelInfo.color[1] = 1.0f;
                labelInfo.color[2] = 1.0f;
                labelInfo.color[3] = 1.0f;
                labelInfo.pLabelName = label;

                vkCmdBeginDebugUtilsLabelEXT(commandBuffer, &labelInfo);
                return true;
            }
            return false;
#endif
            return true;
        }

        bool insertLabel(VkCommandBuffer commandBuffer, const char* label)
        {
#if _DEBUG
            if (vkCmdInsertDebugUtilsLabelEXT != nullptr)
            {
                VkDebugUtilsLabelEXT labelInfo{};
                labelInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
                labelInfo.color[0] = 1.0f;
                labelInfo.color[1] = 1.0f;
                labelInfo.color[2] = 1.0f;
                labelInfo.color[3] = 1.0f;
                labelInfo.pLabelName = label;

                vkCmdInsertDebugUtilsLabelEXT(commandBuffer, &labelInfo);
                return true;
            }
            return false;
#endif
            return true;
        }

        bool endLabel(VkCommandBuffer commandBuffer)
        {
#if _DEBUG
            if (vkCmdEndDebugUtilsLabelEXT != nullptr)
            {
                vkCmdEndDebugUtilsLabelEXT(commandBuffer);
                return true;
            }
            return false;
#endif
            return true;
        }

        bool beginLabel(VezCommandBuffer commandBuffer, const char* label)
        {
#if _DEBUG
            return beginLabel(reinterpret_cast<vez::CommandBuffer*>(commandBuffer)->GetHandle(), label);
#endif
            return true;
        }

        bool insertLabel(VezCommandBuffer commandBuffer, const char* label)
        {
#if _DEBUG
            return insertLabel(reinterpret_cast<vez::CommandBuffer*>(commandBuffer)->GetHandle(), label);
#endif
            return true;
        }

        bool endLabel(VezCommandBuffer commandBuffer)
        {
#if _DEBUG
            return endLabel(reinterpret_cast<vez::CommandBuffer*>(commandBuffer)->GetHandle());
#endif
            return true;
        }

        template <typename T>
        bool setObjectName(VkDevice device, T object, const char* objectName)
        {
#if _DEBUG
            if (vkSetDebugUtilsObjectNameEXT != nullptr)
            {
                VkDebugUtilsObjectNameInfoEXT nameInfo{};
                nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
                nameInfo.pObjectName = objectName;

                if constexpr (!std::is_same<vez::traits::vez_type<T>::vulkan_type, std::nullptr_t>::value)
                {
                    // T is a Vez...
                    using vulkan_type = typename vez::traits::vez_type<T>::vulkan_type;
                    static_assert(vez::traits::vulkan_type<vulkan_type>::type != VK_OBJECT_TYPE_UNKNOWN,
                        "Unable to set a name for the provided object.");

                    nameInfo.objectType = vez::traits::vulkan_type<vulkan_type>::type;

                    auto underlyingInstance = reinterpret_cast<typename vez::traits::vez_type<T>::underlying_type*>(object);
                    nameInfo.objectHandle = reinterpret_cast<std::uintptr_t>(underlyingInstance->GetHandle());
                }
                else
                {
                    static_assert(vez::traits::vulkan_type<T>::type != VK_OBJECT_TYPE_UNKNOWN,
                        "Unable to set a name for the provided object.");

                    nameInfo.objectType = vez::traits::vulkan_type<T>::type;
                    nameInfo.objectHandle = reinterpret_cast<std::uintptr_t>(object);
                }

                return vkSetDebugUtilsObjectNameEXT(device, &nameInfo) == VK_SUCCESS;
            }
            else if (vkDebugMarkerSetObjectNameEXT != nullptr)
            {
                VkDebugMarkerObjectNameInfoEXT nameInfo{};
                nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
                nameInfo.pObjectName = objectName;

                if constexpr (!std::is_same<vez::traits::vez_type<T>::vulkan_type, std::nullptr_t>::value)
                {
                    // T is a Vez...
                    using vulkan_type = typename vez::traits::vez_type<T>::vulkan_type;
                    static_assert(vez::traits::vulkan_type<vulkan_type>::type_ext != VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT,
                        "Unable to set a name for the provided object.");

                    nameInfo.objectType = vez::traits::vulkan_type<vulkan_type>::type_ext;

                    auto underlyingInstance = reinterpret_cast<typename vez::traits::vez_type<T>::underlying_type*>(object);
                    nameInfo.object = reinterpret_cast<std::uintptr_t>(underlyingInstance->GetHandle());
                }
                else
                {
                    static_assert(vez::traits::vulkan_type<T>::type_ext != VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT,
                        "Unable to set a name for the provided object.");

                    nameInfo.objectType = vez::traits::vulkan_type<T>::type_ext;
                    nameInfo.object = reinterpret_cast<std::uintptr_t>(object);
                }

                return vkDebugMarkerSetObjectNameEXT(device, &nameInfo) == VK_SUCCESS;
            }

            return false;
#endif
            return true;
        }

        template bool setObjectName<VkInstance>(VkDevice device, VkInstance object, const char* objectName);
        template bool setObjectName<VkPhysicalDevice>(VkDevice device, VkPhysicalDevice object, const char* objectName);
        template bool setObjectName<VkDevice>(VkDevice device, VkDevice object, const char* objectName);
        template bool setObjectName<VkQueue>(VkDevice device, VkQueue object, const char* objectName);
        template bool setObjectName<VkSemaphore>(VkDevice device, VkSemaphore object, const char* objectName);
        template bool setObjectName<VkCommandBuffer>(VkDevice device, VkCommandBuffer object, const char* objectName);
        template bool setObjectName<VkFence>(VkDevice device, VkFence object, const char* objectName);
        template bool setObjectName<VkDeviceMemory>(VkDevice device, VkDeviceMemory object, const char* objectName);
        template bool setObjectName<VkBuffer>(VkDevice device, VkBuffer object, const char* objectName);
        template bool setObjectName<VkImage>(VkDevice device, VkImage object, const char* objectName);
        template bool setObjectName<VkEvent>(VkDevice device, VkEvent object, const char* objectName);
        template bool setObjectName<VkQueryPool>(VkDevice device, VkQueryPool object, const char* objectName);
        template bool setObjectName<VkBufferView>(VkDevice device, VkBufferView object, const char* objectName);
        template bool setObjectName<VkImageView>(VkDevice device, VkImageView object, const char* objectName);
        template bool setObjectName<VkShaderModule>(VkDevice device, VkShaderModule object, const char* objectName);
        template bool setObjectName<VkPipelineCache>(VkDevice device, VkPipelineCache object, const char* objectName);
        template bool setObjectName<VkPipelineLayout>(VkDevice device, VkPipelineLayout object, const char* objectName);
        template bool setObjectName<VkRenderPass>(VkDevice device, VkRenderPass object, const char* objectName);
        template bool setObjectName<VkPipeline>(VkDevice device, VkPipeline object, const char* objectName);
        template bool setObjectName<VkDescriptorSetLayout>(VkDevice device, VkDescriptorSetLayout object, const char* objectName);
        template bool setObjectName<VkSampler>(VkDevice device, VkSampler object, const char* objectName);
        template bool setObjectName<VkDescriptorPool>(VkDevice device, VkDescriptorPool object, const char* objectName);
        template bool setObjectName<VkDescriptorSet>(VkDevice device, VkDescriptorSet object, const char* objectName);
        template bool setObjectName<VkFramebuffer>(VkDevice device, VkFramebuffer object, const char* objectName);
        template bool setObjectName<VkCommandPool>(VkDevice device, VkCommandPool object, const char* objectName);
        template bool setObjectName<VkSurfaceKHR>(VkDevice device, VkSurfaceKHR object, const char* objectName);
        template bool setObjectName<VkSwapchainKHR>(VkDevice device, VkSwapchainKHR object, const char* objectName);
        template bool setObjectName<VkDebugReportCallbackEXT>(VkDevice device, VkDebugReportCallbackEXT object, const char* objectName);
        template bool setObjectName<VkDisplayKHR>(VkDevice device, VkDisplayKHR object, const char* objectName);
        template bool setObjectName<VkDisplayModeKHR>(VkDevice device, VkDisplayModeKHR object, const char* objectName);
        template bool setObjectName<VkObjectTableNVX>(VkDevice device, VkObjectTableNVX object, const char* objectName);
        template bool setObjectName<VkIndirectCommandsLayoutNVX>(VkDevice device, VkIndirectCommandsLayoutNVX object, const char* objectName);
        template bool setObjectName<VkDescriptorUpdateTemplateKHR>(VkDevice device, VkDescriptorUpdateTemplateKHR object, const char* objectName);
        template bool setObjectName<VkSamplerYcbcrConversionKHR>(VkDevice device, VkSamplerYcbcrConversionKHR object, const char* objectName);

        template bool setObjectName<VezCommandBuffer>(VkDevice device, VezCommandBuffer object, const char* objectName);

    }
}
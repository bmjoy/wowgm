#pragma once

#include <vulkan/vulkan.h>
#include <stdexcept>

namespace wowgm::graphics
{
    template<class VkParent, class VkChild>
    void DestroyVulkanObject(VkParent parent, VkChild object, VkAllocationCallbacks* callbacks = nullptr);

    template <> void DestroyVulkanObject(VkInstance instance, void*, VkAllocationCallbacks* callbacks) {
        vkDestroyInstance(instance, callbacks);
    }

    template <> void DestroyVulkanObject(VkDevice instance, void*, VkAllocationCallbacks* callbacks) {
        vkDestroyDevice(instance, callbacks);
    }

    template <> void DestroyVulkanObject<VkInstance, VkSurfaceKHR>(VkInstance parent, VkSurfaceKHR instance, VkAllocationCallbacks* callbacks) {
        vkDestroySurfaceKHR(parent, instance, callbacks);
    }

    template <typename T> struct has_parent_device : std::false_type { };

#define VK_DESTROY_SPECIALIZATION( VkChild , Deleter )                                    \
    template<>                                                                            \
    inline void DestroyVulkanObject<VkDevice, VkChild>(VkDevice device, VkChild object, VkAllocationCallbacks* callbacks) { \
        Deleter(device, object, callbacks);                                               \
    }                                                                                     \
                                                                                          \
    template <> struct has_parent_device<VkChild> : std::true_type { };

    VK_DESTROY_SPECIALIZATION(VkSemaphore, vkDestroySemaphore)
    // VK_DESTROY_SPECIALIZATION( VkCommandBuffer, vkFreeCommandBuffers ) <- command buffers are freed along with the pool
    VK_DESTROY_SPECIALIZATION(VkFence, vkDestroyFence)
    VK_DESTROY_SPECIALIZATION(VkDeviceMemory, vkFreeMemory)
    VK_DESTROY_SPECIALIZATION(VkBuffer, vkDestroyBuffer)
    VK_DESTROY_SPECIALIZATION(VkImage, vkDestroyImage)
    VK_DESTROY_SPECIALIZATION(VkEvent, vkDestroyEvent)
    VK_DESTROY_SPECIALIZATION(VkQueryPool, vkDestroyQueryPool)
    VK_DESTROY_SPECIALIZATION(VkBufferView, vkDestroyBufferView)
    VK_DESTROY_SPECIALIZATION(VkImageView, vkDestroyImageView)
    VK_DESTROY_SPECIALIZATION(VkShaderModule, vkDestroyShaderModule)
    VK_DESTROY_SPECIALIZATION(VkPipelineCache, vkDestroyPipelineCache)
    VK_DESTROY_SPECIALIZATION(VkPipelineLayout, vkDestroyPipelineLayout)
    VK_DESTROY_SPECIALIZATION(VkRenderPass, vkDestroyRenderPass)
    VK_DESTROY_SPECIALIZATION(VkPipeline, vkDestroyPipeline)
    VK_DESTROY_SPECIALIZATION(VkDescriptorSetLayout, vkDestroyDescriptorSetLayout)
    VK_DESTROY_SPECIALIZATION(VkSampler, vkDestroySampler)
    VK_DESTROY_SPECIALIZATION(VkDescriptorPool, vkDestroyDescriptorPool)
    // VK_DESTROY_SPECIALIZATION( VkDescriptorSet, vkFreeDescriptorSets ) <- descriptor sets are freed along with the pool
    VK_DESTROY_SPECIALIZATION(VkFramebuffer, vkDestroyFramebuffer)
    VK_DESTROY_SPECIALIZATION(VkCommandPool, vkDestroyCommandPool)
    VK_DESTROY_SPECIALIZATION(VkSwapchainKHR, vkDestroySwapchainKHR)

    template <typename T, typename Parent = void>
    class VulkanHandle
    {
    protected:
        virtual T AsHandle() = 0;

        virtual VkAllocationCallbacks* GetAllocationCallbacks() { return nullptr; }
        virtual Parent GetParentHandle() { }

        virtual ~VulkanHandle()
        {
            DestroyVulkanObject(GetParentHandle(), AsHandle(), GetAllocationCallbacks());
            AsHandle() = VK_NULL_HANDLE;
        }

    public:
        operator T() const { return AsHandle(); }
        operator bool() const { return AsHandle() != VK_NULL_HANDLE; }
    };
}
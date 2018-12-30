#include <graphics/vulkan/Queue.hpp>
#include <graphics/vulkan/Device.hpp>
#include <graphics/vulkan/CommandBuffer.hpp>
#include <graphics/vulkan/PhysicalDevice.hpp>
#include <graphics/vulkan/Instance.hpp>
#include <graphics/vulkan/Swapchain.hpp>
#include <graphics/vulkan/Mutator.hpp>

#include <boost/iterator/transform_iterator.hpp>

#include <mutex>
#include <thread>

namespace gfx::vk
{
    PFN_vkQueueEndDebugUtilsLabelEXT Queue::_vkQueueEndDebugUtilsLabelEXT;
    PFN_vkQueueBeginDebugUtilsLabelEXT Queue::_vkQueueBeginDebugUtilsLabelEXT;
    PFN_vkQueueInsertDebugUtilsLabelEXT Queue::_vkQueueInsertDebugUtilsLabelEXT;

    Queue::Queue(Device* device, VkQueue queue, uint32_t familyIndex, uint32_t index, const VkQueueFamilyProperties& properties)
    {
        _device = device;
        _queue = queue;
        _familyIndex = familyIndex;
        _index = index;
        _properties = properties;

        _InitializeLabelPointers();
    }

    VkResult Queue::WaitIdle()
    {
        return vkQueueWaitIdle(GetHandle());
    }

    VkResult Queue::WaitForFences(uint32_t fenceCount, VkFence* pFence, uint64_t timeout)
    {
        if (pFence != nullptr)
            return vkWaitForFences(GetDevice()->GetHandle(), fenceCount, pFence, VK_TRUE, timeout);

        return VK_SUCCESS;
    }

    VkResult Queue::ResetFences(uint32_t fenceCount, VkFence* pFence)
    {
        if (pFence != nullptr)
            return vkResetFences(GetDevice()->GetHandle(), fenceCount, pFence);

        return VK_SUCCESS;
    }

    VkResult Queue::AcquireNextImage(Swapchain* swapchain, uint32_t& imageIndex, VkSemaphore* pSemaphore /* = nullptr */, VkFence* pFence /* = nullptr */)
    {
        return vkAcquireNextImageKHR(_device->GetHandle(),
            swapchain->GetHandle(),
            std::numeric_limits<uint64_t>::max(),
            pSemaphore == nullptr ? VK_NULL_HANDLE : *pSemaphore,
            pFence == nullptr ? VK_NULL_HANDLE : *pFence,
            &imageIndex);
    }

    VkResult Queue::Submit(uint32_t submitCount, const SubmitInfo* pSubmits, VkFence* pFence)
    {
        std::vector<VkSubmitInfo> submitInfos(submitCount);

        std::vector<std::vector<VkCommandBuffer>> commandBuffers(submitCount);
        std::vector<std::vector<VkSemaphore>> waitSemaphores(submitCount);
        std::vector<std::vector<uint32_t>> waitDstStageMask(submitCount);

        for (uint32_t i = 0; i < submitCount; ++i)
        {
            auto& submitInfo = submitInfos[i];
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

            submitInfo.commandBufferCount = pSubmits[i].commandBuffers.size();
            for (auto&& itr : pSubmits[i].commandBuffers)
            {
                BOOST_ASSERT_MSG(itr != nullptr, "A command buffer is being submitted as nullptr!");
                commandBuffers[i].push_back(itr->GetHandle());
            }

            submitInfo.pCommandBuffers = commandBuffers[i].data();

            submitInfo.signalSemaphoreCount = pSubmits[i].signalSemaphores.size();
            submitInfo.pSignalSemaphores = pSubmits[i].signalSemaphores.data();

            submitInfo.waitSemaphoreCount = pSubmits[i].waitSemaphores.size();
            for (auto&& itr : pSubmits[i].waitSemaphores) {
                waitSemaphores[i].push_back(itr.first);
                waitDstStageMask[i].push_back(itr.second);
            }

            submitInfo.waitSemaphoreCount = pSubmits[i].waitSemaphores.size();
            submitInfo.pWaitSemaphores = waitSemaphores[i].data();
            submitInfo.pWaitDstStageMask = waitDstStageMask[i].data();
        }

        return vkQueueSubmit(GetHandle(),
            submitCount,
            submitInfos.data(),
            pFence == nullptr ? VK_NULL_HANDLE : *pFence);
    }

    VkResult Queue::Present(const PresentInfo* pPresentInfo)
    {
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = pPresentInfo->waitSemaphores.size();
        presentInfo.pWaitSemaphores = pPresentInfo->waitSemaphores.data();

        std::vector<VkSwapchainKHR> swapchains(pPresentInfo->swapchains.size());
        std::vector<uint32_t> imageIndices(pPresentInfo->swapchains.size());
        std::vector<VkResult> results(pPresentInfo->swapchains.size());

        uint32_t i = 0;
        for (auto&& itr : pPresentInfo->swapchains)
        {
            swapchains[i] = itr.swapchain->GetHandle();
            imageIndices[i] = itr.swapchain->GetImageIndex(itr.image);

            ++i;
        }

        presentInfo.pSwapchains = swapchains.data();
        presentInfo.swapchainCount = swapchains.size();

        presentInfo.pImageIndices = imageIndices.data();
        presentInfo.pResults = results.data();

        VkResult result = vkQueuePresentKHR(GetHandle(), &presentInfo);
        for (uint32_t i = 0; i < results.size(); ++i)
            const_cast<gfx::vk::PresentInfo::PresentChain&>(pPresentInfo->swapchains[i]).result = results[i];

        return result;
    }

    VkResult Queue::_InitializeLabelPointers()
    {
        static std::once_flag onceFlag;
        std::call_once(onceFlag, [&](VkInstance instance) -> void {
            _vkQueueEndDebugUtilsLabelEXT    = PFN_vkQueueEndDebugUtilsLabelEXT(vkGetInstanceProcAddr(instance, "vkQueueEndDebugUtilsLabelEXT"));
            _vkQueueBeginDebugUtilsLabelEXT  = PFN_vkQueueBeginDebugUtilsLabelEXT(vkGetInstanceProcAddr(instance, "vkQueueBeginDebugUtilsLabelEXT"));
            _vkQueueInsertDebugUtilsLabelEXT = PFN_vkQueueInsertDebugUtilsLabelEXT(vkGetInstanceProcAddr(instance, "vkQueueInsertDebugUtilsLabelEXT"));
        }, _device->GetPhysicalDevice()->GetInstance()->GetHandle());

        if (_vkQueueEndDebugUtilsLabelEXT == nullptr)
            return VK_ERROR_LAYER_NOT_PRESENT;

        return VK_SUCCESS;
    }

    VkResult Queue::BeginLabel(std::string_view label, float (&color)[4])
    {
        if (_vkQueueBeginDebugUtilsLabelEXT != nullptr)
        {
            VkDebugUtilsLabelEXT labelInfo{};
            labelInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
            memcpy(labelInfo.color, color, sizeof(color));
            labelInfo.pLabelName = label.data();

            _vkQueueBeginDebugUtilsLabelEXT(_queue, &labelInfo);
            return VK_SUCCESS;
        }

        return VK_ERROR_LAYER_NOT_PRESENT;
    }

    VkResult Queue::EndLabel()
    {
        if (_vkQueueEndDebugUtilsLabelEXT != nullptr)
        {
            _vkQueueEndDebugUtilsLabelEXT(_queue);
            return VK_SUCCESS;
        }

        return VK_ERROR_LAYER_NOT_PRESENT;
    }

    VkResult Queue::InsertLabel(std::string_view label, float (&color)[4])
    {
        if (_vkQueueInsertDebugUtilsLabelEXT != nullptr)
        {
            VkDebugUtilsLabelEXT labelInfo{};
            labelInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
            memcpy(labelInfo.color, color, sizeof(color));
            labelInfo.pLabelName = label.data();

            _vkQueueInsertDebugUtilsLabelEXT(_queue, &labelInfo);
            return VK_SUCCESS;
        }

        return VK_ERROR_LAYER_NOT_PRESENT;
    }
}

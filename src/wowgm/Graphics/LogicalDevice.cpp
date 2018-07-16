#include "LogicalDevice.hpp"
#include "PhysicalDevice.hpp"
#include "Queue.hpp"
#include "SynchronizationPrimitive.hpp"
#include "Assert.hpp"
#include "CommandBuffer.hpp"
#include "SwapChain.hpp"

#include "SharedGraphicsDefines.hpp"

#undef min
#undef max

namespace wowgm::graphics
{
    LogicalDevice::LogicalDevice(VkDevice device, QueueFamilyIndices& indices) : _device(device), _graphicsQueue(nullptr), _presentQueue(nullptr)
    {
        for (std::uint32_t i = 0; i < indices.GetQueueCount(); ++i)
        {
            auto queueIndice = indices.EnumerateFamilies()[i];

            VkQueue deviceQueue;
            vkGetDeviceQueue(device, queueIndice, 0, &deviceQueue);
            reinterpret_cast<Queue**>(&_graphicsQueue)[i] = new Queue(this, deviceQueue, queueIndice);
        }
    }

    LogicalDevice::~LogicalDevice()
    {
        delete _graphicsQueue;
        _graphicsQueue = nullptr;
        delete _presentQueue;
        _presentQueue = nullptr;

        vkDestroyDevice(_device, nullptr);
        _device = VK_NULL_HANDLE;
    }

    Queue* LogicalDevice::GetGraphicsQueue()
    {
        return _graphicsQueue;
    }

    Queue* LogicalDevice::GetPresentQueue()
    {
        return _presentQueue;
    }

    void LogicalDevice::Draw(SwapChain* swapChain)
    {
        std::uint32_t imageIndex;
        // TODO: Fix the swapchain here
        vkAcquireNextImageKHR(_device, *swapChain, std::numeric_limits<std::uint64_t>::max(), *_imageAvailable[currentFrame], VK_NULL_HANDLE, &imageIndex);

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { *_imageAvailable[currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;

        {
            // TODO: Cache this.
            auto mutator = [](CommandBuffer* buffer) -> VkCommandBuffer { return *buffer; };
            auto itr = boost::iterators::make_transform_iterator(_commandBuffers.begin(), mutator);
            auto end = boost::iterators::make_transform_iterator(_commandBuffers.end(), mutator);
            std::vector<VkCommandBuffer> buffers(_commandBuffers.size());
            buffers.insert(buffers.begin(), itr, end);
            submitInfo.pCommandBuffers = buffers.data();
        }

        VkSemaphore signalSemaphores[] = { *_renderFinished[currentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(*_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
            wowgm::exceptions::throw_with_trace(std::runtime_error("Failed to submit a draw command buffer!"));

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { *swapChain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;

        vkQueuePresentKHR(*_presentQueue, &presentInfo);
        vkQueueWaitIdle(*_presentQueue); // Fix me with flight frames

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }
}

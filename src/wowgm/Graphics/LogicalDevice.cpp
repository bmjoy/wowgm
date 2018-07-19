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

        for (std::uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            _imageAvailable[i] = new Semaphore(this);
            _renderFinished[i] = new Semaphore(this);
            _inflightFence[i] = new Fence(this);
        }
    }

    LogicalDevice::~LogicalDevice()
    {
        vkDeviceWaitIdle(_device);
        for (std::uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            delete _imageAvailable[i];
            delete _renderFinished[i];
            delete _inflightFence[i];
        }

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
        VkFence currentFence = *_inflightFence[currentFrame];

        VkResult result = vkWaitForFences(_device, 1, &currentFence, VK_TRUE, std::numeric_limits<std::uint64_t>::max());
        result = vkResetFences(_device, 1, &currentFence);

        std::uint32_t imageIndex;
        result = vkAcquireNextImageKHR(_device, *swapChain, std::numeric_limits<std::uint64_t>::max(), *_imageAvailable[currentFrame], currentFence, &imageIndex);

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { *_imageAvailable[currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = _commandBuffers.size();

        // TODO: Cache this.
        auto mutator = [](CommandBuffer* buffer) -> VkCommandBuffer { return *buffer; };
        auto itr = boost::iterators::make_transform_iterator(_commandBuffers.begin(), mutator);
        auto end = boost::iterators::make_transform_iterator(_commandBuffers.end(), mutator);
        std::vector<VkCommandBuffer> buffers(itr, end);
        submitInfo.pCommandBuffers = buffers.data();

        VkSemaphore signalSemaphores[] = { *_renderFinished[currentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        result = vkQueueSubmit(*_graphicsQueue, 1, &submitInfo, currentFence);
        if (result != VK_SUCCESS)
            wowgm::exceptions::throw_with_trace(std::runtime_error("Failed to submit a draw command buffer!"));

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { *swapChain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR(*_presentQueue, &presentInfo);
        result = vkQueueWaitIdle(*_presentQueue); // Fix me with flight frames

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void LogicalDevice::AddCommandBuffer(CommandBuffer* buffer)
    {
        _commandBuffers.push_back(buffer);
    }
}

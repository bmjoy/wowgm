#include "LogicalDevice.hpp"
#include "PhysicalDevice.hpp"
#include "SynchronizationPrimitive.hpp"
#include "Assert.hpp"
#include "CommandBuffer.hpp"
#include "SwapChain.hpp"
#include "SharedGraphicsDefines.hpp"
#include "RenderPass.hpp"

#include <limits>
#include <boost/iterator/transform_iterator.hpp>

// Fucking hell.
#undef min
#undef max
#undef CreateSemaphore

namespace wowgm::graphics
{
    LogicalDevice::LogicalDevice(VkDevice device, QueueFamilyIndices& indices) : _device(device)
    {
        auto getDeviceQueue = [](VkDevice& device, std::int32_t indice) -> VkQueue {
            VkQueue queue;
            vkGetDeviceQueue(device, indice, 0, &queue);
            return queue;
        };

        _graphicsQueue = CreateQueue(VK_QUEUE_GRAPHICS_BIT, getDeviceQueue(device, indices.Graphics), indices.Graphics);
        CreateQueue(VK_QUEUE_COMPUTE_BIT,  getDeviceQueue(device, indices.Compute),  indices.Compute);
        // CreateQueue(VK_QUEUE_TRANSFER_BIT, getDeviceQueue(device, indices.Transfer), indices.Transfer);

        for (auto&& itr : _ownedQueues)
        {
            if (itr->GetFamilyIndice() == indices.Present)
            {
                _presentQueue = itr;
                break;
            }
        }

        for (std::uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            _imageAvailable[i] = CreateSemaphore();
            _renderFinished[i] = CreateSemaphore();
            _inflightFence[i] = CreateFence();
        }
    }

    LogicalDevice::~LogicalDevice()
    {
        // Wait for work to be done on the device before cleaning up
        WaitIdle();

        for (auto&& itr : _ownedFences)
            delete itr;
        _ownedFences.clear();

        for (auto&& itr : _ownedSemaphores)
            delete itr;
        _ownedSemaphores.clear();

        for (auto&& itr : _ownedCommandBuffers)
            for (auto&& itr2 : itr.second)
                delete itr2;
        _ownedCommandBuffers.clear();

        for (auto&& itr : _ownedQueues)
            delete itr;
        _ownedQueues.clear();

        for (auto&& itr : _ownedRenderPasses)
            delete itr;
        _ownedRenderPasses.clear();

        vkDestroyDevice(_device, nullptr);
        _device = VK_NULL_HANDLE;

        // Housekeeping, nullptr everything
        _graphicsQueue = nullptr;
        _presentQueue = nullptr;

        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            _inflightFence[i] = nullptr;
            _imageAvailable[i] = nullptr;
            _renderFinished[i] = nullptr;
        }
    }

    void LogicalDevice::WaitIdle()
    {
        vkDeviceWaitIdle(_device);
    }

    Queue* LogicalDevice::GetGraphicsQueue()
    {
        return _graphicsQueue;
    }

    Queue* LogicalDevice::GetPresentQueue()
    {
        return _presentQueue;
    }

    std::uint32_t LogicalDevice::AcquireNextImage(SwapChain* swapChain)
    {
        constexpr static const std::uint64_t MAX_TIMEOUT = std::numeric_limits<std::uint64_t>::max();

        VkResult result = vkWaitForFences(_device, _waitFences.size(), _waitFences.data(), VK_TRUE, MAX_TIMEOUT);

        std::uint32_t imageIndex;
        result = vkAcquireNextImageKHR(_device, *swapChain, MAX_TIMEOUT, *_imageAvailable[_currentFrame], VK_NULL_HANDLE, &imageIndex);
        result = vkResetFences(_device, _waitFences.size(), _waitFences.data());

        _waitFences.clear();

        return imageIndex;
    }

    void LogicalDevice::AddWaitFence(Fence* fence)
    {
        _waitFences.push_back(*fence);
    }

    void LogicalDevice::AddWaitFence(VkFence fence)
    {
        _waitFences.push_back(fence);
    }

    void LogicalDevice::Submit(std::uint32_t imageIndex, Fence* submitFence)
    {
        VkFence currentFence = *submitFence;

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { *_imageAvailable[_currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores; // Wait for this semaphore to signal before executing
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = _ownedCommandBuffers[imageIndex].size();
        auto fn = [](CommandBuffer* b) -> VkCommandBuffer { return *b; };
        auto itr = boost::make_transform_iterator(_ownedCommandBuffers[imageIndex].begin(), fn);
        auto end = boost::make_transform_iterator(_ownedCommandBuffers[imageIndex].end(), fn);
        std::vector<VkCommandBuffer> commandBuffers(itr, end);

        submitInfo.pCommandBuffers = commandBuffers.data();

        VkSemaphore signalSemaphores[] = { *_renderFinished[_currentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores; // Signal this semaphore when submitted

        VkResult result = vkQueueSubmit(*_graphicsQueue, 1, &submitInfo, currentFence);
        if (result != VK_SUCCESS)
            wowgm::exceptions::throw_with_trace<std::runtime_error>("Failed to submit a draw command buffer!");
    }

    void LogicalDevice::Present(std::uint32_t imageToPresent, SwapChain* swapChain, Semaphore* waitSemaphore)
    {
        VkSemaphore waitSemaphore_ = *waitSemaphore;

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &waitSemaphore_;

        VkSwapchainKHR swapChains[] = { *swapChain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageToPresent;

        VkResult result = vkQueuePresentKHR(*_presentQueue, &presentInfo);
        if (result != VK_SUCCESS)
            wowgm::exceptions::throw_with_trace<std::runtime_error>("Unable to present");

        _currentFrame = (_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void LogicalDevice::AddCommandBuffer(std::uint32_t frameIndex, CommandBuffer* buffer)
    {
        _ownedCommandBuffers[frameIndex].push_back(buffer);
    }

    Semaphore* LogicalDevice::GetImageAvailableSemaphore()
    {
        return _imageAvailable[_currentFrame];
    }

    Semaphore* LogicalDevice::GetSignalSemaphore()
    {
        return _renderFinished[_currentFrame];
    }

    Fence* LogicalDevice::GetFlightFence()
    {
        return _inflightFence[_currentFrame];
    }

    Semaphore* LogicalDevice::CreateSemaphore()
    {
        Semaphore* semaphore = new Semaphore(this);
        _ownedSemaphores.push_back(semaphore);
        return semaphore;
    }

    Fence* LogicalDevice::CreateFence()
    {
        Fence* fence = new Fence(this);
        _ownedFences.push_back(fence);
        return fence;
    }

    Queue* LogicalDevice::CreateQueue(VkQueueFlagBits queueType, VkQueue queueObject, std::int32_t indice)
    {
        Queue* queue = new Queue(this, queueType, queueObject, indice);
        _ownedQueues.push_back(queue);
        return queue;
    }

    RenderPass*  LogicalDevice::CreateRenderPass()
    {
        RenderPass* renderPass = new RenderPass(this);
        _ownedRenderPasses.push_back(renderPass);
        return renderPass;
    }
}

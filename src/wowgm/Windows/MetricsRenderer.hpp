#pragma once

#include "Renderer.hpp"

#include <shared/assert/assert.hpp>
#include <shared/log/log.hpp>

#include <graphics/vulkan/CommandBuffer.hpp>
#include <graphics/vulkan/Device.hpp>
#include <graphics/vulkan/PhysicalDevice.hpp>

#include <vulkan/vulkan.h>
#include <boost/core/demangle.hpp>

#include <type_traits>
#include <chrono>

namespace gfx::vk
{
    class Swapchain;
}

namespace wowgm
{
    // Quick hack to circumvent MetricsRenderer<MetricsRenderer<T>>
    template <typename T>
    struct metrics_traits {
        using base_type = T;
        using type      = T;
    };

    template <typename T> class MetricsRenderer;

    template <typename T>
    struct metrics_traits<MetricsRenderer<T>> {
        using base_type = typename metrics_traits<T>::base_type;
        using type      = MetricsRenderer<T>;
    };

    /**
     * A renderer that measures the execution of a series of commands issued by another renderer.
     */
    template <typename T>
    class MetricsRenderer final : public T
    {
    public:
        static_assert(std::is_base_of<Renderer, T>::value, "yikes");

        using base_t = typename metrics_traits<T>::base_type;

        using hrc = std::chrono::high_resolution_clock;

        MetricsRenderer(gfx::vk::Swapchain* swapchain) : base_t(swapchain)
        {
            _queryPool = VK_NULL_HANDLE;

            // Before doing anything, we check if the device supports timestamps on all graphics and compute queues.
            bool supported = false;
            if (GetDevice()->GetPhysicalDevice()->GetProperties().limits.timestampComputeAndGraphics == VK_TRUE)
                supported = true;

            if (!supported)
            {
                LOG_GRAPHICS("Timestamp queries are not supported and will thus be skipped.");
                return;
            }

            VkQueryPoolCreateInfo queryPoolCreateInfo{};
            queryPoolCreateInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
            // We want timestamps and we only need two queries - one for storing the timestamp before execution, and one afterwards.
            queryPoolCreateInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
            queryPoolCreateInfo.queryCount = 2;

            VkResult result = vkCreateQueryPool(GetDevice()->GetHandle(), &queryPoolCreateInfo, nullptr, &_queryPool);
            BOOST_ASSERT_MSG(result == VK_SUCCESS, "Failed to allocate a timestamps query pool");
        }

        ~MetricsRenderer()
        {
            vkDestroyQueryPool(GetDevice()->GetHandle(), _queryPool, nullptr);
        }

        void beforeRenderQuery(gfx::vk::CommandBuffer* commandBuffer) override
        {
            if (_queryPool != VK_NULL_HANDLE)
            {
                vkCmdResetQueryPool(commandBuffer->GetHandle(), _queryPool, 0, 2);
            }

            base_t::beforeRenderQuery(commandBuffer);
        }

        void onRenderQuery(gfx::vk::CommandBuffer* commandBuffer) override
        {
            if (_queryPool != VK_NULL_HANDLE)
                vkCmdWriteTimestamp(commandBuffer->GetHandle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, _queryPool, 0);

            auto startTime = hrc::now();
            base_t::onRenderQuery(commandBuffer);
            _cpuTime = hrc::now() - startTime;

            if (_queryPool != VK_NULL_HANDLE)
                vkCmdWriteTimestamp(commandBuffer->GetHandle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, _queryPool, 1);
        }

        void afterRenderQuery(gfx::vk::CommandBuffer* commandBuffer, std::vector<std::pair<VkSemaphore, VkPipelineStageFlags>>& waitSemaphores) override
        {
            base_t::afterRenderQuery(commandBuffer, waitSemaphores);

            if (_queryPool == VK_NULL_HANDLE)
                return;

            uint64_t timers[2];

            VkResult result = vkGetQueryPoolResults(GetDevice()->GetHandle(),
                _queryPool, 0, 2, sizeof(uint64_t) * 2, timers, 0,
                VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);
            BOOST_ASSERT_MSG(result == VK_SUCCESS, "Failed to acquire the timestamps");

            // Constexpr so that its just not a magic number. used for demangling
            constexpr const char ns[] = "class wowgm::";

            uint64_t interval = timers[1] - timers[0];
            double executionTime = interval * GetDevice()->GetPhysicalDevice()->GetProperties().limits.timestampPeriod / 1000000.0f;
            LOG_GRAPHICS("{} - CPU: {:01.3f} ms, GPU: {:01.3f} ms",
                boost::core::demangle(typeid(T).name()).substr(sizeof(ns) - 1),
                std::chrono::duration_cast<std::chrono::microseconds>(_cpuTime).count() / 1000.0f,
                executionTime);
        }

    private:
        VkQueryPool _queryPool;
        hrc::duration _cpuTime;
    };

}
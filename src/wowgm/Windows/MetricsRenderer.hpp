#pragma once

#include "Renderer.hpp"

#include <shared/assert/assert.hpp>
#include <shared/log/log.hpp>

#include <graphics/vulkan/CommandBuffer.hpp>
#include <graphics/vulkan/Device.hpp>
#include <graphics/vulkan/PhysicalDevice.hpp>

#include <vulkan/vulkan.h>
#include <type_traits>

namespace gfx::vk
{
    class Swapchain;
}

namespace wowgm
{

    template <typename T>
    struct metrics_traits
    {
        using base_type = T;
        using type = T;
    };

    template <typename T> class MetricsRenderer;

    template <typename T>
    struct metrics_traits<MetricsRenderer<T>> {
        using base_type = T;
        using type = MetricsRenderer<T>;
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

        MetricsRenderer(gfx::vk::Swapchain* swapchain) : base_t(swapchain)
        {
            _queryPool = VK_NULL_HANDLE;

            // Before doing anything, we check if the device supports timestamps on all graphics and compute queues.
            bool supported = false;
            if (GetDevice()->GetPhysicalDevice()->GetPhysicalDeviceProperties().limits.timestampComputeAndGraphics == VK_TRUE)
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
            if (_queryPool == VK_NULL_HANDLE)
                return;

            vkCmdResetQueryPool(commandBuffer->GetHandle(), _queryPool, 0, 2);
        }

        void onRenderQuery(gfx::vk::CommandBuffer* commandBuffer) override
        {
            if (_queryPool != VK_NULL_HANDLE)
                vkCmdWriteTimestamp(commandBuffer->GetHandle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, _queryPool, 0);

            base_t::onRenderQuery(commandBuffer);

            if (_queryPool != VK_NULL_HANDLE)
                vkCmdWriteTimestamp(commandBuffer->GetHandle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, _queryPool, 1);
        }

        void afterRenderQuery(gfx::vk::CommandBuffer* commandBuffer) override
        {
            if (_queryPool == VK_NULL_HANDLE)
                return;

            uint64_t timers[2];

            VkResult result = vkGetQueryPoolResults(GetDevice()->GetHandle(),
                _queryPool, 0, 2, sizeof(uint64_t) * 2, timers, 0,
                VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);
            BOOST_ASSERT_MSG(result == VK_SUCCESS, "Failed to acquire the timestamps");

            uint64_t interval = timers[1] - timers[0];
            double executionTime = interval * GetDevice()->GetPhysicalDevice()->GetPhysicalDeviceProperties().limits.timestampPeriod;
            LOG_GRAPHICS("Execution time {}", executionTime);
        }

    private:
        VkQueryPool _queryPool;
    };

}
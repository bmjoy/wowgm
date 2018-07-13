#pragma once

#include <vulkan/vulkan.h>
#include <vector>

#include "Pipeline.hpp"

namespace wowgm::graphics
{
    class Pipeline;
    class SwapChain;

    class PipelineCache
    {
        PipelineCache(SwapChain* swapChain);
        ~PipelineCache();

        void Insert(Pipeline* pipeline);

        operator VkPipelineCache() const { return _cache; }

        // Cookbook 824/1166
        void CreatePipelines();

    private:
        SwapChain* _swapchain;
        VkPipelineCache _cache;
        std::vector<Pipeline*> _pipelines;
    };
}
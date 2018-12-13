#pragma once

#include <vector>
#include <cstdint>
#include <vulkan/vulkan.h>

namespace gfx::vk
{
    struct PipelineResource;

    namespace SPIRV
    {
        bool ReflectResources(std::vector<uint32_t> const& spirvCode, VkShaderStageFlagBits shaderStage, std::vector<PipelineResource>& resourceContainer);
    }
}
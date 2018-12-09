#pragma once

#include <vector>
#include <cstdint>
#include <vulkan/vulkan.h>

namespace vez
{
    struct PipelineResource;

    namespace SPIRV
    {
        bool ReflectResources(std::vector<std::uint32_t> const& spirvCode, VkShaderStageFlagBits shaderStage, std::vector<PipelineResource>& resourceContainer);
    }
}
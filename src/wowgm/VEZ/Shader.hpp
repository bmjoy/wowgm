#pragma once

#include <vector>
#include <string>
#include <string_view>

#include "VEZ.hpp"
#include "NamedObject.hpp"

namespace vez
{
    class Shader : public NamedObject<Shader>
    {
    public:
        static VkResult Create(Device* pDevice, const ShaderModuleCreateInfo* pCreateInfo, Shader** pShader);

        VkShaderModule GetHandle() const { return _handle; }

        VkShaderStageFlagBits GetStage() const { return _stage; }

        const std::string_view GetEntryPoint() const { return std::string_view(_entryPoint); }

        const std::vector<PipelineResource>& GetResources() const { return _resources; }

        ShaderModuleCreateInfo const& GetCreateInfo() const { return _createInfo; }

        Instance* GetInstance() const override;
        Device* GetDevice() const override;

    private:
        Device* _device = nullptr;

        VkShaderStageFlagBits _stage;

        VkShaderModule _handle = VK_NULL_HANDLE;

        std::vector<PipelineResource> _resources;

        std::string _entryPoint;
        std::vector<std::uint32_t> _spirv;

        ShaderModuleCreateInfo _createInfo;
    };
}

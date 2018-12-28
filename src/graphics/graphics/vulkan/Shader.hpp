#pragma once

#include <vector>
#include <string>
#include <string_view>
#include <cstdint>

#include <graphics/vulkan/VK.hpp>

namespace gfx::vk
{
    class Device;

    class Shader final // : public NamedObject<Shader>
    {
    public:
        ~Shader();

        static Shader* FromDisk(Device* device, const char* filePath, const char* entryPoint, VkShaderStageFlagBits stage);

        static VkResult Create(Device* pDevice, const ShaderModuleCreateInfo* pCreateInfo, Shader** pShader);

        VkShaderModule GetHandle() const { return _handle; }

        VkShaderStageFlagBits GetStage() const { return _stage; }

        const std::string_view GetEntryPoint() const { return std::string_view(_entryPoint); }

        const std::vector<PipelineResource>& GetResources() const { return _resources; }

        Instance* GetInstance() const;
        Device* GetDevice() const;

    private:
        Device* _device = nullptr;

        VkShaderStageFlagBits _stage;

        VkShaderModule _handle = VK_NULL_HANDLE;

        std::vector<PipelineResource> _resources;

        std::string _entryPoint;
        std::vector<uint8_t> _spirv;
    };
}

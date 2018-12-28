#include <graphics/vulkan/Shader.hpp>
#include <graphics/vulkan/Reflection.hpp>
#include <graphics/vulkan/Instance.hpp>

#include <shared/assert/assert.hpp>

#include <fstream>

namespace gfx::vk
{
    Shader* Shader::FromDisk(Device* device, const char* filePath, const char* entryPoint, VkShaderStageFlagBits stage)
    {
        std::ifstream file(filePath, std::ifstream::binary | std::ifstream::ate);

        if (!file.is_open())
            shared::assert::throw_with_trace("Unable to open the shader located at {}", filePath);

        ShaderModuleCreateInfo shaderCreateInfo;
        shaderCreateInfo.code.resize(file.tellg());

        file.seekg(0);
        file.read(reinterpret_cast<char*>(shaderCreateInfo.code.data()), shaderCreateInfo.code.size());
        file.close();

        shaderCreateInfo.stage = stage;
        shaderCreateInfo.pEntryPoint = entryPoint;

        Shader* shader = nullptr;
        Shader::Create(device, &shaderCreateInfo, &shader); // Check done within
        return shader;
    }

    VkResult Shader::Create(Device* pDevice, const ShaderModuleCreateInfo* pCreateInfo, Shader** ppShader)
    {
        Shader* shaderModule = new Shader;
        shaderModule->_device = pDevice;
        shaderModule->_stage = pCreateInfo->stage;
        shaderModule->_entryPoint = pCreateInfo->pEntryPoint;

        VkResult result = VK_SUCCESS;
        if (pCreateInfo->pGLSLSource != nullptr)
        {
            // Compile here
        }
        else
        {
            shaderModule->_spirv.resize(pCreateInfo->code.size());
            memcpy(shaderModule->_spirv.data(), pCreateInfo->code.data(), pCreateInfo->code.size());
        }

        if (result != VK_SUCCESS)
        {
            delete shaderModule;
            return result;
        }

        if (!SPIRV::ReflectResources(shaderModule->_spirv, pCreateInfo->stage, shaderModule->_resources))
            return VK_ERROR_INITIALIZATION_FAILED;

        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.pNext = pCreateInfo->pNext;
        createInfo.codeSize = shaderModule->_spirv.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderModule->_spirv.data());
        result = vkCreateShaderModule(pDevice->GetHandle(), &createInfo, nullptr, &shaderModule->_handle);
        if (result != VK_SUCCESS)
        {
            delete shaderModule;
            return result;
        }

        *ppShader = shaderModule;
        return result;
    }

    Instance* Shader::GetInstance() const
    {
        return _device->GetInstance();
    }

    Device* Shader::GetDevice() const
    {
        return _device;
    }

    Shader::~Shader()
    {
        vkDestroyShaderModule(_device->GetHandle(), _handle, nullptr);
    }
}

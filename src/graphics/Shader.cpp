#include "Shader.hpp"
#include "Reflection.hpp"
#include "Instance.hpp"

namespace vez
{
    VkResult Shader::Create(Device* pDevice, const ShaderModuleCreateInfo* pCreateInfo, Shader** ppShader)
    {
        Shader* shaderModule = new Shader;
        shaderModule->_device = pDevice;
        shaderModule->_stage = pCreateInfo->stage;

        VkResult result = VK_SUCCESS;
        if (pCreateInfo->pGLSLSource != nullptr)
        {
            // Compile here
        }
        else
        {
            shaderModule->_spirv.resize(pCreateInfo->codeSize);
            memcpy(shaderModule->_spirv.data(), pCreateInfo->pCode, pCreateInfo->codeSize);
        }

        if (result != VK_SUCCESS)
        {
            delete shaderModule;
            return result;
        }

        if (!SPIRV::ReflectResources(shaderModule->_spirv, pCreateInfo->stage, shaderModule->_resources))
            return VK_ERROR_INITIALIZATION_FAILED;

        VkShaderModuleCreateInfo moduleCreateInfo{};
        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.pNext = pCreateInfo->pNext;
        createInfo.codeSize = shaderModule->_spirv.size() * sizeof(uint32_t);
        createInfo.pCode = shaderModule->_spirv.data();
        result = vkCreateShaderModule(pDevice->GetHandle(), &createInfo, nullptr, &shaderModule->_handle);
        if (result != VK_SUCCESS)
        {
            delete shaderModule;
            return result;
        }

        memcpy(&shaderModule->_createInfo, pCreateInfo, sizeof(ShaderModuleCreateInfo));

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
}

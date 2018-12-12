#include "DescriptorSetLayout.hpp"
#include "Device.hpp"
#include "DescriptorPool.hpp"
#include "VEZ.hpp"
#include "Helpers.hpp"

#include <extstd/containers/extract.hpp>

namespace vez
{
    VkResult DescriptorSetLayout::Create(Device* device, const std::vector<PipelineResource>& setResources, DescriptorSetLayout** pLayout)
    {
        auto descriptorSetLayout = new DescriptorSetLayout;
        descriptorSetLayout->_device = device;

        // Static mapping between VulkanEZ pipeline resource types to Vulkan descriptor types.
        std::unordered_map<PipelineResourceType, VkDescriptorType> resourceTypeMapping = {
            { PIPELINE_RESOURCE_TYPE_SAMPLER, VK_DESCRIPTOR_TYPE_SAMPLER },
            { PIPELINE_RESOURCE_TYPE_COMBINED_IMAGE_SAMPLER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER },
            { PIPELINE_RESOURCE_TYPE_SAMPLED_IMAGE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE },
            { PIPELINE_RESOURCE_TYPE_STORAGE_IMAGE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE },
            { PIPELINE_RESOURCE_TYPE_UNIFORM_TEXEL_BUFFER, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER },
            { PIPELINE_RESOURCE_TYPE_STORAGE_TEXEL_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER },
            { PIPELINE_RESOURCE_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER },
            { PIPELINE_RESOURCE_TYPE_STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER },
            { PIPELINE_RESOURCE_TYPE_INPUT_ATTACHMENT, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT },
        };

        for (auto&& resource : setResources)
        {
            switch (resource.resourceType)
            {
                case PIPELINE_RESOURCE_TYPE_INPUT:
                case PIPELINE_RESOURCE_TYPE_OUTPUT:
                case PIPELINE_RESOURCE_TYPE_PUSH_CONSTANT_BUFFER:
                    // Skip these.
                    continue;
            }

            // Convert from VkPipelineResourceType to VkDescriptorType.
            auto descriptorType = resourceTypeMapping.at(resource.resourceType);

            // Populate the Vulkan binding info struct.
            VkDescriptorSetLayoutBinding bindingInfo = {};
            bindingInfo.binding = resource.binding;
            bindingInfo.descriptorCount = resource.arraySize;
            bindingInfo.descriptorType = resourceTypeMapping.at(resource.resourceType);
            bindingInfo.stageFlags = resource.stages;
            descriptorSetLayout->_bindings.emplace(resource.binding, bindingInfo);
        }

        VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
        layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutCreateInfo.bindingCount = static_cast<uint32_t>(descriptorSetLayout->_bindings.size());
        layoutCreateInfo.pBindings = extstd::values(descriptorSetLayout->_bindings).data();
        auto result = vkCreateDescriptorSetLayout(descriptorSetLayout->_device->GetHandle(), &layoutCreateInfo, nullptr, &descriptorSetLayout->_handle);
        if (result != VK_SUCCESS)
        {
            delete descriptorSetLayout;
            return result;
        }

        // Allocate a DescriptorPool from the new instance.
        descriptorSetLayout->_descriptorPool = new DescriptorPool(descriptorSetLayout);

        // Save handle.
        *pLayout = descriptorSetLayout;

        // Return success.
        return VK_SUCCESS;
    }

    DescriptorSetLayout::~DescriptorSetLayout()
    {
        vkDestroyDescriptorSetLayout(_device->GetHandle(), _handle, nullptr);
        delete _descriptorPool;
    }

    bool DescriptorSetLayout::GetLayoutBinding(uint32_t bindingIndex, VkDescriptorSetLayoutBinding** pBinding)
    {
        auto it = _bindings.find(bindingIndex);
        if (it == _bindings.end())
            return false;

        *pBinding = &it->second;
        return true;
    }

    VkDescriptorSet DescriptorSetLayout::AllocateDescriptorSet()
    {
        // Return new descriptor set allocation.
        return _descriptorPool->AllocateDescriptorSet();
    }

    VkResult DescriptorSetLayout::FreeDescriptorSet(VkDescriptorSet descriptorSet)
    {
        // Free descriptor set handle.
        return _descriptorPool->FreeDescriptorSet(descriptorSet);
    }
}

#include <graphics/vulkan/DescriptorSetLayout.hpp>
#include <graphics/vulkan/Device.hpp>
#include <graphics/vulkan/DescriptorPool.hpp>
#include <graphics/vulkan/VK.hpp>
#include <graphics/vulkan/Helpers.hpp>
#include <graphics/vulkan/Sampler.h>
#include <graphics/vulkan/ImageView.hpp>

#include <extstd/containers/extract.hpp>

namespace gfx::vk
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

        auto bindings = extstd::values(descriptorSetLayout->_bindings);

        VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
        layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutCreateInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutCreateInfo.pBindings = bindings.data();
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
    }

    VkDescriptorSetLayoutBinding const& DescriptorSetLayout::GetBinding(uint32_t bindingIndex)
    {
        return _bindings[bindingIndex];
    }

    VkDescriptorSet const& DescriptorSetLayout::GetDescriptorSet(uint32_t bindingIndex)
    {
        return _descriptorSets[bindingIndex];
    }

    void DescriptorSetLayout::UpdateBinding(uint32_t bindingIndex, Sampler* sampler, ImageView* imageView, VkImageLayout layout)
    {
        VkDescriptorSetLayoutBinding const& binding = GetBinding(bindingIndex);

        VkDescriptorImageInfo imageInfo;
        imageInfo.imageLayout = layout;
        imageInfo.imageView = imageView->GetHandle();
        imageInfo.sampler = sampler->GetHandle();

        VkWriteDescriptorSet writeSet;
        writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeSet.pNext = nullptr;

        writeSet.descriptorCount = 1;
        writeSet.descriptorType = binding.descriptorType;
        writeSet.dstBinding = binding.binding;
        writeSet.dstArrayElement = 0; // TODO: ?

        _descriptorSets[bindingIndex] = AllocateDescriptorSet();
        writeSet.dstSet = _descriptorSets[bindingIndex];

        writeSet.pImageInfo = &imageInfo;
        writeSet.pBufferInfo = nullptr;
        writeSet.pTexelBufferView = nullptr;

        vkUpdateDescriptorSets(GetDevice()->GetHandle(), 1, &writeSet, 0, nullptr);
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

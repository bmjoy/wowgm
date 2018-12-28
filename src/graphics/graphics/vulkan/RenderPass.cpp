#include <graphics/vulkan/RenderPass.hpp>
#include <graphics/vulkan/Device.hpp>

#include <shared/assert/assert.hpp>

#include <extstd/containers/extract.hpp>

#include <set>
#include <stdexcept>

namespace gfx::vk
{
    RenderPass::RenderPass(Device* device)
    {
        _device = device;
    }

    RenderPass::~RenderPass()
    {
        vkDestroyRenderPass(_device->GetHandle(), _handle, nullptr);

        _attachments.clear();
        _subpasses.clear();
    }

    uint32_t RenderPass::AddAttachment(AttachmentType type, VkAttachmentDescription attachmentDescription)
    {
        if (_handle != VK_NULL_HANDLE)
            shared::assert::throw_with_trace("Tried to add an attachment to a renderpass that has already been finalized");

        _attachments[type].push_back(attachmentDescription);
        return _attachments[type].size() - 1;
    }

    void RenderPass::BeginSubpass()
    {
        if (_handle != VK_NULL_HANDLE)
            shared::assert::throw_with_trace("Tried to begin a subpass on a render pass that has already been finalized");

        if (_currentSubpass != nullptr)
            shared::assert::throw_with_trace("Did you forget to call RenderPass::FinalizeSubpass() ?");

        _currentSubpass = &_subpasses.emplace_back(std::make_pair(VkSubpassDescription{}, attachment_reference_map{}));
    }

    void RenderPass::AddAttachmentReference(AttachmentType type, uint32_t attachmentIndex, VkImageLayout layout)
    {
        if (_handle != VK_NULL_HANDLE)
            shared::assert::throw_with_trace("Tried to add an attachment reference on a render pass that has already been finalized");

        if (_currentSubpass == nullptr)
            shared::assert::throw_with_trace("Did you forget to call RenderPass::BeginSubpass() ?");

        auto& attachments = _attachments[type];
        if (attachmentIndex < 0 || attachmentIndex >= attachments.size())
            shared::assert::throw_with_trace<std::out_of_range>("");

        _currentSubpass->second[type].push_back(VkAttachmentReference{ attachmentIndex, layout });
    }

    void RenderPass::FinalizeSubpass()
    {
        if (_handle != VK_NULL_HANDLE)
            shared::assert::throw_with_trace("Tried to finalize a subpass on a render pass that has already been finalized");

        if (_currentSubpass == nullptr)
            shared::assert::throw_with_trace<std::runtime_error>("Did you forget to call RenderPass::BeginSubpass() ?");

        uint32_t subpassIndex = _subpasses.size() - 1;

        auto currentSubpass = &_currentSubpass->first;

        currentSubpass->pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

        currentSubpass->pColorAttachments = _currentSubpass->second[AttachmentType::Color].data();
        currentSubpass->colorAttachmentCount = _currentSubpass->second[AttachmentType::Color].size();

        currentSubpass->pInputAttachments = _currentSubpass->second[AttachmentType::Input].data();
        currentSubpass->inputAttachmentCount = _currentSubpass->second[AttachmentType::Input].size();

        // TODO: Resolve and DepthStencil

        _currentSubpass = nullptr;
    }

    void RenderPass::Finalize()
    {
        if (_handle != VK_NULL_HANDLE)
            return;

        if (_currentSubpass != nullptr)
            shared::assert::throw_with_trace<std::runtime_error>("Did you forget to call RenderPass::FinalizeSubpass() ?");


        VkRenderPassCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

        createInfo.subpassCount = _subpasses.size();

        auto subpasses = extstd::keys(_subpasses);
        createInfo.pSubpasses = subpasses.data();

        std::vector<VkAttachmentDescription> attachments;

        std::unordered_map<AttachmentType, uint32_t> attachmentOffsets;

        uint32_t offset = 0;
        for (auto&& kv : _attachments)
        {
            attachments.insert(attachments.end(), kv.second.begin(), kv.second.end());
            attachmentOffsets[kv.first] = offset;
            offset += kv.second.size();
        }

        // This is a dirty way to fix the attachment indices since they all are supposed to be in the same map
        for (auto&& subpass : _subpasses)
        {
            for (uint32_t i = 0; i < subpass.first.colorAttachmentCount; ++i)
                const_cast<VkAttachmentReference*>(&subpass.first.pColorAttachments[i])->attachment += attachmentOffsets[AttachmentType::Color];

            for (uint32_t i = 0; i < subpass.first.inputAttachmentCount; ++i)
                const_cast<VkAttachmentReference*>(&subpass.first.pInputAttachments[i])->attachment += attachmentOffsets[AttachmentType::Input];

            // TODO: Do the same for resolve, depthStencil, and preserve attachment
        }

        createInfo.pAttachments = attachments.data();
        createInfo.attachmentCount = attachments.size();

        createInfo.pDependencies = nullptr;

        if (vkCreateRenderPass(_device->GetHandle(), &createInfo, nullptr, &_handle) != VK_SUCCESS)
            shared::assert::throw_with_trace<std::runtime_error>("Failed to create render pass handle");
    }

    VkRenderPass RenderPass::GetHandle()
    {
        Finalize();
        return _handle;
    }
}

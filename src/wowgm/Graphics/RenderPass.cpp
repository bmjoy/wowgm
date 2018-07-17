#include "RenderPass.hpp"
#include "Subpass.hpp"
#include "LogicalDevice.hpp"
#include "Assert.hpp"
#include "LogicalDevice.hpp"

#include <boost/iterator/transform_iterator.hpp>

namespace wowgm::graphics
{
    RenderPass::RenderPass(LogicalDevice* device) : _device(device)
    {
        _renderPass = VK_NULL_HANDLE;

        VkSubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        SetDependency(dependency);

        _CreateDefaultSubpass();
    }

    RenderPass::~RenderPass()
    {
        if (_renderPass != VK_NULL_HANDLE)
            vkDestroyRenderPass(*_device, _renderPass, nullptr);

        for (std::uint32_t i = 0; i < _subpasses.size(); ++i)
            delete _subpasses[i];

        _subpasses.clear();

        _device = nullptr;
    }

    void RenderPass::AddAttachment(VkAttachmentDescription attachment)
    {
        if (_renderPass != VK_NULL_HANDLE)
            wowgm::exceptions::throw_with_trace(std::runtime_error("Unable to add attachments to a finalized render pass"));

        _attachmentDescriptions.push_back(attachment);
    }

    void RenderPass::_CreateDefaultSubpass()
    {
        Subpass* subpass = new Subpass(VK_PIPELINE_BIND_POINT_GRAPHICS);
        subpass->AddColor({ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
        _subpasses.push_back(subpass);
    }

    std::uint32_t RenderPass::Insert(Subpass* subpass)
    {
        if (_renderPass != VK_NULL_HANDLE)
            wowgm::exceptions::throw_with_trace(std::runtime_error("Unable to add a subpass to a finalized render pass"));

        std::uint32_t attachmentIndex = _subpasses.size();
        _subpasses.push_back(subpass);
        return attachmentIndex;
    }

    void RenderPass::SetDependency(VkSubpassDependency dependency)
    {
        if (_renderPass != VK_NULL_HANDLE)
            wowgm::exceptions::throw_with_trace(std::runtime_error("Unable to set subpass dependencies on a finalized render pass"));
        _subpassDependencies.push_back(dependency);
    }

    void RenderPass::Finalize()
    {
        if (_renderPass != VK_NULL_HANDLE)
            wowgm::exceptions::throw_with_trace(std::runtime_error("RenderPass::Finalize called twice!"));

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

        renderPassInfo.attachmentCount = _attachmentDescriptions.size();
        renderPassInfo.pAttachments = _attachmentDescriptions.data();

        renderPassInfo.subpassCount = _subpasses.size();

        auto subpassTransformer = [](Subpass* pass) -> VkSubpassDescription { pass->Finalize(); return *pass; };
        auto itr = boost::iterators::make_transform_iterator(_subpasses.begin(), subpassTransformer);
        auto end = boost::iterators::make_transform_iterator(_subpasses.end(), subpassTransformer);

        std::vector<VkSubpassDescription> descriptions(itr, end);
        renderPassInfo.pSubpasses = descriptions.data();

        renderPassInfo.dependencyCount = _subpassDependencies.size();
        renderPassInfo.pDependencies = _subpassDependencies.data();

        VkResult result = vkCreateRenderPass(*_device, &renderPassInfo, nullptr, &_renderPass);
        if (result != VK_SUCCESS)
            wowgm::exceptions::throw_with_trace(std::runtime_error("Unable to create a render pass!"));
    }
}

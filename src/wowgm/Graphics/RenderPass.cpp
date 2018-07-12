#include "RenderPass.hpp"
#include "Subpass.hpp"
#include "SwapChain.hpp"
#include "LogicalDevice.hpp"

#include <boost/compute/iterator/transform_iterator.hpp>

namespace wowgm::graphics
{
    RenderPass::RenderPass(SwapChain* swapChain) : _swapchain(swapChain)
    {
        _CreateDefaultSubpass();
    }

    RenderPass::~RenderPass()
    {
        if (_renderPass != VK_NULL_HANDLE)
            vkDestroyRenderPass(*_swapchain->GetLogicalDevice(), _renderPass, nullptr);

        for (std::uint32_t i = 0; i < _subpasses.size(); ++i)
            delete _subpasses[i];

        _subpasses.clear();

        _swapchain = nullptr;
    }

    void RenderPass::_CreateDefaultSubpass()
    {
        Subpass* subpass = new Subpass();
        subpass->AddColor({ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
        _subpasses.push_back(subpass);
    }

    void RenderPass::AddSubpass(Subpass* subpass)
    {
        _subpasses.push_back(subpass);
    }

    void RenderPass::AddSubpassDependency(VkSubpassDependency dependency)
    {
        _subpassDependencies.push_back(dependency);
    }

    void RenderPass::Finalize()
    {
        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

        renderPassInfo.attachmentCount = _attachmentDescriptions.size();
        renderPassInfo.pAttachments = _attachmentDescriptions.data();

        renderPassInfo.subpassCount = _subpasses.size();

        auto subpassTransformer = [](Subpass* pass) -> VkSubpassDescription { return *pass; };
        auto itr = boost::make_transform_iterator(_subpasses.begin(), subpassTransformer);
        auto end = boost::make_transform_iterator(_subpasses.end(), subpassTransformer);

        std::vector<VkSubpassDescription> descriptions(_subpasses.size());
        descriptions.insert(descriptions.begin(), itr, end);
        renderPassInfo.pSubpasses = descriptions.data();

        renderPassInfo.dependencyCount = _subpassDependencies.size();
        renderPassInfo.pDependencies = _subpassDependencies.data();

        LogicalDevice* logicalDevice = _swapchain->GetLogicalDevice();
        VkResult result = vkCreateRenderPass(*logicalDevice, &renderPassInfo, nullptr, &_renderPass);
        if (result != VK_SUCCESS)
            throw std::runtime_error("Unable to create a render pass!");

    }
}

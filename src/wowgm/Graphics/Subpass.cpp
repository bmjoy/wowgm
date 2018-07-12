#include "Subpass.hpp"

namespace wowgm::graphics
{

    Subpass::Subpass()
    {

    }

    Subpass::~Subpass()
    {

    }

    void Subpass::AddInput(VkAttachmentReference ref)
    {
        _inputAttachments.push_back(ref);
    }

    void Subpass::AddColor(VkAttachmentReference color)
    {
        _colorAttachments.push_back(color);
    }


    void Subpass::AddResolve(VkAttachmentReference resolve)
    {
        _resolveAttachments.push_back(resolve);
    }

    void Subpass::Finalize()
    {
        _subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        _subpassDescription.flags = 0;

        _subpassDescription.inputAttachmentCount = _inputAttachments.size();
        _subpassDescription.pInputAttachments = _inputAttachments.data();

        _subpassDescription.colorAttachmentCount = _colorAttachments.size();
        _subpassDescription.pColorAttachments = _colorAttachments.data();

        _subpassDescription.colorAttachmentCount = _colorAttachments.size();
        _subpassDescription.pColorAttachments = _colorAttachments.data();

        _subpassDescription.pResolveAttachments = _resolveAttachments.data();

        _subpassDescription.pDepthStencilAttachment = _depthStencilAttachment;

        _subpassDescription.preserveAttachmentCount = _preserveAttachments.size();
        _subpassDescription.pPreserveAttachments = _preserveAttachments.data();
    }
}

#include "Subpass.hpp"

namespace wowgm::graphics
{

    Subpass::Subpass(VkPipelineBindPoint bindPoint) : _bindPoint(bindPoint)
    {
        _depthStencilAttachment = nullptr;
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
        _subpassDescription = { };

        _subpassDescription.flags = 0;
        _subpassDescription.pipelineBindPoint = _bindPoint;
        _subpassDescription.inputAttachmentCount = _inputAttachments.size();
        _subpassDescription.pInputAttachments = _inputAttachments.data();
        _subpassDescription.colorAttachmentCount = _colorAttachments.size();
        _subpassDescription.pColorAttachments = _colorAttachments.data();
        _subpassDescription.pResolveAttachments = _resolveAttachments.data();
        _subpassDescription.pDepthStencilAttachment = _depthStencilAttachment;
        _subpassDescription.preserveAttachmentCount = _preserveAttachments.size();
        _subpassDescription.pPreserveAttachments = _preserveAttachments.data();
    }
}

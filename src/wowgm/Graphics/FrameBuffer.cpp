#include "FrameBuffer.hpp"
#include "RenderPass.hpp"
#include "ImageView.hpp"
#include "SwapChain.hpp"
#include "LogicalDevice.hpp"

#include <boost/iterator/transform_iterator.hpp>

namespace wowgm::graphics
{

    FrameBuffer::~FrameBuffer()
    {
        vkDestroyFramebuffer(*_swapChain->GetLogicalDevice(), _frameBuffer, nullptr);
        _frameBuffer = VK_NULL_HANDLE;
    }

    void FrameBuffer::Finalize()
    {
        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = *_renderPass;

        auto transform = [](ImageView* view) -> VkImageView { return *view; };
        auto itr = boost::iterators::make_transform_iterator(_attachments.begin(), transform);
        auto end = boost::iterators::make_transform_iterator(_attachments.end(), transform);
        std::vector<VkImageView> attachments(_attachments.size());
        attachments.insert(attachments.begin(), itr, end);

        framebufferInfo.attachmentCount = attachments.size();
        framebufferInfo.pAttachments = attachments.data();

        framebufferInfo.width = _swapChain->GetExtent().width;
        framebufferInfo.height = _swapChain->GetExtent().height;

        //! TODO: This needs to match the amount of alyers of all the imageviews we attached to.
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(*_swapChain->GetLogicalDevice(), &framebufferInfo, nullptr, &_frameBuffer) != VK_SUCCESS)
            throw std::runtime_error("failed to create framebuffer!");
    }
}
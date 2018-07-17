#include "FrameBuffer.hpp"
#include "RenderPass.hpp"
#include "ImageView.hpp"
#include "SwapChain.hpp"
#include "LogicalDevice.hpp"
#include "Assert.hpp"

#include <stdexcept>

#include <boost/iterator/transform_iterator.hpp>

namespace wowgm::graphics
{
    FrameBuffer::FrameBuffer(RenderPass* renderPass, SwapChain* swapChain) : _renderPass(renderPass), _swapChain(swapChain)
    {
        _frameBuffer = VK_NULL_HANDLE;
    }

    FrameBuffer::~FrameBuffer()
    {
        if (_frameBuffer != VK_NULL_HANDLE)
            vkDestroyFramebuffer(*_swapChain->GetLogicalDevice(), _frameBuffer, nullptr);
        _frameBuffer = VK_NULL_HANDLE;

        _renderPass = nullptr;
        _swapChain = nullptr;
    }

    void FrameBuffer::AttachImageView(ImageView* imageView)
    {
        _attachments.push_back(imageView);
    }

    void FrameBuffer::Finalize()
    {
        if (_frameBuffer != VK_NULL_HANDLE)
            wowgm::exceptions::throw_with_trace(std::runtime_error("FrameBuffer::Finalize called twice"));

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = *_renderPass;

        auto transform = [](ImageView* view) -> VkImageView { return *view; };
        auto itr = boost::iterators::make_transform_iterator(_attachments.begin(), transform);
        auto end = boost::iterators::make_transform_iterator(_attachments.end(), transform);
        std::vector<VkImageView> attachments(itr, end);

        framebufferInfo.attachmentCount = attachments.size();
        framebufferInfo.pAttachments = attachments.data();

        framebufferInfo.width = _swapChain->GetExtent().width;
        framebufferInfo.height = _swapChain->GetExtent().height;

        //! TODO: This needs to match the amount of alyers of all the imageviews we attached to.
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(*_swapChain->GetLogicalDevice(), &framebufferInfo, nullptr, &_frameBuffer) != VK_SUCCESS)
            wowgm::exceptions::throw_with_trace(std::runtime_error("failed to create framebuffer!"));
    }

    RenderPass* FrameBuffer::GetRenderPass()
    {
        return _renderPass;
    }
}
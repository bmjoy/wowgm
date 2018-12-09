#include "CommandBuffer.hpp"
#include "CommandPool.hpp"
#include "Device.hpp"
#include "Buffer.hpp"
#include "Image.hpp"
#include "Pipeline.hpp"
#include "Instance.hpp"
#include "Framebuffer.hpp"
#include "RenderPass.hpp"

#include <vector>
#include <boost/iterator/transform_iterator.hpp>
#include <mutex>
#include <thread>

namespace vez
{
    CommandBuffer::CommandBuffer(CommandPool* pool, VkCommandBuffer handle, VkCommandBufferLevel level) : _pool(pool), _handle(handle), _level(level)
    {

    }

    CommandBuffer::~CommandBuffer()
    {
        _pool->FreeCommandBuffers(1, &_handle);
        _handle = VK_NULL_HANDLE;
    }

    VkResult CommandBuffer::_InitializeLabelPointers()
    {
        static std::once_flag onceFlag;
        std::call_once(onceFlag, [&](VkInstance instance) -> void {
            _vkCmdEndDebugUtilsLabelEXT = PFN_vkCmdEndDebugUtilsLabelEXT(vkGetInstanceProcAddr(instance, "vkCmdEndDebugUtilsLabelEXT"));
            _vkCmdBeginDebugUtilsLabelEXT = PFN_vkCmdBeginDebugUtilsLabelEXT(vkGetInstanceProcAddr(instance, "vkCmdBeginDebugUtilsLabelEXT"));
            _vkCmdInsertDebugUtilsLabelEXT = PFN_vkCmdInsertDebugUtilsLabelEXT(vkGetInstanceProcAddr(instance, "vkCmdInsertDebugUtilsLabelEXT"));
        }, _pool->GetDevice()->GetInstance()->GetHandle());

        if (_vkCmdEndDebugUtilsLabelEXT == nullptr)
            return VK_ERROR_LAYER_NOT_PRESENT;

        return VK_SUCCESS;
    }

    VkResult CommandBuffer::BeginLabel(std::string_view label, const float(&color)[4])
    {
#if _DEBUG
        if (_vkCmdBeginDebugUtilsLabelEXT != nullptr)
        {
            VkDebugUtilsLabelEXT labelInfo{};
            labelInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
            memcpy(labelInfo.color, color, sizeof(color));
            labelInfo.pLabelName = label.data();

            _vkCmdBeginDebugUtilsLabelEXT(_handle, &labelInfo);
            return VK_SUCCESS;
        }
#endif

        return VK_ERROR_LAYER_NOT_PRESENT;
    }

    VkResult CommandBuffer::EndLabel()
    {
#if _DEBUG
        if (_vkCmdEndDebugUtilsLabelEXT != nullptr)
        {
            _vkCmdEndDebugUtilsLabelEXT(_handle);
            return VK_SUCCESS;
        }
#endif

        return VK_ERROR_LAYER_NOT_PRESENT;
    }

    VkResult CommandBuffer::InsertLabel(std::string_view label, const float(&color)[4])
    {
#if _DEBUG
        if (_vkCmdInsertDebugUtilsLabelEXT != nullptr)
        {
            VkDebugUtilsLabelEXT labelInfo{};
            labelInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
            memcpy(labelInfo.color, color, sizeof(color));
            labelInfo.pLabelName = label.data();

            _vkCmdInsertDebugUtilsLabelEXT(_handle, &labelInfo);
            return VK_SUCCESS;
        }
#endif

        return VK_ERROR_LAYER_NOT_PRESENT;
    }

    Instance* CommandBuffer::GetInstance() const
    {
        return _pool->GetDevice()->GetInstance();
    }

    Device* CommandBuffer::GetDevice() const
    {
        return _pool->GetDevice();
    }

    VkResult CommandBuffer::Begin(VkCommandBufferUsageFlags flags, const VkCommandBufferInheritanceInfo* pInheritanceInfo)
    {
        if (_handle == VK_NULL_HANDLE)
            return VK_INCOMPLETE;

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = flags;
        beginInfo.pInheritanceInfo = pInheritanceInfo;

        VkResult result = vkBeginCommandBuffer(_handle, &beginInfo);
        if (result == VK_SUCCESS)
            _isRecording = true;
        return result;
    }

    VkResult CommandBuffer::End()
    {
        if (_handle == VK_NULL_HANDLE || !_isRecording)
            return VK_INCOMPLETE;

        VkResult result = vkEndCommandBuffer(_handle);
        if (result == VK_SUCCESS)
            _isRecording = false;
        return result;
    }

    VkResult CommandBuffer::Reset(VkCommandBufferResetFlags resetFlags)
    {
        if (_handle == VK_NULL_HANDLE)
            return VK_INCOMPLETE;

#if _DEBUG
        _viewportSet = false;
        _scissorSet = false;
        _lineWidthSet = false;
        _depthBias = false;
        _blendConstants = false;
        _depthBounds = false;
        _stencilCompareMask = false;
        _stencilWriteMask = false;
        _stencilReference = false;
#endif

        return vkResetCommandBuffer(_handle, resetFlags);
    }

    void CommandBuffer::BeginRenderPass(const RenderPassBeginInfo* pBeginInfo, VkSubpassContents contents /* = VK_SUBPASS_CONTENTS_INLINE */)
    {
#if _DEBUG
        // https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/vkCmdBeginRenderPass.html
        if (_level != VK_COMMAND_BUFFER_LEVEL_PRIMARY)
            return;

        if (_currentRenderPass != nullptr)
            return;
#endif

        VkRenderPassBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        beginInfo.framebuffer = pBeginInfo->pFramebuffer->GetHandle();
        beginInfo.renderPass = pBeginInfo->pRenderPass->GetHandle();
        memcpy(&beginInfo.renderArea, &pBeginInfo->renderArea, sizeof(VkRect2D));

#if _DEBUG
        _subpassIndex = 0;
        _subpassContents = contents;
        _currentRenderPass = pBeginInfo->pRenderPass;
#endif

        vkCmdBeginRenderPass(_handle, &beginInfo, contents);
    }

    void CommandBuffer::NextSubpass(VkSubpassContents contents /* = VK_SUBPASS_CONTENTS_INLINE */)
    {
#if _DEBUG
        // https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/vkCmdNextSubpass.html
        if (_level != VK_COMMAND_BUFFER_LEVEL_PRIMARY)
            return;

        if (_currentRenderPass == nullptr)
            return;
#endif

        ++_subpassIndex;
        _subpassContents = contents;

        vkCmdNextSubpass(_handle, contents);
    }

    void CommandBuffer::EndRenderPass()
    {
#if _DEBUG
        // https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/vkCmdEndRenderPass.html
        if (_level != VK_COMMAND_BUFFER_LEVEL_PRIMARY)
            return;

        if (_currentRenderPass == nullptr)
            return;

        if (_subpassIndex != _currentRenderPass->GetSubpassCount() - 1)
            return;

        _currentRenderPass = nullptr;
#endif

        _ValidateDynamicStates();

        vkCmdEndRenderPass(_handle);
    }

    // https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/vkCmdBindPipeline.html
    void CommandBuffer::BindPipeline(Pipeline* pPipeline)
    {
        if (_boundPipeline != nullptr)
            _ValidateDynamicStates();

        _boundPipeline = pPipeline;

        vkCmdBindPipeline(_handle, pPipeline->GetBindPoint(), pPipeline->GetHandle());
    }

    void CommandBuffer::PushConstants(uint32_t offset, uint32_t size, const void* pValues)
    {
        vkCmdPushConstants(_handle, _boundPipeline->GetLayout(), _boundPipeline->GetPushConstantsRangeStages(offset, size), offset, size, pValues);
    }

    void CommandBuffer::BindBuffer(Buffer* pBuffer, VkDeviceSize offset, VkDeviceSize range, uint32_t set, uint32_t binding, uint32_t arrayElement)
    {
    }

    void CommandBuffer::BindBufferView(BufferView* pBufferView, uint32_t set, uint32_t binding, uint32_t arrayElement)
    {

    }

    void CommandBuffer::BindImageView(ImageView* pImageView, Sampler* sampler, uint32_t set, uint32_t binding, uint32_t arrayElement)
    {

    }

    void CommandBuffer::BindSampler(VkSampler sampler, uint32_t set, uint32_t binding, uint32_t arrayElement)
    {
    }

    void CommandBuffer::BindVertexBuffers(uint32_t firstBinding, uint32_t bindingCount, Buffer** ppBuffers, const VkDeviceSize* pOffsets)
    {
        std::vector<Buffer*> bufferObjects(&ppBuffers[firstBinding], &ppBuffers[firstBinding + bindingCount]);

        auto mutator = [](Buffer* buffer) -> VkBuffer { return buffer->GetHandle(); };
        auto begin = boost::make_transform_iterator(bufferObjects.begin(), mutator);
        auto end = boost::make_transform_iterator(bufferObjects.end(), mutator);
        std::vector<VkBuffer> buffers(begin, end);

        vkCmdBindVertexBuffers(_handle, firstBinding, bindingCount, buffers.data(), pOffsets);
    }

    void CommandBuffer::BindIndexBuffer(Buffer* pBuffer, VkDeviceSize offset, VkIndexType indexType)
    {
        vkCmdBindIndexBuffer(_handle, pBuffer->GetHandle(), offset, indexType);
    }

    // https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/vkCmdSetViewport.html
    void CommandBuffer::SetViewport(uint32_t firstViewport, uint32_t viewportCount, const VkViewport* pViewports)
    {
#if _DEBUG
        if (_boundPipeline != nullptr && !_boundPipeline->GetDynamicStates().viewport)
            return;

        _viewportSet = true;
#endif

        vkCmdSetViewport(_handle, firstViewport, viewportCount, pViewports);
    }

    void CommandBuffer::SetScissor(uint32_t firstScissor, uint32_t scissorCount, const VkRect2D* pScissors)
    {
#if _DEBUG
        if (_boundPipeline != nullptr && !_boundPipeline->GetDynamicStates().scissors)
            return;

        _scissorSet = true;
#endif

        vkCmdSetScissor(_handle, firstScissor, scissorCount, pScissors);
    }

    void CommandBuffer::SetLineWidth(float lineWidth)
    {
#if _DEBUG
        if (_boundPipeline != nullptr && !_boundPipeline->GetDynamicStates().lineWidth)
            return;

        _lineWidthSet = true;
#endif

        vkCmdSetLineWidth(_handle, lineWidth);
    }

    void CommandBuffer::SetDepthBias(float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor)
    {
#if _DEBUG
        if (_boundPipeline != nullptr && !_boundPipeline->GetDynamicStates().depthBias)
            return;

        _depthBias = true;
#endif

        vkCmdSetDepthBias(_handle, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
    }

    void CommandBuffer::SetBlendConstants(const float blendConstants[4])
    {
#if _DEBUG
        if (_boundPipeline != nullptr && !_boundPipeline->GetDynamicStates().blendConstants)
            return;

        _blendConstants = true;
#endif

        vkCmdSetBlendConstants(_handle, blendConstants);
    }

    void CommandBuffer::SetDepthBounds(float minDepthBounds, float maxDepthBounds)
    {
#if _DEBUG
        if (_boundPipeline != nullptr && !_boundPipeline->GetDynamicStates().depthBounds)
            return;

        _depthBounds = true;
#endif

        vkCmdSetDepthBounds(_handle, minDepthBounds, maxDepthBounds);
    }

    void CommandBuffer::SetStencilCompareMask(VkStencilFaceFlags faceMask, uint32_t compareMask)
    {
#if _DEBUG
        if (_boundPipeline != nullptr && !_boundPipeline->GetDynamicStates().stencilCompareMask)
            return;

        _stencilCompareMask = true;
#endif

        vkCmdSetStencilCompareMask(_handle, faceMask, compareMask);
    }

    void CommandBuffer::SetStencilWriteMask(VkStencilFaceFlags faceMask, uint32_t writeMask)
    {
#if _DEBUG
        if (_boundPipeline != nullptr && !_boundPipeline->GetDynamicStates().stencilWriteMask)
            return;

        _stencilWriteMask = true;
#endif

        vkCmdSetStencilWriteMask(_handle, faceMask, writeMask);
    }

    void CommandBuffer::SetStencilReference(VkStencilFaceFlags faceMask, uint32_t reference)
    {
#if _DEBUG
        if (_boundPipeline != nullptr && !_boundPipeline->GetDynamicStates().stencilReference)
            return;

        _stencilReference = true;
#endif

        vkCmdSetStencilReference(_handle, faceMask, reference);
    }

    void CommandBuffer::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
    {
        _ValidateDynamicStates();
        vkCmdDraw(_handle, vertexCount, instanceCount, firstVertex, firstInstance);
    }

    void CommandBuffer::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
    {
        _ValidateDynamicStates();
        vkCmdDrawIndexed(_handle, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }

    // https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/vkCmdDrawIndirect.html
    void CommandBuffer::DrawIndirect(Buffer* pBuffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride)
    {
        _ValidateDynamicStates();
        vkCmdDrawIndirect(_handle, pBuffer->GetHandle(), offset, drawCount, stride);
    }

    void CommandBuffer::DrawIndexedIndirect(Buffer* pBuffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride)
    {
        _ValidateDynamicStates();
        vkCmdDrawIndexedIndirect(_handle, pBuffer->GetHandle(), offset, drawCount, stride);
    }

    void CommandBuffer::Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
    {
        vkCmdDispatch(_handle, groupCountX, groupCountY, groupCountZ);
    }

    void CommandBuffer::DispatchIndirect(Buffer* pBuffer, VkDeviceSize offset)
    {
        vkCmdDispatchIndirect(_handle, pBuffer->GetHandle(), offset);
    }

    // https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/vkCmdCopyBuffer.html
    void CommandBuffer::CopyBuffer(Buffer* pSrcBuffer, Buffer* pDstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions)
    {
#if _DEBUG
        if (_currentRenderPass != nullptr)
            return;
#endif

        vkCmdCopyBuffer(_handle, pSrcBuffer->GetHandle(), pDstBuffer->GetHandle(), regionCount, pRegions);
    }

    // https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/vkCmdCopyImage.html
    void CommandBuffer::CopyImage(Image* pSrcImage, Image* pDstImage, uint32_t regionCount, const ImageCopy* pRegions)
    {
#if _DEBUG
        if (_currentRenderPass != nullptr)
            return;
#endif

        std::vector<VkImageCopy> vkImageCopies(regionCount);

        vkCmdCopyImage(_handle, pSrcImage->GetHandle(), pSrcImage->GetLayout(), pDstImage->GetHandle(), pDstImage->GetLayout(), regionCount, vkImageCopies.data());
    }

    // https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/vkCmdBlitImage.html
    void CommandBuffer::BlitImage(Image* pSrcImage, Image* pDstImage, uint32_t regionCount, const ImageBlit* pRegions, VkFilter filter)
    {
#if _DEBUG
        if (_currentRenderPass != nullptr)
            return;
#endif

        std::vector<VkImageBlit> vkBlits(regionCount);

        vkCmdBlitImage(_handle, pSrcImage->GetHandle(), pSrcImage->GetLayout(), pDstImage->GetHandle(), pDstImage->GetLayout(), regionCount, vkBlits.data(), filter);
    }

    // https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/vkCmdCopyBufferToImage.html
    void CommandBuffer::CopyBufferToImage(Buffer* pSrcBuffer, Image* pDstImage, uint32_t regionCount, const BufferImageCopy* pRegions)
    {
#if _DEBUG
        if (_currentRenderPass != nullptr)
            return;
#endif

        std::vector<VkBufferImageCopy> bufferRegions(regionCount);
        for (std::uint32_t i = 0; i < regionCount; ++i)
        {
            VkBufferImageCopy& imageCopy = bufferRegions[i];
            imageCopy.imageSubresource.baseArrayLayer = pRegions[i].imageSubresource.baseArrayLayer;
            imageCopy.imageSubresource.layerCount = pRegions[i].imageSubresource.layerCount;
            imageCopy.imageSubresource.mipLevel = pRegions[i].imageSubresource.mipLevel;
            imageCopy.imageSubresource.aspectMask = pDstImage->GetImageAspectFlags();
            memcpy(&imageCopy.imageExtent, &pRegions[i].imageExtent, sizeof(VkExtent3D));
            memcpy(&imageCopy.imageOffset, &pRegions[i].imageOffset, sizeof(VkOffset3D));
            imageCopy.bufferOffset = pRegions[i].bufferOffset;
            imageCopy.bufferImageHeight = pRegions[i].bufferImageHeight;
            imageCopy.bufferRowLength = pRegions[i].bufferRowLength;
        }

        vkCmdCopyBufferToImage(_handle, pSrcBuffer->GetHandle(), pDstImage->GetHandle(), pDstImage->GetLayout(), regionCount, bufferRegions.data());
    }

    void CommandBuffer::CopyImageToBuffer(Image* pSrcImage, Buffer* pDstBuffer, uint32_t regionCount, const BufferImageCopy* pRegions)
    {
#if _DEBUG
        if (_currentRenderPass != nullptr)
            return;
#endif

        std::vector<VkBufferImageCopy> bufferRegions(regionCount);
        for (std::uint32_t i = 0; i < regionCount; ++i)
        {
            VkBufferImageCopy& imageCopy = bufferRegions[i];
            imageCopy.imageSubresource.baseArrayLayer = pRegions[i].imageSubresource.baseArrayLayer;
            imageCopy.imageSubresource.layerCount = pRegions[i].imageSubresource.layerCount;
            imageCopy.imageSubresource.mipLevel = pRegions[i].imageSubresource.mipLevel;
            imageCopy.imageSubresource.aspectMask = pSrcImage->GetImageAspectFlags();
            memcpy(&imageCopy.imageExtent, &pRegions[i].imageExtent, sizeof(VkExtent3D));
            memcpy(&imageCopy.imageOffset, &pRegions[i].imageOffset, sizeof(VkOffset3D));
            imageCopy.bufferOffset = pRegions[i].bufferOffset;
            imageCopy.bufferImageHeight = pRegions[i].bufferImageHeight;
            imageCopy.bufferRowLength = pRegions[i].bufferRowLength;
        }

        vkCmdCopyImageToBuffer(_handle, pSrcImage->GetHandle(), pSrcImage->GetLayout(), pDstBuffer->GetHandle(), regionCount, bufferRegions.data());
    }

    void CommandBuffer::UpdateBuffer(Buffer* pDstBuffer, VkDeviceSize dstOffset, VkDeviceSize dataSize, const void* pData)
    {
        vkCmdUpdateBuffer(_handle, pDstBuffer->GetHandle(), dstOffset, dataSize, pData);
    }

    void CommandBuffer::FillBuffer(Buffer* pDstBuffer, VkDeviceSize dstOffset, VkDeviceSize size, uint32_t data)
    {
        vkCmdFillBuffer(_handle, pDstBuffer->GetHandle(), dstOffset, size, data);
    }

    void CommandBuffer::ClearColorImage(Image* pImage, const VkClearColorValue* pColor, uint32_t rangeCount, const ImageSubresourceRange* pRanges)
    {
        std::vector<VkImageSubresourceRange> subresourceRanges(rangeCount);
        for (std::uint32_t i = 0; i < rangeCount; ++i)
        {
            VkImageSubresourceRange& vkRange = subresourceRanges[i];
            ImageSubresourceRange const& appRange = pRanges[i];

            vkRange.baseMipLevel = appRange.baseMipLevel;
            vkRange.levelCount = appRange.levelCount;
            vkRange.baseArrayLayer = appRange.baseArrayLayer;
            vkRange.layerCount = appRange.layerCount;

            vkRange.aspectMask = pImage->GetImageAspectFlags();
        }

        vkCmdClearColorImage(_handle, pImage->GetHandle(), pImage->GetLayout(), pColor, rangeCount, subresourceRanges.data());
    }

    void CommandBuffer::ClearDepthStencilImage(Image* pImage, const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount, const ImageSubresourceRange* pRanges)
    {
        std::vector<VkImageSubresourceRange> subresourceRanges(rangeCount);
        for (std::uint32_t i = 0; i < rangeCount; ++i)
        {
            VkImageSubresourceRange& vkRange = subresourceRanges[i];
            ImageSubresourceRange const& appRange = pRanges[i];

            vkRange.baseMipLevel = appRange.baseMipLevel;
            vkRange.levelCount = appRange.levelCount;
            vkRange.baseArrayLayer = appRange.baseArrayLayer;
            vkRange.layerCount = appRange.layerCount;

            vkRange.aspectMask = pImage->GetImageAspectFlags();
        }

        vkCmdClearDepthStencilImage(_handle, pImage->GetHandle(), pImage->GetLayout(), pDepthStencil, rangeCount, subresourceRanges.data());
    }

    void CommandBuffer::ClearAttachments(uint32_t attachmentCount, const ClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects)
    {
        std::vector<VkClearAttachment> attachments(attachmentCount);
        for (std::uint32_t i = 0; i < attachmentCount; ++i)
        {
            ClearAttachment const& attachment = pAttachments[i];
            VkClearAttachment& vkAttachment = attachments[i];

            vkAttachment.colorAttachment = attachment.colorAttachment;
            vkAttachment.clearValue = attachment.clearValue;
            // vkAttachment.aspectMask = getCurrentSubpass()->;
        }

        vkCmdClearAttachments(_handle, attachmentCount, attachments.data(), rectCount, pRects);
    }

    void CommandBuffer::ResolveImage(Image* pSrcImage, Image* pDstImage, uint32_t regionCount, const ImageResolve* pRegions)
    {
        VkImageResolve imageResolve;

        vkCmdResolveImage(_handle, pSrcImage->GetHandle(), pSrcImage->GetLayout(), pDstImage->GetHandle(), pDstImage->GetLayout(), regionCount, &imageResolve);
    }

    void CommandBuffer::SetEvent(VkEvent event, VkPipelineStageFlags stageMask)
    {
        vkCmdSetEvent(_handle, event, stageMask);
    }

    void CommandBuffer::ResetEvent(VkEvent event, VkPipelineStageFlags stageMask)
    {
        vkCmdResetEvent(_handle, event, stageMask);
    }

    void CommandBuffer::PipelineBarrier(
        VkPipelineStageFlags         srcStageMask,
        VkPipelineStageFlags         dstStageMask,
        VkDependencyFlags            dependencyFlags,
        uint32_t                    memoryBarrierCount,
        const VkMemoryBarrier*       pMemoryBarriers,
        uint32_t                     bufferMemoryBarrierCount,
        const VkBufferMemoryBarrier* pBufferMemoryBarriers,
        uint32_t                     imageMemoryBarrierCount,
        const VkImageMemoryBarrier*  pImageMemoryBarriers)
    {
        vkCmdPipelineBarrier(_handle, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    }

    void CommandBuffer::_ValidateDynamicStates()
    {
#if _DEBUG
        if (_boundPipeline == nullptr)
            return;

        auto dynamicStates = _boundPipeline->GetDynamicStates();
        if (dynamicStates.viewport && !_viewportSet)
            throw std::logic_error("Ended a command buffer on a pipeline with VK_DYNAMIC_STATE_VIEWPORT and no SetViewport call.");

        if (dynamicStates.scissors && !_scissorSet)
            throw std::logic_error("Ended a command buffer on a pipeline with VK_DYNAMIC_STATE_SCISSOR and no SetScissor call.");

        if (dynamicStates.lineWidth && !_lineWidthSet)
            throw std::logic_error("Ended a command buffer on a pipeline with VK_DYNAMIC_STATE_LINE_WIDTH and no SetLineWidth call.");

        if (dynamicStates.depthBias && !_depthBias)
            throw std::logic_error("Ended a command buffer on a pipeline with VK_DYNAMIC_STATE_DEPTH_BIAS and no SetDepthBias call.");

        if (dynamicStates.blendConstants && !_blendConstants)
            throw std::logic_error("Ended a command buffer on a pipeline with VK_DYNAMIC_STATE_BLEND_CONSTANTS and no SetBlendConstants call.");

        if (dynamicStates.depthBounds && !_depthBounds)
            throw std::logic_error("Ended a command buffer on a pipeline with VK_DYNAMIC_STATE_DEPTH_BOUNDS and no SetDepthBounds call.");

        if (dynamicStates.stencilCompareMask && !_stencilCompareMask)
            throw std::logic_error("Ended a command buffer on a pipeline with VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK and no SetStencilCompareMask call.");

        if (dynamicStates.stencilWriteMask && !_stencilWriteMask)
            throw std::logic_error("Ended a command buffer on a pipeline with VK_DYNAMIC_STATE_STENCIL_WRITE_MASK and no SetStencilWriteMask call.");

        if (dynamicStates.stencilReference && !_stencilReference)
            throw std::logic_error("Ended a command buffer on a pipeline with VK_DYNAMIC_STATE_STENCIL_REFERENCE and no SetStencilReference call.");

        //! TODO The rest are extensions and need to be checked on a hardware-basis - future.

        _viewportSet = false;
        _scissorSet = false;
        _lineWidthSet = false;
        _depthBias = false;
        _blendConstants = false;
        _depthBounds = false;
        _stencilCompareMask = false;
        _stencilWriteMask = false;
        _stencilReference = false;
#endif
    }

    /// TODO: Premature optimizations?
    // void CommandBuffer::_EnqueueImageLayoutTransition(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, const VkImageMemoryBarrier* imageBarrier)
    // {
    //     VkImageMemoryBarrier& barrier = _pipelineBarriers[std::make_tuple(srcStageMask, dstStageMask, dependencyFlags)].imageMemoryBarriers.emplace_back(VkImageMemoryBarrier{});
    //     memcpy(&barrier, imageBarrier, sizeof(VkImageMemoryBarrier));
    // }
    //
    //
    // void CommandBuffer::_FlushBarriers(Buffer* buffer)
    // {
    //     for (auto&& kv : _pipelineBarriers)
    //     {
    //         std::vector<VkBufferMemoryBarrier> barriersToDelete;
    //
    //         for (auto&& imageBarriers : kv.second.bufferMemoryBarriers)
    //             if (imageBarriers.buffer == buffer->GetHandle())
    //                 barriersToDelete.push_back(imageBarriers);
    //
    //         vkCmdPipelineBarrier(_handle, std::get<0>(kv.first), std::get<1>(kv.first), std::get<2>(kv.first), 0, nullptr, barriersToDelete.size(), barriersToDelete.data(), 0, nullptr);
    //
    //         std::uint32_t i = 0;
    //         for (auto it = barriersToDelete.begin(); it != barriersToDelete.end(); ++it)
    //             kv.second.bufferMemoryBarriers.erase(it);
    //     }
    // }
    //
    // void CommandBuffer::_FlushMemoryBarriers()
    // {
    //     for (auto&& kv : _pipelineBarriers)
    //     {
    //         vkCmdPipelineBarrier(_handle, std::get<0>(kv.first), std::get<1>(kv.first), std::get<2>(kv.first), kv.second.memoryBarriers.size(), kv.second.memoryBarriers.data(), 0, nullptr, 0, nullptr);
    //         kv.second.memoryBarriers.clear();
    //     }
    // }
    //
    // void CommandBuffer::_FlushBarriers(Image* image)
    // {
    //     for (auto&& kv : _pipelineBarriers)
    //     {
    //         std::vector<VkImageMemoryBarrier> barriersToDelete;
    //
    //         for (auto&& imageBarriers : kv.second.imageMemoryBarriers)
    //             if (imageBarriers.image == image->GetHandle())
    //                 barriersToDelete.push_back(imageBarriers);
    //
    //         vkCmdPipelineBarrier(_handle, std::get<0>(kv.first), std::get<1>(kv.first), std::get<2>(kv.first), 0, nullptr, 0, nullptr, barriersToDelete.size(), barriersToDelete.data());
    //
    //         std::uint32_t i = 0;
    //         for (auto it = barriersToDelete.begin(); it != barriersToDelete.end(); ++it)
    //             kv.second.imageMemoryBarriers.erase(it);
    //     }
    // }
}

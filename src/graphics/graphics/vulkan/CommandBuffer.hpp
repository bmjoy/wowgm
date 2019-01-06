#pragma once

#include <graphics/vulkan/VK.hpp>
#include <graphics/vulkan/NamedObject.hpp>

#include <unordered_map>
#include <vector>
#include <string_view>
#include <type_traits>

namespace gfx::vk
{
    class BufferView; // TODO
    class ImageCopy; // TODO
    class ImageBlit; // TODO
    class ImageResolve; // TODO

    class CommandBuffer final : public NamedObject<CommandBuffer>
    {
    private:
        friend class CommandPool;

        void MarkAsDeleted() {
            _handle = VK_NULL_HANDLE;
        }

    public:
        CommandBuffer(CommandPool* pool, VkCommandBuffer handle, VkCommandBufferLevel level);
        ~CommandBuffer();

        CommandPool* GetPool() { return _pool; }

        Instance* GetInstance() const;
        Device* GetDevice() const;

        VkCommandBuffer GetHandle() const { return _handle; }

        VkCommandBufferLevel GetLevel() const { return _level; }

        VkResult BeginLabel(std::string_view label, const float(&color)[4]);
        VkResult EndLabel();
        VkResult InsertLabel(std::string_view label, const float(&color)[4]);

        VkResult Begin(VkCommandBufferUsageFlags flags, const VkCommandBufferInheritanceInfo* pInheritanceInfo = nullptr);
        VkResult End();
        VkResult Reset(VkCommandBufferResetFlags resetFlags = VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

        template <typename T>
        inline void PushConstants(uint32_t offset, T value) {
            static_assert(std::is_standard_layout<T>::value, "must be standard layout");

            PushConstants(offset, sizeof(T), reinterpret_cast<void*>(value));
        }

        template <typename T, size_t N>
        inline void PushConstants(uint32_t offset, T (&value)[N]) {
            static_assert(std::is_standard_layout<T>::value, "must be standard layout");

            PushConstants(offset, sizeof(T), reinterpret_cast<void*>(value));
        }

        void BeginRenderPass(const RenderPassBeginInfo* pBeginInfo,
            VkSubpassContents               contents = VK_SUBPASS_CONTENTS_INLINE);
        void NextSubpass(
            VkSubpassContents               contents = VK_SUBPASS_CONTENTS_INLINE);
        void EndRenderPass();
        void BindPipeline(
            Pipeline*                       pPipeline);
        void PushConstants(
            uint32_t                        offset,
            uint32_t                        size,
            const void*                     pValues);
        void BindBuffer(
            Buffer*                         pBuffer,
            VkDeviceSize                    offset,
            VkDeviceSize                    range,
            uint32_t                        set,
            uint32_t                        binding,
            uint32_t                        arrayElement);
        void BindBufferView(
            BufferView*                     pBufferView,
            uint32_t                        set,
            uint32_t                        binding,
            uint32_t                        arrayElement);
        void BindImageView(
            ImageView*                      pImageView,
            Sampler*                        sampler,
            uint32_t                        set,
            uint32_t                        binding,
            uint32_t                        arrayElement);
        void BindSampler(
            VkSampler                       sampler,
            uint32_t                        set,
            uint32_t                        binding,
            uint32_t                        arrayElement);
        void BindVertexBuffers(
            uint32_t                        firstBinding,
            uint32_t                        bindingCount,
            Buffer**                        ppBuffers,
            const VkDeviceSize*             pOffsets);
        void BindIndexBuffer(
            Buffer*                         pBuffer,
            VkDeviceSize                    offset,
            VkIndexType                     indexType);
        void SetViewport(
            uint32_t                        firstViewport,
            uint32_t                        viewportCount,
            const VkViewport*               pViewports);
        void SetScissor(
            uint32_t                        firstScissor,
            uint32_t                        scissorCount,
            const VkRect2D*                 pScissors);
        void SetLineWidth(
            float                           lineWidth);
        void SetDepthBias(
            float                           depthBiasConstantFactor,
            float                           depthBiasClamp,
            float                           depthBiasSlopeFactor);
        void SetBlendConstants(
            const float                     blendConstants[4]);
        void SetDepthBounds(
            float                           minDepthBounds,
            float                           maxDepthBounds);
        void SetStencilCompareMask(
            VkStencilFaceFlags              faceMask,
            uint32_t                        compareMask);
        void SetStencilWriteMask(
            VkStencilFaceFlags              faceMask,
            uint32_t                        writeMask);
        void SetStencilReference(
            VkStencilFaceFlags              faceMask,
            uint32_t                        reference);
        void Draw(
            uint32_t                        vertexCount,
            uint32_t                        instanceCount = 1,
            uint32_t                        firstVertex = 0,
            uint32_t                        firstInstance = 0);
        void DrawIndexed(
            uint32_t                        indexCount,
            uint32_t                        instanceCount,
            uint32_t                        firstIndex,
            int32_t                         vertexOffset,
            uint32_t                        firstInstance);
        void DrawIndirect(
            Buffer*                         pBuffer,
            VkDeviceSize                    offset,
            uint32_t                        drawCount,
            uint32_t                        stride);
        void DrawIndexedIndirect(
            Buffer*                         pBuffer,
            VkDeviceSize                    offset,
            uint32_t                        drawCount,
            uint32_t                        stride);
        void Dispatch(
            uint32_t                        groupCountX,
            uint32_t                        groupCountY,
            uint32_t                        groupCountZ);
        void DispatchIndirect(
            Buffer*                         pBuffer,
            VkDeviceSize                    offset);
        void CopyBuffer(
            Buffer*                         pSrcBuffer,
            Buffer*                         pDstBuffer,
            uint32_t                        regionCount,
            const VkBufferCopy*             pRegions);
        void CopyImage(
            Image*                          pSrcImage,
            Image*                          pDstImage,
            uint32_t                        regionCount,
            const ImageCopy*                pRegions);
        void BlitImage(
            Image*                          pSrcImage,
            Image*                          pDstImage,
            uint32_t                        regionCount,
            const ImageBlit*                pRegions,
            VkFilter                        filter);
        void CopyBufferToImage(
            Buffer*                         pSrcBuffer,
            Image*                          pDstImage,
            VkImageLayout                   dstLayout,
            uint32_t                        regionCount,
            const BufferImageCopy*          pRegions);
        void CopyImageToBuffer(
            Image*                          pSrcImage,
            Buffer*                         pDstBuffer,
            uint32_t                        regionCount,
            const BufferImageCopy*          pRegions);
        void UpdateBuffer(
            Buffer*                         pDstBuffer,
            VkDeviceSize                    dstOffset,
            VkDeviceSize                    dataSize,
            const void*                     pData);
        void FillBuffer(
            Buffer*                         pDstBuffer,
            VkDeviceSize                    dstOffset,
            VkDeviceSize                    size,
            uint32_t                        data);
        void ClearColorImage(
            Image*                          pImage,
            const VkClearColorValue*        pColor,
            uint32_t                        rangeCount,
            const ImageSubresourceRange* pRanges);
        void ClearDepthStencilImage(
            Image*                          pImage,
            const VkClearDepthStencilValue* pDepthStencil,
            uint32_t                        rangeCount,
            const ImageSubresourceRange* pRanges);
        void ClearAttachments(
            uint32_t                        attachmentCount,
            const ClearAttachment*          pAttachments,
            uint32_t                        rectCount,
            const VkClearRect*              pRects);
        void ResolveImage(
            Image*                          pSrcImage,
            Image*                          pDstImage,
            uint32_t                        regionCount,
            const ImageResolve*             pRegions);
        void SetEvent(
            VkEvent                         event,
            VkPipelineStageFlags            stageMask);
        void ResetEvent(
            VkEvent                         event,
            VkPipelineStageFlags            stageMask);

        void PipelineBarrier(
            ImageMemoryBarrier const* memoryBarrier,
            VkPipelineStageFlags           srcStageMask,
            VkPipelineStageFlags           dstStageMask,
            VkDependencyFlags              dependencyFlags);

        // void _EnqueueImageLayoutTransition(
        //     VkPipelineStageFlags        srcStageMask,
        //     VkPipelineStageFlags        dstStageMask,
        //     VkDependencyFlags           dependencyFlags,
        //     const VkImageMemoryBarrier* imageBarrier);
        //
        // void _FlushBarriers(Image* image);
        // void _FlushBarriers(Buffer* buffer);
        // void _FlushMemoryBarriers();

    private:
        void _ValidateDynamicStates();
        VkResult _InitializeLabelPointers();

    private:
        friend class CommandPool;

        CommandPool* _pool = nullptr;
        VkCommandBuffer _handle = VK_NULL_HANDLE;
        Pipeline* _boundPipeline = nullptr;
        bool _isRecording = false;
        bool _submittedToQueue = false;

        /// NYI. The idea is to bucket matching pipeline barriers into a single command
        /// and only commit actual barriers to the stream when the corresponding objects are used.
        /// Say you call vkCmdPipelineBarrier for an image, then do some vertex buffer stuff, then barrier the image again
        /// We won't actually barrier stuff until when the driver actually uses them.
        struct PipelineBarrierBucket
        {
            std::vector<VkMemoryBarrier> memoryBarriers;
            std::vector<VkBufferMemoryBarrier> bufferMemoryBarriers;
            std::vector<VkImageMemoryBarrier> imageMemoryBarriers;
        };
        std::unordered_map<std::tuple<VkPipelineStageFlags, VkPipelineStageFlags, VkDependencyFlags>, PipelineBarrierBucket> _pipelineBarriers;

#if _DEBUG
        // VK_DYNAMIC_STATE_VIEWPORT specifies that the pViewports state in VkPipelineViewportStateCreateInfo will be ignored
        // and must be set dynamically with vkCmdSetViewport before any draw commands. The number of viewports used by a pipeline
        // is still specified by the viewportCount member of VkPipelineViewportStateCreateInfo.
        bool _viewportSet = false;

        // VK_DYNAMIC_STATE_SCISSOR specifies that the pScissors state in VkPipelineViewportStateCreateInfo will be ignored
        // and must be set dynamically with vkCmdSetScissor before any draw commands.The number of scissor rectangles used by a
        // pipeline is still specified by the scissorCount member of VkPipelineViewportStateCreateInfo.
        bool _scissorSet = false;

        // VK_DYNAMIC_STATE_LINE_WIDTH specifies that the lineWidth state in VkPipelineRasterizationStateCreateInfo will be
        // ignored and must be set dynamically with vkCmdSetLineWidth before any draw commands that generate line primitives for
        // the rasterizer.
        bool _lineWidthSet = false;

        // VK_DYNAMIC_STATE_DEPTH_BIAS specifies that the depthBiasConstantFactor, depthBiasClamp and depthBiasSlopeFactor
        // states in VkPipelineRasterizationStateCreateInfo will be ignored and must be set dynamically with vkCmdSetDepthBias
        // before any draws are performed with depthBiasEnable in VkPipelineRasterizationStateCreateInfo set to VK_TRUE.
        bool _depthBias = false;

        // VK_DYNAMIC_STATE_BLEND_CONSTANTS specifies that the blendConstants state in VkPipelineColorBlendStateCreateInfo will
        // be ignored and must be set dynamically with vkCmdSetBlendConstants before any draws are performed with a pipeline state
        // with VkPipelineColorBlendAttachmentState member blendEnable set to VK_TRUE and any of the blend functions using a
        // constant blend color.
        bool _blendConstants = false;

        // VK_DYNAMIC_STATE_DEPTH_BOUNDS specifies that the minDepthBounds and maxDepthBounds states of
        // VkPipelineDepthStencilStateCreateInfo will be ignored and must be set dynamically with vkCmdSetDepthBounds before any
        // draws are performed with a pipeline state with VkPipelineDepthStencilStateCreateInfo member depthBoundsTestEnable set
        // to VK_TRUE.
        bool _depthBounds = false;

        // VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK specifies that the compareMask state in VkPipelineDepthStencilStateCreateInfo
        // for both front and back will be ignored and must be set dynamically with vkCmdSetStencilCompareMask before any draws
        // are performed with a pipeline state with VkPipelineDepthStencilStateCreateInfo member stencilTestEnable set to VK_TRUE.
        bool _stencilCompareMask = false;

        // VK_DYNAMIC_STATE_STENCIL_WRITE_MASK specifies that the writeMask state in VkPipelineDepthStencilStateCreateInfo for
        // both front and back will be ignored and must be set dynamically with vkCmdSetStencilWriteMask before any draws are
        // performed with a pipeline state with VkPipelineDepthStencilStateCreateInfo member stencilTestEnable set to VK_TRUE.
        bool _stencilWriteMask = false;

        // VK_DYNAMIC_STATE_STENCIL_REFERENCE specifies that the reference state in VkPipelineDepthStencilStateCreateInfo for
        // both front and back will be ignored and must be set dynamically with vkCmdSetStencilReference before any draws are
        // performed with a pipeline state with VkPipelineDepthStencilStateCreateInfo member stencilTestEnable set to VK_TRUE.
        bool _stencilReference = false;

#endif
        PFN_vkCmdEndDebugUtilsLabelEXT    _vkCmdEndDebugUtilsLabelEXT = nullptr;
        PFN_vkCmdBeginDebugUtilsLabelEXT  _vkCmdBeginDebugUtilsLabelEXT = nullptr;
        PFN_vkCmdInsertDebugUtilsLabelEXT _vkCmdInsertDebugUtilsLabelEXT = nullptr;

        VkCommandBufferLevel _level;

#if _DEBUG
        RenderPass* _currentRenderPass = nullptr;
        uint32_t _subpassIndex;
        VkSubpassContents _subpassContents = VK_SUBPASS_CONTENTS_INLINE;
#endif

    };
}
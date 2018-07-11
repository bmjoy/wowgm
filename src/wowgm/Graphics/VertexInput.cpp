#include "VertexInput.hpp"
#include "SwapChain.hpp"

namespace wowgm::graphics
{
    VertexInput::VertexInput(SwapChain* swapChain)
    {
        _inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        _inputAssembly.pNext = nullptr;
        _inputAssembly.flags = 0;

        _tessellationState.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
        _tessellationState.pNext = nullptr;
        _tessellationState.flags = 0;

        _viewportCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        _viewportCreateInfo.pNext = nullptr;
        _viewportCreateInfo.flags = 0;

        // For now, we only have 1 viewport. not sure why we would need more??
        // If we ever do, we require a GPU feature (multiViewport on a logical device).
        _viewportCreateInfo.viewportCount = 1;
        _viewportCreateInfo.scissorCount = 1;
        _viewportCreateInfo.pViewports = &_viewport;
        _viewportCreateInfo.pScissors = &_scissors;

        _viewport.x = 0.0f;
        _viewport.y = 0.0f;
        _viewport.maxDepth = 1.0f;
        _viewport.minDepth = 0.0f;
        _viewport.width = static_cast<float>(swapChain->GetExtent().width);
        _viewport.height = static_cast<float>(swapChain->GetExtent().height);

        _scissors.offset = { 0 , 0 };
        _scissors.extent = swapChain->GetExtent();

        _rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        _rasterizationState.pNext = nullptr;
        _rasterizationState.flags = 0;
        _rasterizationState.depthClampEnable = VK_FALSE;
        // If rasterizerDiscardEnable is set to VK_TRUE, then geometry never passes through
        // the rasterizer stage. This basically disables any output to the framebuffer.
        _rasterizationState.rasterizerDiscardEnable = VK_FALSE;
        _rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
        // Any value larger than 1 requires `wideLine` feature on the logical device.
        _rasterizationState.lineWidth = 1.0f;
        _rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
        _rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        // This is usually used for shadow mapping. We don't use that by default.
        _rasterizationState.depthBiasEnable = VK_FALSE;
        _rasterizationState.depthBiasConstantFactor = 0.0f; // Optional
        _rasterizationState.depthBiasClamp = 0.0f; // Optional
        _rasterizationState.depthBiasSlopeFactor = 0.0f; // Optional

        _multisamplingState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        _multisamplingState.pNext = nullptr;
        _multisamplingState.flags = 0;
        //! TOOD: https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkSampleCountFlagBits.html
        // The number of samples generated per pixel.
        _multisamplingState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        _multisamplingState.minSampleShading = 1.0f; // Optional
        _multisamplingState.pSampleMask = nullptr; // Optional
        _multisamplingState.alphaToCoverageEnable = VK_FALSE; // Optional
        _multisamplingState.alphaToOneEnable = VK_FALSE; // Optional

        // This requires a depth/stencil test resource to be dynamically added, so, uh, wait a bit.
        // For now, disable depth testing
        // _depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        // _depthStencilState.pNext = nullptr;
        // _depthStencilState.flags = 0;
        // _depthStencilState.depthTestEnable = VK_TRUE;
        // _depthStencilState.stencilTestEnable = VK_FALSE;
    }

    void VertexInput::AddBinding(VkVertexInputBindingDescription binding)
    {
        _vertexBindingDescriptions.push_back(binding);
    }

    void VertexInput::AddAttribute(VkVertexInputAttributeDescription attrDescription)
    {
        _vertexAttributeDescriptions.push_back(attrDescription);
    }

    void VertexInput::Finalize()
    {
        VkPipelineVertexInputStateCreateInfo createInfo = { };
        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;

        createInfo.vertexBindingDescriptionCount = _vertexBindingDescriptions.size();
        createInfo.pVertexBindingDescriptions = _vertexBindingDescriptions.data();

        createInfo.vertexAttributeDescriptionCount = _vertexAttributeDescriptions.size();
        createInfo.pVertexAttributeDescriptions = _vertexAttributeDescriptions.data();
    }

    void VertexInput::SetPrimitiveType(VkPrimitiveTopology topology, bool restartEnable)
    {
        _inputAssembly.topology = topology;
        _inputAssembly.primitiveRestartEnable = restartEnable ? VK_TRUE : VK_FALSE;
    }

    void VertexInput::SetTessellationControlPoints(std::uint32_t controlPoints)
    {
        _tessellationState.patchControlPoints = controlPoints;
    }

    void VertexInput::SetViewport(std::uint32_t width, std::uint32_t height)
    {
        _viewport.width = static_cast<float>(width);
        _viewport.height = static_cast<float>(height);
    }

    void VertexInput::SetViewportDepth(float minDepth, float maxDepth)
    {
        _viewport.maxDepth = maxDepth;
        _viewport.minDepth = minDepth;
    }

    void VertexInput::SetScissors(std::uint32_t width, std::uint32_t height)
    {
        _scissors.extent.width = width;
        _scissors.extent.height = height;
    }

    void VertexInput::SetWireframe(bool wireframe)
    {
        _rasterizationState.polygonMode = wireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
    }

    void VertexInput::SetFragmentClampState(bool clampFragments)
    {
        _rasterizationState.depthClampEnable = clampFragments ? VK_TRUE : VK_FALSE;
    }

    void VertexInput::SetCulling(VkCullModeFlagBits cullMode)
    {
        _rasterizationState.cullMode = cullMode;
    }

    void VertexInput::SetFrontFaceOrientation(bool clockwise)
    {
        _rasterizationState.frontFace = clockwise ? VK_FRONT_FACE_CLOCKWISE : VK_FRONT_FACE_COUNTER_CLOCKWISE;
    }

    void VertexInput::SetDepthTest(bool enable)
    {
        _depthStencilState.depthTestEnable = enable ? VK_TRUE : VK_FALSE;
    }

    void VertexInput::SetDepthTest(VkCompareOp op)
    {
        SetDepthTest(true);
        _depthStencilState.depthCompareOp = op;
    }

    void VertexInput::SetStencilTest(bool enable)
    {
        _depthStencilState.stencilTestEnable = enable ? VK_TRUE : VK_FALSE;
    }
}

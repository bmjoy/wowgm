#pragma once

#include <vulkan/vulkan.h>

#include <vector>

namespace wowgm::graphics
{
    class SwapChain;

    /*
     * This class holds informations about a pipeline's binding description,
     * attribute description, and input state.
     *
     * When we want to draw a geometry, we prepare vertices along with their
     * additional attributes like normal vectors, colors, or texture coordinates.
     * Such vertex data is chosen arbitrarily by us, so for the hardware to
     * properly use them, we need to specify how many attributes there are,
     * how are they laid out in memory, or where are they taken from. This
     * information is provided through the vertex binding description and
     * attribute description required to create a graphics pipeline.
     */
    class VertexInput
    {
    public:
        VertexInput(SwapChain* swapChain);

        void AddBinding(VkVertexInputBindingDescription binding);

        template <typename T>
        void AddBinding(std::uint32_t index, VkVertexInputRate inputRate) {
            AddBinding({ index, sizeof(T), inputRate });
        }

        // TODO: This was needed in OpenGL; is it still needed in Vulkan?
        // template <>
        // void AddBinding<glm::mat4>(std::uint32_t index, VkVertexInputRate inputRate)
        // {
        //     AddBinding({ index, sizeof(glm::vec4), inputRate });
        //     AddBinding({ index + 1, sizeof(glm::vec4), inputRate });
        //     AddBinding({ index + 2, sizeof(glm::vec4), inputRate });
        //     AddBinding({ index + 3, sizeof(glm::vec4), inputRate });
        // }

        void AddAttribute(VkVertexInputAttributeDescription attrDescription);

        /*
         * In case of drawing commands that use vertex indices, `restartEnable` specifies wether
         * a special index value should restart a primitive. Defaults to false.
         */
        void SetPrimitiveType(VkPrimitiveTopology topology, bool restartEnable = false);

        void SetTessellationControlPoints(std::uint32_t controlPoints);

        /*
         * Allows you to specify custom values for the viewport. You shouldn't need to call this, as values are inferred from the swapchain.
         */
        void SetViewport(std::uint32_t width, std::uint32_t height);

        /*
        * Allows you to specify custom values for the viewport. You shouldn't need to call this, as values are inferred from the swapchain.
        */
        void SetViewportDepth(float minDepth, float maxDepth);

        /*
        * Allows you to specify custom values for the scissors. You shouldn't need to call this, as values are inferred from the swapchain.
        */
        void SetScissors(std::uint32_t width, std::uint32_t height);

        /*
         * Finalize the viewport. This *needs* to be called.
         */
        void Finalize();

        /*
         * If set to true, fragments whose depth is outside of the min/max range
         * specified in a viewport state are clamped to the range. If set to false,
         * said fragments are discarded. Defaults to false.
         */
        void SetFragmentClampState(bool clampFragments);

        void SetWireframe(bool wireframe);
        void SetCulling(VkCullModeFlagBits cullMode);
        void SetFrontFaceOrientation(bool clockWise);
        void SetDepthTest(bool enable);
        void SetDepthTest(VkCompareOp op);
        void SetStencilTest(bool enable);

    private:
        std::vector<VkVertexInputBindingDescription> _vertexBindingDescriptions;
        std::vector<VkVertexInputAttributeDescription> _vertexAttributeDescriptions;

        VkPipelineInputAssemblyStateCreateInfo _inputAssembly;
        VkPipelineTessellationStateCreateInfo _tessellationState;
        VkPipelineViewportStateCreateInfo _viewportCreateInfo;
        VkPipelineRasterizationStateCreateInfo _rasterizationState;
        VkPipelineMultisampleStateCreateInfo _multisamplingState;
        VkPipelineDepthStencilStateCreateInfo _depthStencilState;

        VkViewport _viewport;
        VkRect2D _scissors;
    };
}

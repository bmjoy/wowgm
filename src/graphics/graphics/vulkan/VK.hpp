#pragma once

#include <vulkan/vulkan.h>
#include <cstdint>
#include <cstring>

#include <vk_mem_alloc.h>

#include <boost/functional/hash.hpp>
#include <tuple>

namespace gfx::vk
{
    class Instance;

    class PhysicalDevice;
    class Device;

    class Buffer;
    class Image;

    class ImageView;
    class Sampler;

    class CommandPool;
    class CommandBuffer;

    class Shader;
    class Pipeline;
    class PipelineCache;

    class DescriptorSetLayout;
    class DescriptorSetLayoutCache;
    class DescriptorPool;

    class Queue;

    class Swapchain;
    class Framebuffer;
    class RenderPass;

    enum PipelineResourceType
    {
        PIPELINE_RESOURCE_TYPE_INPUT = 0,
        PIPELINE_RESOURCE_TYPE_OUTPUT = 1,
        PIPELINE_RESOURCE_TYPE_SAMPLER = 2,
        PIPELINE_RESOURCE_TYPE_COMBINED_IMAGE_SAMPLER = 3,
        PIPELINE_RESOURCE_TYPE_SAMPLED_IMAGE = 4,
        PIPELINE_RESOURCE_TYPE_STORAGE_IMAGE = 5,
        PIPELINE_RESOURCE_TYPE_UNIFORM_TEXEL_BUFFER = 6,
        PIPELINE_RESOURCE_TYPE_STORAGE_TEXEL_BUFFER = 7,
        PIPELINE_RESOURCE_TYPE_UNIFORM_BUFFER = 8,
        PIPELINE_RESOURCE_TYPE_STORAGE_BUFFER = 9,
        PIPELINE_RESOURCE_TYPE_INPUT_ATTACHMENT = 10,
        PIPELINE_RESOURCE_TYPE_PUSH_CONSTANT_BUFFER = 11,
    };

    enum PipelineResourceBaseType
    {
        PIPELINE_RESOURCE_BASE_TYPE_UNKNOWN = -1,
        PIPELINE_RESOURCE_BASE_TYPE_INT = 0,
        PIPELINE_RESOURCE_BASE_TYPE_UINT = 1,
        PIPELINE_RESOURCE_BASE_TYPE_FLOAT = 2,
        PIPELINE_RESOURCE_BASE_TYPE_DOUBLE = 3,
    };

    struct ApplicationInfo
    {
        const char*                              pApplicationName;
        const char*                              pEngineName;
        uint32_t                                 applicationVersion;
        uint32_t                                 engineVersion;
        uint32_t                                 apiVersion;
    };

    struct InstanceCreateInfo
    {
        const ApplicationInfo*                        pApplicationInfo;
        std::vector<const char*>                      enabledLayerNames;
        std::vector<const char*>                      enabledExtensionNames;
        VkInstanceCreateFlags                         flags = 0;
        struct {
            VkDebugUtilsMessageSeverityFlagsEXT       messageSeverity;
            VkDebugUtilsMessageTypeFlagsEXT           messageType;
            PFN_vkDebugUtilsMessengerCallbackEXT      messengerCallback = nullptr;
            void*                                     pUserData = nullptr;
        } debugUtils;
        struct {
            VkDebugReportFlagsEXT                     flags;
            PFN_vkDebugReportCallbackEXT              callback = nullptr;
            void*                                     pUserData = nullptr;
        } debugReport;
    };

    struct DeviceCreateInfo
    {
        const void*                                   pNext = nullptr;
        PhysicalDevice*                               physicalDevice = nullptr;
        std::vector<const char*>                      enabledLayerNames;
        std::vector<const char*>                      enabledExtensionNames;
    };

    struct BufferCreateInfo
    {
        const char*                                   pBufferName = nullptr;

        const void*                                   pNext = nullptr;
        std::vector<uint32_t>                         queueFamilyIndices;
        VkDeviceSize                                  size = 0;
        VkBufferUsageFlags                            usage = 0;
    };

    struct ImageCreateInfo
    {
        const char*                                   pImageName = nullptr;

        const void*                                   pNext = nullptr;
        std::vector<uint32_t>                         queueFamilyIndices;
        VkImageType                                   imageType;
        VkFormat                                      format;
        VkExtent3D                                    extent;
        uint32_t                                      mipLevels;
        uint32_t                                      arrayLayers;
        VkSampleCountFlagBits                         samples;
        VkImageTiling                                 tiling;
        VkImageUsageFlags                             usage;
    };

    struct PipelineShaderStageCreateInfo
    {
        const char*                                   pName = nullptr;

        const void*                                   pNext = nullptr;
        const VkSpecializationInfo*                   pSpecializationInfo;
        VkPipelineShaderStageCreateFlags              flags;
        VkShaderStageFlagBits                         stage;
        Shader*                                       shader = nullptr;
    };

    struct PipelineVertexInputStateCreateInfo
    {
        const void*                                   pNext = nullptr;
        std::vector<VkVertexInputBindingDescription>  vertexBindingDescriptions;
        std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions;
        VkPipelineVertexInputStateCreateFlags         flags;
    };

    struct PipelineInputAssemblyStateCreateInfo
    {
        const void*                                   pNext = nullptr;
        VkPipelineInputAssemblyStateCreateFlags       flags;
        VkPrimitiveTopology                           topology;
        VkBool32                                      primitiveRestartEnable;
    };

    struct PipelineTessellationStateCreateInfo
    {
        const void*                                   pNext = nullptr;
        VkPipelineTessellationStateCreateFlags        flags;
        uint32_t                                      patchControlPoints;
    };

    struct PipelineViewportStateCreateInfo
    {
        const void*                                   pNext = nullptr;
        std::vector<VkViewport>                       viewports;
        std::vector<VkRect2D>                         scissors;
        VkPipelineViewportStateCreateFlags            flags;
    };

    struct PipelineRasterizationStateCreateInfo
    {
        const void*                                   pNext = nullptr;
        VkPipelineRasterizationStateCreateFlags       flags;
        VkBool32                                      depthClampEnable;
        VkBool32                                      rasterizerDiscardEnable;
        VkPolygonMode                                 polygonMode;
        VkCullModeFlags                               cullMode;
        VkFrontFace                                   frontFace;
        VkBool32                                      depthBiasEnable;
        float                                         depthBiasConstantFactor;
        float                                         depthBiasClamp;
        float                                         depthBiasSlopeFactor;
        float                                         lineWidth = 1.0f;
    };

    struct PipelineMultisampleStateCreateInfo
    {
        const void*                                   pNext = nullptr;
        const VkSampleMask*                           pSampleMask;
        VkPipelineMultisampleStateCreateFlags         flags;
        VkSampleCountFlagBits                         rasterizationSamples;
        VkBool32                                      sampleShadingEnable;
        float                                         minSampleShading;
        VkBool32                                      alphaToCoverageEnable;
        VkBool32                                      alphaToOneEnable;
    };

    struct PipelineDepthStencilStateCreateInfo
    {
        const void*                                   pNext = nullptr;
        VkPipelineDepthStencilStateCreateFlags        flags;
        VkBool32                                      depthTestEnable;
        VkBool32                                      depthWriteEnable;
        VkCompareOp                                   depthCompareOp;
        VkBool32                                      depthBoundsTestEnable;
        VkBool32                                      stencilTestEnable;
        VkStencilOpState                              front;
        VkStencilOpState                              back;
        float                                         minDepthBounds;
        float                                         maxDepthBounds;
    };

    struct PipelineColorBlendStateCreateInfo
    {
        const void*                                   pNext = nullptr;
        VkPipelineColorBlendStateCreateFlags          flags;
        VkBool32                                      logicOpEnable = VK_FALSE;
        VkLogicOp                                     logicOp = VK_LOGIC_OP_COPY;
        std::vector<VkPipelineColorBlendAttachmentState> attachments;
        float                                         blendConstants[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    };

    struct PipelineDynamicStateCreateInfo
    {
        const void*                                   pNext = nullptr;
        VkPipelineDynamicStateCreateFlags             flags;
        uint32_t                                      dynamicStateCount;
        const VkDynamicState*                         pDynamicStates;
    };

#pragma pack(push, 1)
    struct PipelineDynamicState
    {
        bool                                          viewport : 1;
        bool                                          scissors : 1;
        bool                                          lineWidth : 1;
        bool                                          depthBias : 1;
        bool                                          blendConstants : 1;
        bool                                          depthBounds : 1;
        bool                                          stencilCompareMask : 1;
        bool                                          stencilWriteMask : 1;
        bool                                          stencilReference : 1;

        PipelineDynamicState()
        {
            std::memset(this, 0, sizeof(PipelineDynamicState)); // I feel bad.
        }

        operator bool() const
        {
            return viewport || scissors || lineWidth || depthBias || blendConstants || depthBounds || stencilCompareMask || stencilWriteMask || stencilReference;
        }
    };
#pragma pack(pop)

    struct GraphicsPipelineCreateInfo
    {
        const void*                                      pNext = nullptr;
        VkPipelineCreateFlags                            flags = 0;
        std::vector<Shader*>                             shaders;
        PipelineVertexInputStateCreateInfo               vertexInputState{};
        PipelineInputAssemblyStateCreateInfo             inputAssemblyState{};
        PipelineTessellationStateCreateInfo              tessellationState{};
        PipelineViewportStateCreateInfo                  viewportState{};
        PipelineRasterizationStateCreateInfo             rasterizationState{};
        PipelineMultisampleStateCreateInfo               multisampleState{};
        PipelineDepthStencilStateCreateInfo              depthStencilState;
        PipelineColorBlendStateCreateInfo                colorBlendState{};
        PipelineDynamicState                             dynamicState{};
        VkPipelineLayout*                                pLayout = nullptr; // Optional. If not set, we generate the layout from SPIRV reflection.
        RenderPass*                                      renderPass = nullptr;
        uint32_t                                         subpass = 0;
        VkPipeline                                       basePipelineHandle = VK_NULL_HANDLE;
        int32_t                                          basePipelineIndex = -1;
    };

    struct ShaderModuleCreateInfo
    {
        const void*                                      pNext = nullptr;
        VkShaderStageFlagBits                            stage;
        std::vector<uint8_t>                             code;
        const char*                                      pGLSLSource = nullptr;
        const char*                                      pEntryPoint = nullptr;
    };

    struct PipelineResource
    {
        VkShaderStageFlags                               stages;
        PipelineResourceType                             resourceType;
        PipelineResourceBaseType                         baseType;
        VkAccessFlags                                    access;
        uint32_t                                         set;
        uint32_t                                         binding;
        uint32_t                                         location;
        uint32_t                                         inputAttachmentIndex;
        uint32_t                                         vecSize;
        uint32_t                                         columns;
        uint32_t                                         arraySize;
        uint32_t                                         offset;
        uint32_t                                         size;
        char                                             name[VK_MAX_DESCRIPTION_SIZE];
    };

    struct SwapchainCreateInfo
    {
        const void*                                      pNext = nullptr;
        VkSurfaceKHR                                     surface;
        VkSurfaceFormatKHR                               preferredFormat;
        bool                                             tripleBuffer;
    };

    struct ImageSubresourceRange
    {
        uint32_t                                         baseMipLevel;
        uint32_t                                         levelCount;
        uint32_t                                         baseArrayLayer;
        uint32_t                                         layerCount;
    };

    struct ImageViewCreateInfo
    {
        const void*                                      pNext = nullptr;
        Image*                                           image;
        VkImageViewType                                  viewType;
        VkComponentMapping                               components;
        ImageSubresourceRange                            subresourceRange;
    };

    struct ImageSubresourceLayers
    {
        uint32_t                                         mipLevel;
        uint32_t                                         baseArrayLayer;
        uint32_t                                         layerCount;
    };

    struct ImageSubDataInfo
    {
        uint32_t                                         dataRowLength;
        uint32_t                                         dataImageHeight;
        ImageSubresourceLayers                           imageSubresource;
        VkOffset3D                                       imageOffset;
        VkExtent3D                                       imageExtent;
    };

    struct ImageMemoryBarrier
    {
        VkAccessFlags                                    srcAccessMask;
        VkAccessFlags                                    dstAccessMask;
        VkImageLayout                                    oldLayout;
        VkImageLayout                                    newLayout;
        uint32_t                                         srcQueueFamilyIndex;
        uint32_t                                         dstQueueFamilyIndex;
        Image*                                           image;
        ImageSubresourceRange                            subresourceRange;
    };

    struct BufferImageCopy
    {
        VkDeviceSize                                     bufferOffset;
        uint32_t                                         bufferRowLength;
        uint32_t                                         bufferImageHeight;
        ImageSubresourceLayers                           imageSubresource;
        VkOffset3D                                       imageOffset;
        VkExtent3D                                       imageExtent;
    };

    // https://vulkan.lunarg.com/doc/view/1.0.33.0/linux/vkspec.chunked/ch29s06.html#VkPresentInfoKHR
    struct PresentInfo
    {
        struct PresentChain {
            //< A given swapchain must not appear in this list more than once.
            Swapchain* swapchain = nullptr;
            //< Each entry in this array identifies the image to present on the corresponding entry in the pSwapchains array.
            Image*     image = nullptr;
            //<  Applications that do not need per-swapchain results can use NULL for pResults.
            //< If non-NULL, each entry in pResults will be set to the VkResult for presenting
            //< the swapchain corresponding to the same index in pSwapchains.
            VkResult result;
        };

        // pNext is NULL or a pointer to an extension-specific structure.
        const void*                                      pNext = nullptr;

        std::vector<PresentChain>                        swapchains;
        // waitSemaphores, if not VK_NULL_HANDLE, is an array of VkSemaphore objects,
        // and specifies the semaphores to wait for before issuing the present request.
        std::vector<VkSemaphore>                         waitSemaphores;
    };

    struct SubmitInfo
    {
        const void*                                      pNext = nullptr;
        std::vector<std::pair<VkSemaphore, VkPipelineStageFlags>> waitSemaphores;
        std::vector<CommandBuffer*>                      commandBuffers;
        std::vector<VkSemaphore>                         signalSemaphores;
    };

    struct AttachmentReference
    {
        VkAttachmentLoadOp                               loadOp;
        VkAttachmentStoreOp                              storeOp;
        VkClearValue                                     clearValue;
    };

    struct RenderPassBeginInfo
    {
        const void*                                      pNext = nullptr;
        Framebuffer*                                     pFramebuffer;
        RenderPass*                                      pRenderPass;
        VkRect2D                                         renderArea;
        std::vector<VkClearValue>                        clearValues;
    };

    struct SamplerCreateInfo
    {
        const void*                                      pNext = nullptr;
        VkFilter                                         magFilter;
        VkFilter                                         minFilter;
        VkSamplerMipmapMode                              mipmapMode;
        VkSamplerAddressMode                             addressModeU;
        VkSamplerAddressMode                             addressModeV;
        VkSamplerAddressMode                             addressModeW;
        float                                            mipLodBias;
        VkBool32                                         anisotropyEnable;
        float                                            maxAnisotropy;
        VkBool32                                         compareEnable;
        VkCompareOp                                      compareOp;
        float                                            minLod;
        float                                            maxLod;
        VkBorderColor                                    borderColor;
        VkBool32                                         unnormalizedCoordinates;
    };

    struct FramebufferCreateInfo
    {
        const void*                                      pNext = nullptr;
        std::vector<ImageView*>                          attachments;
        uint32_t                                         width;
        uint32_t                                         height;
        uint32_t                                         layers;
        RenderPass*                                      pRenderPass;
    };

    struct ClearAttachment
    {
        uint32_t              colorAttachment;
        VkClearValue          clearValue;
    };
}

namespace std
{
    // I'm sorry.
    template <typename... Ts>
    struct hash<tuple<Ts...>>
    {
        size_t operator()(tuple<Ts...> const& arg) const noexcept
        {
            return boost::hash_value(arg);
        }
    };
}

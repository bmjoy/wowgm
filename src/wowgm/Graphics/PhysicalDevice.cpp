#include "PhysicalDevice.hpp"
#include "Surface.hpp"
#include "Instance.hpp"

#include "SharedGraphicsDefines.hpp"

#include <set>
#include <string>

namespace wowgm::graphics
{
    bool QueueFamilyIndices::IsComplete()
    {
        return Graphics >= 0 && Present >= 0;
    }

    std::uint32_t QueueFamilyIndices::GetQueueCount()
    {
        return sizeof(QueueFamilyIndices) / sizeof(std::int32_t);
    }

    std::int32_t* QueueFamilyIndices::EnumerateFamilies()
    {
        return reinterpret_cast<std::int32_t*>(this);
    }

    PhysicalDevice::~PhysicalDevice()
    {
        _surface = nullptr;

        // Managed by Vulkan (no vkCreate... call!)
        _device = VK_NULL_HANDLE;
    }


    VkPhysicalDeviceMemoryProperties& PhysicalDevice::GetMemoryProperties()
    {
        return _deviceMemoryProperties;
    }

    PhysicalDevice::PhysicalDevice(VkPhysicalDevice device, Surface* surface) : _device(device), _deviceScore(0), _surface(surface)
    {
        vkGetPhysicalDeviceProperties(device, &_deviceProperties);
        vkGetPhysicalDeviceFeatures(device, &_deviceFeatures);

        if (_deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) // Favor dedicated graphics card
            _deviceScore += 1000;

        // Favor devices with larger texture sizes
        _deviceScore += _deviceProperties.limits.maxImageDimension2D;

        {
            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

            std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

            std::int32_t i = 0;
            for (const auto& queueFamily : queueFamilies)
            {
                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, *surface, &presentSupport);

                if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                    _queueFamilyIndices.Graphics = i;

                if (presentSupport && queueFamily.queueCount > 0)
                    _queueFamilyIndices.Present = i;

                if (_queueFamilyIndices.IsComplete())
                    break;

                i++;
            }
        }

        _CreateSwapChainSupportDetails();

        bool extensionsSupported = CheckDeviceExtensionSupport();
        bool swapChainAdequate = !_surfaceFormats.empty() && !_surfacePresentModes.empty();

        // Ignore devices without a graphics queue, without the required expansion support, or without swap chain support
        if (!_queueFamilyIndices.IsComplete() || !extensionsSupported || !swapChainAdequate)
            _deviceScore = 0;
    }

    std::uint32_t PhysicalDevice::GetScore()
    {
        return _deviceScore;
    }

    QueueFamilyIndices& PhysicalDevice::GetQueues()
    {
        return _queueFamilyIndices;
    }

    VkPhysicalDeviceFeatures& PhysicalDevice::GetPhysicalDeviceFeatures()
    {
        return _deviceFeatures;
    }

    VkPhysicalDeviceProperties& PhysicalDevice::GetPhysicalDeviceProperties()
    {
        return _deviceProperties;
    }

    bool PhysicalDevice::CheckDeviceExtensionSupport()
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(_device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(_device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(details::DeviceExtensions.begin(), details::DeviceExtensions.end());

        for (const auto& extension : availableExtensions)
            requiredExtensions.erase(extension.extensionName);

        return requiredExtensions.empty();
    }

    VkSurfaceCapabilitiesKHR& PhysicalDevice::GetCapabilities()
    {
        return _surfaceCapabilities;
    }

    std::vector<VkSurfaceFormatKHR>& PhysicalDevice::GetFormats()
    {
        return _surfaceFormats;
    }

    std::vector<VkPresentModeKHR> PhysicalDevice::GetPresentModes()
    {
        return _surfacePresentModes;
    }

    void PhysicalDevice::_CreateSwapChainSupportDetails()
    {
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_device, *_surface, &_surfaceCapabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(_device, *_surface, &formatCount, nullptr);

        if (formatCount != 0)
        {
            _surfaceFormats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(_device, *_surface, &formatCount, _surfaceFormats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(_device, *_surface, &presentModeCount, nullptr);

        if (presentModeCount != 0)
        {
            _surfacePresentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(_device, *_surface, &presentModeCount, _surfacePresentModes.data());
        }
    }

    Surface* PhysicalDevice::GetSurface()
    {
        return _surface;
    }

    Instance* PhysicalDevice::GetInstance()
    {
        return _surface->GetInstance();
    }


    std::string PhysicalDevice::ToString()
    {
        std::stringstream ss;
        ToString(ss);
        return ss.str();
    }

    namespace details
    {
        template <typename T>
        inline std::ostream& stream(std::ostream& ss, T const& t)
        {
            ss << t;
            return ss;
        }

        template <typename T, size_t N>
        inline std::ostream& stream(std::ostream& ss, T const (&n)[N])
        {
            ss << "{\n        [0] = ";
            stream(ss, n[0]);
            ss << ",\n";
            for (int i = 1; i < N; ++i)
            {
                ss << "        [" << i << "] = ";
                stream(ss, n[i]);
                ss << ",\n";
            }
            ss << "    }";
            ss.flush();
            return ss;
        }

        template <>
        inline std::ostream& stream<VkDeviceSize>(std::ostream& ss, VkDeviceSize const& deviceSize)
        {
            static const int unit = 1024;
            if (deviceSize < unit)
                ss << static_cast<int>(deviceSize) << " B";
            else
            {
                auto exp = static_cast<std::int32_t>(std::log(deviceSize) / std::log(unit));
                char pre = "KMGTPE"[exp - 1];
                ss << static_cast<float>(deviceSize / std::pow(unit, exp)) << ' ' << pre << "iB";
            }
            return ss;
        }
    }

    void PhysicalDevice::ToString(std::stringstream& ss)
    {
        ss << _deviceProperties.deviceName << std::endl;
#define PRINT_LIMIT(name) { ss << "    " << #name << " : "; details::stream(ss, _deviceProperties.limits.name); ss << std::endl; }

        PRINT_LIMIT(maxImageDimension1D);
        PRINT_LIMIT(maxImageDimension2D);
        PRINT_LIMIT(maxImageDimension3D);
        PRINT_LIMIT(maxImageDimensionCube);
        PRINT_LIMIT(maxImageArrayLayers);
        PRINT_LIMIT(maxTexelBufferElements);
        PRINT_LIMIT(maxUniformBufferRange);
        PRINT_LIMIT(maxStorageBufferRange);
        PRINT_LIMIT(maxPushConstantsSize);
        PRINT_LIMIT(maxMemoryAllocationCount);
        PRINT_LIMIT(maxSamplerAllocationCount);
        PRINT_LIMIT(bufferImageGranularity);
        PRINT_LIMIT(sparseAddressSpaceSize);
        PRINT_LIMIT(maxBoundDescriptorSets);
        PRINT_LIMIT(maxPerStageDescriptorSamplers);
        PRINT_LIMIT(maxPerStageDescriptorUniformBuffers);
        PRINT_LIMIT(maxPerStageDescriptorStorageBuffers);
        PRINT_LIMIT(maxPerStageDescriptorSampledImages);
        PRINT_LIMIT(maxPerStageDescriptorStorageImages);
        PRINT_LIMIT(maxPerStageDescriptorInputAttachments);
        PRINT_LIMIT(maxPerStageResources);
        PRINT_LIMIT(maxDescriptorSetSamplers);
        PRINT_LIMIT(maxDescriptorSetUniformBuffers);
        PRINT_LIMIT(maxDescriptorSetUniformBuffersDynamic);
        PRINT_LIMIT(maxDescriptorSetStorageBuffers);
        PRINT_LIMIT(maxDescriptorSetStorageBuffersDynamic);
        PRINT_LIMIT(maxDescriptorSetSampledImages);
        PRINT_LIMIT(maxDescriptorSetStorageImages);
        PRINT_LIMIT(maxDescriptorSetInputAttachments);
        PRINT_LIMIT(maxVertexInputAttributes);
        PRINT_LIMIT(maxVertexInputBindings);
        PRINT_LIMIT(maxVertexInputAttributeOffset);
        PRINT_LIMIT(maxVertexInputBindingStride);
        PRINT_LIMIT(maxVertexOutputComponents);
        PRINT_LIMIT(maxTessellationGenerationLevel);
        PRINT_LIMIT(maxTessellationPatchSize);
        PRINT_LIMIT(maxTessellationControlPerVertexInputComponents);
        PRINT_LIMIT(maxTessellationControlPerVertexOutputComponents);
        PRINT_LIMIT(maxTessellationControlPerPatchOutputComponents);
        PRINT_LIMIT(maxTessellationControlTotalOutputComponents);
        PRINT_LIMIT(maxTessellationEvaluationInputComponents);
        PRINT_LIMIT(maxTessellationEvaluationOutputComponents);
        PRINT_LIMIT(maxGeometryShaderInvocations);
        PRINT_LIMIT(maxGeometryInputComponents);
        PRINT_LIMIT(maxGeometryOutputComponents);
        PRINT_LIMIT(maxGeometryOutputVertices);
        PRINT_LIMIT(maxGeometryTotalOutputComponents);
        PRINT_LIMIT(maxFragmentInputComponents);
        PRINT_LIMIT(maxFragmentOutputAttachments);
        PRINT_LIMIT(maxFragmentDualSrcAttachments);
        PRINT_LIMIT(maxFragmentCombinedOutputResources);
        PRINT_LIMIT(maxComputeSharedMemorySize);
        PRINT_LIMIT(maxComputeWorkGroupCount);
        PRINT_LIMIT(maxComputeWorkGroupInvocations);
        PRINT_LIMIT(maxComputeWorkGroupSize);
        PRINT_LIMIT(subPixelPrecisionBits);
        PRINT_LIMIT(subTexelPrecisionBits);
        PRINT_LIMIT(mipmapPrecisionBits);
        PRINT_LIMIT(maxDrawIndexedIndexValue);
        PRINT_LIMIT(maxDrawIndirectCount);
        PRINT_LIMIT(maxSamplerLodBias);
        PRINT_LIMIT(maxSamplerAnisotropy);
        PRINT_LIMIT(maxViewports);
        PRINT_LIMIT(maxViewportDimensions);
        PRINT_LIMIT(viewportBoundsRange);
        PRINT_LIMIT(viewportSubPixelBits);
        PRINT_LIMIT(minMemoryMapAlignment);
        PRINT_LIMIT(minTexelBufferOffsetAlignment);
        PRINT_LIMIT(minUniformBufferOffsetAlignment);
        PRINT_LIMIT(minStorageBufferOffsetAlignment);
        PRINT_LIMIT(minTexelOffset);
        PRINT_LIMIT(maxTexelOffset);
        PRINT_LIMIT(minTexelGatherOffset);
        PRINT_LIMIT(maxTexelGatherOffset);
        PRINT_LIMIT(minInterpolationOffset);
        PRINT_LIMIT(maxInterpolationOffset);
        PRINT_LIMIT(subPixelInterpolationOffsetBits);
        PRINT_LIMIT(maxFramebufferWidth);
        PRINT_LIMIT(maxFramebufferHeight);
        PRINT_LIMIT(maxFramebufferLayers);
        PRINT_LIMIT(framebufferColorSampleCounts);
        PRINT_LIMIT(framebufferDepthSampleCounts);
        PRINT_LIMIT(framebufferStencilSampleCounts);
        PRINT_LIMIT(framebufferNoAttachmentsSampleCounts);
        PRINT_LIMIT(maxColorAttachments);
        PRINT_LIMIT(sampledImageColorSampleCounts);
        PRINT_LIMIT(sampledImageIntegerSampleCounts);
        PRINT_LIMIT(sampledImageDepthSampleCounts);
        PRINT_LIMIT(sampledImageStencilSampleCounts);
        PRINT_LIMIT(storageImageSampleCounts);
        PRINT_LIMIT(maxSampleMaskWords);
        PRINT_LIMIT(timestampComputeAndGraphics);
        PRINT_LIMIT(timestampPeriod);
        PRINT_LIMIT(maxClipDistances);
        PRINT_LIMIT(maxCullDistances);
        PRINT_LIMIT(maxCombinedClipAndCullDistances);
        PRINT_LIMIT(discreteQueuePriorities);
        PRINT_LIMIT(pointSizeRange);
        PRINT_LIMIT(lineWidthRange);
        PRINT_LIMIT(pointSizeGranularity);
        PRINT_LIMIT(lineWidthGranularity);
        PRINT_LIMIT(strictLines);
        PRINT_LIMIT(standardSampleLocations);
        PRINT_LIMIT(optimalBufferCopyOffsetAlignment);
        PRINT_LIMIT(optimalBufferCopyRowPitchAlignment);
        PRINT_LIMIT(nonCoherentAtomSize);

#undef PRINT_LIMIT
    }
}

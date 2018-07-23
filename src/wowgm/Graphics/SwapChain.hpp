#pragma once
#include "Instance.hpp"

#include <vector>
#include <cstdint>
#include <vulkan/vulkan.h>

namespace wowgm::graphics
{
    class LogicalDevice;
    class PhysicalDevice;
    class Surface;
    class ImageView;
    class Image;
    class FrameBuffer;
    class Pipeline;

    class SwapChain
    {
        friend SwapChain* Instance::CreateSwapChain();
        SwapChain(PhysicalDevice* device);

        SwapChain(SwapChain&&) = delete;
        SwapChain(SwapChain const&) = delete;

    public:
        ~SwapChain();

        VkSurfaceFormatKHR GetSurfaceFormat();
        VkPresentModeKHR GetPresentMode();
        VkExtent2D GetExtent();

        PhysicalDevice* GetPhysicalDevice();
        LogicalDevice* GetLogicalDevice();

        Image* GetImage(std::uint32_t index);
        ImageView* GetImageView(std::uint32_t index);
        std::vector<ImageView*>& GetImageViews();
        std::uint32_t GetImageCount();

        operator VkSwapchainKHR() const { return _swapChain; }

    private:
        void _SelectFormat();
        void _SelectPresentMode();
        void _SelectExtent();

    private:
        PhysicalDevice* _physicalDevice;

        VkSwapchainKHR _swapChain;

        VkSurfaceFormatKHR _surfaceFormat;
        VkPresentModeKHR _presentMode;
        VkExtent2D _extent;

        std::vector<Image*> _swapChainImages;
        std::vector<ImageView*> _imageViews;
    };
}
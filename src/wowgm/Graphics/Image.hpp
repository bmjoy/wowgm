#pragma once

#include <vulkan/vulkan.h>

namespace wowgm::graphics
{
    class Image
    {
    public:
        Image(VkImage image);
        Image();

        VkImage GetVkImage();

        operator VkImage() const { return _image; }

    private:
        VkImage _image;
    };
}
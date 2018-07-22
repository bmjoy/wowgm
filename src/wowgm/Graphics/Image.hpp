#pragma once

#include <vulkan/vulkan.h>

namespace wowgm::graphics
{
    class Image
    {
        Image(Image&&) = default;
        Image(const Image&) = delete;

    public:
        explicit Image(VkImage image);
        Image();

        ~Image() = default;

        operator VkImage() const { return _image; }

    private:
        VkImage _image;
    };
}
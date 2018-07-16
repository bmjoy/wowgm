#pragma once

#include <vulkan/vulkan.h>

namespace wowgm::graphics
{
    class Image
    {
    public:
        Image(VkImage image);
        Image();

        Image(Image&&) = default;
        Image(Image const&) = delete;

        ~Image() = default;

        operator VkImage() const { return _image; }

    private:
        VkImage _image;
    };
}
#include "Image.hpp"

#include <GLFW/glfw3.h>

namespace wowgm::graphics
{
    Image::Image(VkImage image) : _image(image)
    {
    }

    Image::Image()
    {

    }
}
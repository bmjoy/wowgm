#pragma once

#include "VEZ.hpp"

#include <vector>

namespace vez
{
    class Framebuffer
    {
    public:
        static VkResult Create(Device* pDevice, const FramebufferCreateInfo* pCreateInfo, Framebuffer** ppFramebuffer);

        ~Framebuffer();

        VkExtent2D GetExtents() const;

        ImageView* GetAttachment(uint32_t attachmentIndex) const;

        VkFramebuffer GetHandle() const { return _handle; }

    private:
        Device* _device = nullptr;
        uint32_t _width;
        uint32_t _height;
        uint32_t _layers;

        std::vector<ImageView*> _attachments; // Non-owning!
        VkFramebuffer _handle = VK_NULL_HANDLE;
    };
}
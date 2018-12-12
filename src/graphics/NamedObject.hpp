#pragma once

#include "Instance.hpp"
#include <string_view>
#include <string>

namespace vez
{
    class Device;

    template <typename T>
    class NamedObject
    {
    public:
        inline VkResult SetName(std::string_view name)
        {
#if _DEBUG
            _objectName = name;
            /// return GetInstance()->SetObjectName(GetDevice(), this, name);
#endif
            return VK_ERROR_LAYER_NOT_PRESENT;
        }

        virtual Instance* GetInstance() const = 0;
        virtual Device* GetDevice() const = 0;

        std::string const& GetName() const { return _objectName; }

    private:
        std::string _objectName;
    };
}

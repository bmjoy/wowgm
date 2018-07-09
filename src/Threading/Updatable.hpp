#pragma once

#include <cstdint>

class Updatable
{
    public:
        virtual void Update(std::uint32_t timeInterval) = 0;
};

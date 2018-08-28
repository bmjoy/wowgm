#pragma once

#include <cstdint>

namespace wowgm::threading {

    class Updatable
    {
    public:
        virtual ~Updatable() { }

        virtual void Update(std::uint32_t timeInterval) = 0;

        virtual void Destroy() = 0;
    };

} // wowgm::threading

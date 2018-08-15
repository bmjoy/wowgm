#pragma once

#include "C3Vector.hpp"

namespace wowgm::game::structures
{
    struct C4Vector : public C3Vector
    {
        C4Vector() : C3Vector() { }

        float O = 0.0f;
    };
}
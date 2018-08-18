#pragma once

#include "C3Vector.hpp"

namespace wowgm::game::structures
{
    /// An axis aligned box described by the minimum and maximum point.
    struct CAaBox
    {
        C3Vector Minimum;
        C3Vector Maximum;
    };
}

#pragma once

#include "ObjectGuid.hpp"
#include "CMovementStatus.hpp"
#include "JamCliValuesUpdate.hpp"

namespace wowgm::game::structures
{
    enum class UpdateType : std::uint8_t
    {
        Values = 0,
        CreateObject1 = 1,
        CreateObject2 = 2,
        DestroyObjects = 3
    };

    struct CClientObjCreate
    {
        CClientObjCreate() : GUID(), Movement(), Values() { }

        UpdateType UpdateType;
        ObjectGuid GUID;
        CMovementStatus Movement;
        JamCliValuesUpdate Values;
    };
}
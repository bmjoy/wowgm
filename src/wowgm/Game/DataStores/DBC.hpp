#pragma once

#include "DBStorage.hpp"
#include "DBCStructures.hpp"
#include "DBCMeta.hpp"

namespace wowgm::game::datastores
{
    void Initialize();

    extern Storage<MapEntry, MapMeta> Map;
}

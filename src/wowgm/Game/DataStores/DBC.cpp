#include "DBC.hpp"

namespace wowgm::game::datastores
{
    Storage<MapEntry, MapMeta> Map;

    void Initialize()
    {
        Map.Initialize();
    }
}
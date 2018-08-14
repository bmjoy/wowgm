#include "ObjectMgr.hpp"

namespace wowgm::game::entities
{
    ObjectMgr* ObjectMgr::Instance()
    {
        static ObjectMgr instance;
        return &instance;
    }
}

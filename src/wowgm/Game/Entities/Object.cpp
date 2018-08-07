#include "Object.hpp"

namespace wowgm::game::entities
{
    using namespace wowgm::utilities;

    Object::Object()
    {

    }

    Object::~Object()
    {

    }

    ObjectGuid Object::GetGUID() const
    {
        return _objectData.GUID.GetValue();
    }

    CGObjectData const& Object::GetData() const
    {
        return _objectData;
    }
}
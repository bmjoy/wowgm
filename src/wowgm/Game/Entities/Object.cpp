#include "Object.hpp"

namespace wowgm::game::entities
{
    using namespace wowgm::utilities;

    Object::Object() : CGObjectData()
    {

    }

    Object::~Object()
    {

    }

    ObjectGuid Object::GetGUID() const
    {
        return GUID;
    }

    CGObjectData const& Object::GetData() const
    {
        return *this;
    }

    float Object::GetScale() const
    {
        return Scale;
    }
}
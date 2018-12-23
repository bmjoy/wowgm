#include "ObjectGuid.hpp"
#include <shared/assert/assert.hpp>

#include <sstream>
#include <iomanip>
#include <string>

namespace wowgm::game::structures
{
    ObjectGuid const ObjectGuid::Empty = ObjectGuid();

    uint8_t& ObjectGuid::operator [] (uint32_t index)
    {
        BOOST_ASSERT(index < sizeof(uint64_t));
        return _data._bytes[index];
    }

    uint8_t const& ObjectGuid::operator [] (uint32_t index) const
    {
        BOOST_ASSERT(index < sizeof(uint64_t));
        return _data._bytes[index];
    }

    char const* ObjectGuid::GetTypeName(HighGuid high)
    {
        switch (high)
        {
            case HighGuid::Item:          return "Item";
            case HighGuid::Player:        return "Player";
            case HighGuid::GameObject:    return "Gameobject";
            case HighGuid::Transport:     return "Transport";
            case HighGuid::Unit:          return "Creature";
            case HighGuid::Pet:           return "Pet";
            case HighGuid::Vehicle:       return "Vehicle";
            case HighGuid::DynamicObject: return "DynObject";
            case HighGuid::Corpse:        return "Corpse";
            case HighGuid::AreaTrigger:   return "AreaTrigger";
            case HighGuid::BattleGround:  return "Battleground";
            case HighGuid::Mo_Transport:  return "MoTransport";
            case HighGuid::Instance:      return "InstanceID";
            case HighGuid::Group:         return "Group";
            case HighGuid::Guild:         return "Guild";
            default:
                return "<unknown>";
        }
    }

    std::string ObjectGuid::ToString() const
    {
        std::ostringstream str;
        str << "GUID Full: 0x" << std::hex << std::setw(16) << std::setfill('0') << _data._guid << std::dec;
        str << " Type: " << GetTypeName();
        if (HasEntry())
            str << (IsPet() ? " Pet number: " : " Entry: ") << GetEntry();

        str << " Low: " << GetCounter();
        return str.str();
    }

    ByteBuffer& operator << (ByteBuffer& buf, ObjectGuid const& guid)
    {
        buf << uint64_t(guid.GetRawValue());
        return buf;
    }

    ByteBuffer& operator >> (ByteBuffer& buf, ObjectGuid& guid)
    {
        guid.Set(buf.read<uint64_t>());
        return buf;
    }

    ByteBuffer& operator << (ByteBuffer& buf, PackedGuid const& guid)
    {
        buf.append(guid._packedGuid);
        return buf;
    }

}
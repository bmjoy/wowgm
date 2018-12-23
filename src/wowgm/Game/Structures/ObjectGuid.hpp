#pragma once

#include <shared/assert/assert.hpp>
#include "ByteBuffer.hpp"

#include <cstdint>

namespace wowgm::game::structures
{
    enum TypeID : uint8_t
    {
        TYPEID_OBJECT        = 0,
        TYPEID_ITEM          = 1,
        TYPEID_CONTAINER     = 2,
        TYPEID_UNIT          = 3,
        TYPEID_PLAYER        = 4,
        TYPEID_GAMEOBJECT    = 5,
        TYPEID_DYNAMICOBJECT = 6,
        TYPEID_CORPSE        = 7,
        TYPEID_AREATRIGGER   = 8
    };

    enum TypeMask
    {
        TYPEMASK_OBJECT         = 0x0001,
        TYPEMASK_ITEM           = 0x0002,
        TYPEMASK_CONTAINER      = 0x0006,                       // TYPEMASK_ITEM | 0x0004
        TYPEMASK_UNIT           = 0x0008,                       // creature
        TYPEMASK_PLAYER         = 0x0010,
        TYPEMASK_GAMEOBJECT     = 0x0020,
        TYPEMASK_DYNAMICOBJECT  = 0x0040,
        TYPEMASK_CORPSE         = 0x0080,
        TYPEMASK_AREATRIGGER    = 0x0100,
        TYPEMASK_SEER           = TYPEMASK_PLAYER | TYPEMASK_UNIT | TYPEMASK_DYNAMICOBJECT
    };

    enum class HighGuid
    {
        Item           = 0x400,                       // blizz 4000
        Container      = 0x400,                       // blizz 4000
        Player         = 0x000,                       // blizz 0000
        GameObject     = 0xF11,                       // blizz F110
        Transport      = 0xF12,                       // blizz F120 (for GAMEOBJECT_TYPE_TRANSPORT)
        Unit           = 0xF13,                       // blizz F130
        Pet            = 0xF14,                       // blizz F140
        Vehicle        = 0xF15,                       // blizz F550
        DynamicObject  = 0xF10,                       // blizz F100
        Corpse         = 0xF101,                      // blizz F100
        AreaTrigger    = 0xF102,
        BattleGround   = 0x1F1,
        Mo_Transport   = 0x1FC,                       // blizz 1FC0 (for GAMEOBJECT_TYPE_MO_TRANSPORT)
        Instance       = 0x1F4,                       // blizz 1F40
        Group          = 0x1F5,
        Guild          = 0x1FF
    };

    class ObjectGuid;
    class PackedGuid;

    struct PackedGuidReader
    {
        explicit PackedGuidReader(ObjectGuid& guid) : Guid(guid) { }
        ObjectGuid& Guid;
    };

    class ObjectGuid
    {
        public:
            static ObjectGuid const Empty;

            typedef uint32_t LowType;

            ObjectGuid()
            {
                _data._guid = 0uLL;
            }

            explicit ObjectGuid(uint64_t guid)
            {
                _data._guid = guid;
            }

            ObjectGuid(HighGuid hi, uint32_t entry, uint32_t counter)
            {
                _data._guid = counter
                    ? uint64_t(counter) | (uint64_t(entry) << 32) | (uint64_t(hi) << ((hi == HighGuid::Corpse || hi == HighGuid::AreaTrigger) ? 48 : 52))
                    : 0;
            }

            ObjectGuid(HighGuid hi, uint32_t counter)
            {
                _data._guid = counter
                    ? uint64_t(counter) | (uint64_t(hi) << ((hi == HighGuid::Corpse || hi == HighGuid::AreaTrigger) ? 48 : 52))
                    : 0;
            }

            ObjectGuid(ObjectGuid const& r) : _data(r._data) { }
            ObjectGuid(ObjectGuid&& r) : _data(r._data) { }

            ObjectGuid& operator = (ObjectGuid const& r) { _data = r._data; return *this; }
            ObjectGuid& operator = (ObjectGuid&& r) { _data = r._data; return *this; }

            operator uint64_t() const { return _data._guid; }
            PackedGuidReader ReadAsPacked() { return PackedGuidReader(*this); }

            void Set(uint64_t guid) { _data._guid = guid; }
            void Clear() { _data._guid = 0; }

            PackedGuid WriteAsPacked() const;

            uint64_t   GetRawValue() const { return _data._guid; }
            HighGuid GetHigh() const
            {
                HighGuid temp = static_cast<HighGuid>(uint32_t((_data._guid >> 48) & 0x0000FFFF));
                return HighGuid((temp == HighGuid::Corpse || temp == HighGuid::AreaTrigger) ? temp : HighGuid(((uint32_t)temp >> 4) & 0x00000FFF));
            }
            uint32_t   GetEntry() const { return HasEntry() ? uint64_t((_data._guid >> 32) & 0x00000000000FFFFFuLL) : 0; }
            uint32_t   GetCounter()  const
            {
                return uint32_t(_data._guid & 0x00000000FFFFFFFFuLL);
            }

            static LowType GetMaxCounter(HighGuid /*high*/)
            {
                return LowType(0xFFFFFFFF);
            }

            ObjectGuid::LowType GetMaxCounter() const { return GetMaxCounter(GetHigh()); }

            uint8_t& operator [] (uint32_t index);
            uint8_t const& operator [] (uint32_t index) const;

            bool IsEmpty()             const { return _data._guid == 0; }
            bool IsCreature()          const { return GetHigh() == HighGuid::Unit; }
            bool IsPet()               const { return GetHigh() == HighGuid::Pet; }
            bool IsVehicle()           const { return GetHigh() == HighGuid::Vehicle; }
            bool IsCreatureOrPet()     const { return IsCreature() || IsPet(); }
            bool IsCreatureOrVehicle() const { return IsCreature() || IsVehicle(); }
            bool IsAnyTypeCreature()   const { return IsCreature() || IsPet() || IsVehicle(); }
            bool IsPlayer()            const { return !IsEmpty() && GetHigh() == HighGuid::Player; }
            bool IsUnit()              const { return IsAnyTypeCreature() || IsPlayer(); }
            bool IsItem()              const { return GetHigh() == HighGuid::Item; }
            bool IsGameObject()        const { return GetHigh() == HighGuid::GameObject; }
            bool IsDynamicObject()     const { return GetHigh() == HighGuid::DynamicObject; }
            bool IsCorpse()            const { return GetHigh() == HighGuid::Corpse; }
            bool IsAreaTrigger()       const { return GetHigh() == HighGuid::AreaTrigger; }
            bool IsBattleground()      const { return GetHigh() == HighGuid::BattleGround; }
            bool IsTransport()         const { return GetHigh() == HighGuid::Transport; }
            bool IsMOTransport()       const { return GetHigh() == HighGuid::Mo_Transport; }
            bool IsAnyTypeGameObject() const { return IsGameObject() || IsTransport() || IsMOTransport(); }
            bool IsInstance()          const { return GetHigh() == HighGuid::Instance; }
            bool IsGroup()             const { return GetHigh() == HighGuid::Group; }
            bool IsGuild()             const { return GetHigh() == HighGuid::Guild; }

            static TypeID GetTypeId(HighGuid high)
            {
                switch (high)
                {
                    case HighGuid::Item:          return TYPEID_ITEM;
                    //case HighGuid::Container:     return TYPEID_CONTAINER; HighGuid::Container==HighGuid::Item currently
                    case HighGuid::Unit:          return TYPEID_UNIT;
                    case HighGuid::Pet:           return TYPEID_UNIT;
                    case HighGuid::Player:        return TYPEID_PLAYER;
                    case HighGuid::GameObject:    return TYPEID_GAMEOBJECT;
                    case HighGuid::DynamicObject: return TYPEID_DYNAMICOBJECT;
                    case HighGuid::Corpse:        return TYPEID_CORPSE;
                    case HighGuid::AreaTrigger:   return TYPEID_AREATRIGGER;
                    case HighGuid::Mo_Transport:  return TYPEID_GAMEOBJECT;
                    case HighGuid::Vehicle:       return TYPEID_UNIT;
                    // unknown
                    case HighGuid::Instance:
                    case HighGuid::BattleGround:
                    case HighGuid::Group:
                    case HighGuid::Guild:
                    default:                      return TYPEID_OBJECT;
                }
            }

            TypeID GetTypeId() const { return GetTypeId(GetHigh()); }

            bool operator!() const { return IsEmpty(); }
            bool operator==(ObjectGuid const& guid) const { return GetRawValue() == guid.GetRawValue(); }
            bool operator!=(ObjectGuid const& guid) const { return GetRawValue() != guid.GetRawValue(); }
            bool operator<(ObjectGuid const& guid) const { return GetRawValue() < guid.GetRawValue(); }

            static char const* GetTypeName(HighGuid high);
            char const* GetTypeName() const { return !IsEmpty() ? GetTypeName(GetHigh()) : "None"; }
            std::string ToString() const;

        private:
            static bool HasEntry(HighGuid high)
            {
                switch (high)
                {
                    case HighGuid::Item:
                    case HighGuid::Player:
                    case HighGuid::DynamicObject:
                    case HighGuid::Corpse:
                    case HighGuid::Mo_Transport:
                    case HighGuid::Instance:
                    case HighGuid::Group:
                        return false;
                    case HighGuid::GameObject:
                    case HighGuid::Transport:
                    case HighGuid::Unit:
                    case HighGuid::Pet:
                    case HighGuid::Vehicle:
                    default:
                        return true;
                }
            }

            bool HasEntry() const { return HasEntry(GetHigh()); }

            explicit ObjectGuid(uint32_t const&) = delete;

            union
            {
                uint64_t _guid;
                uint8_t _bytes[sizeof(uint64_t)];
            } _data;
    };

    using namespace wowgm::protocol;

    class PackedGuid
    {
            static const constexpr int  PACKED_GUID_MIN_BUFFER_SIZE = 9;

            friend ByteBuffer& operator << (ByteBuffer& buf, PackedGuid const& guid);

        public:
            explicit PackedGuid() : _packedGuid(PACKED_GUID_MIN_BUFFER_SIZE) { _packedGuid.appendPackGUID(0); }
            explicit PackedGuid(uint64_t guid) : _packedGuid(PACKED_GUID_MIN_BUFFER_SIZE) { _packedGuid.appendPackGUID(guid); }
            explicit PackedGuid(ObjectGuid guid) : _packedGuid(PACKED_GUID_MIN_BUFFER_SIZE) { _packedGuid.appendPackGUID(guid.GetRawValue()); }

            void Set(uint64_t guid) { _packedGuid.wpos(0); _packedGuid.appendPackGUID(guid); }
            void Set(ObjectGuid guid) { _packedGuid.wpos(0); _packedGuid.appendPackGUID(guid.GetRawValue()); }

            std::size_t size() const { return _packedGuid.size(); }

        private:
            ByteBuffer _packedGuid;
    };

    inline PackedGuid ObjectGuid::WriteAsPacked() const { return PackedGuid(*this); }
}

namespace std
{
    using namespace wowgm::game::structures;

    template <>
    struct hash<ObjectGuid>
    {
        std::size_t operator()(ObjectGuid const& s) const noexcept
        {
            return std::hash<uint64_t>()(s.GetRawValue());
        }
    };
}

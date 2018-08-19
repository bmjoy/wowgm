#include "CGPlayer.hpp"

namespace wowgm::game::entities
{
    using namespace wowgm::game::structures;

    CGPlayer::CGPlayer(CClientObjCreate const& createBlock) : CGUnit(createBlock)
    {

    }

    CGPlayer::~CGPlayer()
    {

    }

    CGPlayer* CGPlayer::ToPlayer()
    {
        return this;
    }

    CGPlayer const* CGPlayer::ToPlayer() const
    {
        return this;
    }

    CGPlayerData const& CGPlayer::GetPlayerData() const
    {
        return static_cast<CGPlayerData const&>(*this);
    }

    CGPlayerData& CGPlayer::GetPlayerData()
    {
        return static_cast<CGPlayerData&>(*this);
    }

    void CGPlayer::UpdateDescriptors(JamCliValuesUpdate const& valuesUpdate)
    {
        CGUnit::UpdateDescriptors(valuesUpdate);

        std::uint8_t* unitDataBase = reinterpret_cast<std::uint8_t*>(&GetPlayerData());
        for (auto&& itr : valuesUpdate.Descriptors)
        {
            auto offset = itr.first * 4;
            if (offset <= sizeof(CGObjectData) + sizeof(CGUnitData))
                continue;

            offset -= sizeof(CGObjectData) + sizeof(CGUnitData);
            if (offset > sizeof(CGPlayerData))
                continue;

            *reinterpret_cast<std::uint32_t*>(unitDataBase + offset) = itr.second;
        }
    }
}

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

    uint32_t CGPlayer::UpdateDescriptors(JamCliValuesUpdate& valuesUpdate)
    {
        uint32_t startOffset = CGUnit::UpdateDescriptors(valuesUpdate);

        uint8_t* unitDatabase = reinterpret_cast<uint8_t*>(&GetPlayerData());

        auto itr = valuesUpdate.Descriptors.begin();
        while (itr != valuesUpdate.Descriptors.end())
        {
            uint32_t calculatedOffset = itr->first * 4 - startOffset;
            if (calculatedOffset > sizeof(CGPlayerData))
            {
                ++itr;
                continue;
            }

            *reinterpret_cast<uint32_t*>(unitDatabase + calculatedOffset) = itr->second;
            itr = valuesUpdate.Descriptors.erase(itr);
        }

        return startOffset + sizeof(CGPlayerData);
    }
}

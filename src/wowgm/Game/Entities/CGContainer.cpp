#include "CGContainer.hpp"
#include "ObjectMgr.hpp"

namespace wowgm::game::entities
{
    using namespace wowgm::game::structures;

    CGContainer::CGContainer(CClientObjCreate const& objCreate) : CGContainerData(), CGItem(objCreate)
    {

    }

    CGContainer::~CGContainer()
    {

    }

    CGContainerData const& CGContainer::GetContainerData() const
    {
        return static_cast<CGContainerData const&>(*this);
    }

    CGContainerData& CGContainer::GetContainerData()
    {
        return static_cast<CGContainerData&>(*this);
    }

    CGItem* CGContainer::ToItem()
    {
        return static_cast<CGItem*>(this);
    }

    CGItem const* CGContainer::ToItem() const
    {
        return static_cast<CGItem const*>(this);
    }

    uint32_t CGContainer::UpdateDescriptors(JamCliValuesUpdate& valuesUpdate)
    {
        uint32_t startOffset = CGItem::UpdateDescriptors(valuesUpdate);
        uint8_t* itemDatabase = reinterpret_cast<uint8_t*>(&GetContainerData());

        auto itr = valuesUpdate.Descriptors.begin();
        while (itr != valuesUpdate.Descriptors.end())
        {
            uint32_t calculatedOffset = itr->first * 4 - startOffset;
            if (calculatedOffset > sizeof(CGContainerData))
            {
                ++itr;
                continue;
            }

            *reinterpret_cast<uint32_t*>(itemDatabase + calculatedOffset) = itr->second;
            itr = valuesUpdate.Descriptors.erase(itr);
        }

        return startOffset + sizeof(CGContainerData);
    }
}

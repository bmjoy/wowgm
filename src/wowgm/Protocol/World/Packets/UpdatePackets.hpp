#pragma once

#include "Packet.hpp"
#include "WorldPacket.hpp"
#include "ObjectGuid.hpp"

#include <vector>
#include <boost/optional.hpp>

using namespace wowgm::utilities;

namespace wowgm::protocol::world::packets
{
    enum class UpdateType : std::uint8_t
    {
        Values         = 0,
        CreateObject1  = 1,
        CreateObject2  = 2,
        DestroyObjects = 3
    };

    struct ClientUpdateObject final : public ServerPacket
    {
        struct SplineInfo
        {
            enum class SplineType : std::uint8_t
            {
                FacingAngle    = 0,
                FacingPosition = 1,
                FacingTarget   = 2,
                Normal         = 3
            };

            std::uint8_t Mode;
            std::uint32_t StartTime;
            ObjectGuid Target;
            std::uint32_t Flags;
            SplineType Type;

            float Elevation;
            float VerticalAcceleration;
            std::uint32_t Time;
            float FacingAngle;

            std::uint32_t ID;
        };

        struct FallInfo
        {
            float Cosine;
            float Sine;
            float HorizontalSpeed;
            std::uint32_t FallTime;
            float VerticalSpeed;
        };

        struct LivingBlock
        {
            boost::optional<std::uint32_t> MovementFlags;
            boost::optional<std::uint32_t> MovementFlagsExtra;

            boost::optional<float> Orientation;

            boost::optional<SplineInfo> Spline;
            FallInfo Falling;

            ObjectGuid TransportGUID;
            std::array<std::uint32_t, 3> TransportTime;
            struct {
                float X, Y, Z, O;
            } TransportOffset;
            std::uint8_t VehicleSeatID;
            std::uint32_t TransportPathTimer;
        };

        struct AnimKitBlock
        {
            boost::optional<std::uint32_t> AIAnimKit, MovementAnimKit, MeleeAnimKit;
        };

        struct CreateObjectBlock
        {
            ObjectGuid GUID;
            std::uint8_t ObjectTypeID;

            boost::optional<LivingBlock> Living;
            boost::optional<ObjectGuid> TargetGUID;

            std::vector<uint32_t> StopFrames;

            float RunBackSpeed;
            float SwimBackSpeed;
            float PitchSpeed;
            float SwimSpeed;
            float WalkSpeed;
            float FlyBackSpeed;
            float TurnSpeed;
            float RunSpeed;
            float FlySpeed;

            float Pitch;

            std::uint32_t Time;

            struct {
                float X, Y, Z;
            } Position;
            float Orientation;

            float VehicleOrientation;
            std::uint32_t VehicleID;

            std::uint16_t AiAnimKit;
            std::uint16_t MovementAnimKit;
            std::uint16_t MeleeAnimKit;
        };

        ClientUpdateObject(WorldPacket&& packet);

        void Read() override;

        std::uint16_t MapID = 0;

        std::vector<ObjectGuid> OutOfRange;

    private:
        void ReadValuesUpdateBlock();
        void ReadMovementUpdateBlock(CreateObjectBlock& block);
        void ReadCreateObjectBlock();
        void ReadOutOfRangeObjects();
    };
}

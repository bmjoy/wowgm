#include "UpdatePackets.hpp"
#include "PacketUtils.hpp"
#include <shared/assert/assert.hpp>
#if _DEBUG

#endif

namespace wowgm::protocol::world::packets
{
    using namespace wowgm::game::structures;

    inline WorldPacket& operator >> (WorldPacket& worldPacket, CMovementStatus& movementInfo)
    {
        worldPacket >> movementInfo.ObjectType;
        movementInfo.PlayHoverAnim = worldPacket.ReadBit();
        movementInfo.IsSuppressingGreetings = worldPacket.ReadBit();
        bool hasGameObjectRotation = worldPacket.ReadBit();
        bool hasAnimKits = worldPacket.ReadBit();
        bool hasAttackingTarget = worldPacket.ReadBit();
        movementInfo.ThisIsYou = worldPacket.ReadBit();
        bool hasVehicleData = worldPacket.ReadBit();
        bool isLiving = worldPacket.ReadBit();
        movementInfo.StopFrames.resize(worldPacket.ReadBits(24));
        movementInfo.NoBirthAnim = worldPacket.ReadBit();
        bool hasGameObjectPosition = worldPacket.ReadBit();
        bool hasStationaryPosition = worldPacket.ReadBit();
        bool unkBit456 = worldPacket.ReadBit();
        bool someBitRelatedToDoors = worldPacket.ReadBit(); // GameObject_Type_Door is the only one to use this.
                                                            // enablePortals?
        bool hasTransport = worldPacket.ReadBit();

        bool hasOrientation = false;
        bool hasPitch = false;
        bool hasFallData = false;
        bool hasSplineElevation = false;
        bool hasTransportData = false;
        bool hasTimestamp = false;
        bool hasTransportTime[] = { false, false };
        bool hasExtendedSplineData = false;
        bool hasSplineStartTime = false;
        bool hasSplineVerticalAcceleration = false;
        bool hasFallDirection = false;
        bool hasGameobjectTransportTime[] = { false, false };
        bool hasAnimKit[] = { false, false, false };
        bool hasSplineData = false;

        if (isLiving)
        {
            bool hasMovementFlags = !worldPacket.ReadBit();
            hasOrientation = !worldPacket.ReadBit();

            movementInfo.GUID[7] = worldPacket.ReadBit();
            movementInfo.GUID[3] = worldPacket.ReadBit();
            movementInfo.GUID[2] = worldPacket.ReadBit();

            if (hasMovementFlags)
                movementInfo.Flags = worldPacket.ReadBits(30);

            bool hasMovementInfoSpline = worldPacket.ReadBit();
            hasPitch = !worldPacket.ReadBit();
            hasSplineData = worldPacket.ReadBit();
            hasFallData = worldPacket.ReadBit();
            hasSplineElevation = !worldPacket.ReadBit();

            movementInfo.GUID[5] = worldPacket.ReadBit();

            hasTransportData = worldPacket.ReadBit();
            hasTimestamp = !worldPacket.ReadBit();

            if (hasTransportData)
            {
                movementInfo.Transport.GUID[1] = worldPacket.ReadBit();
                hasTransportTime[0] = worldPacket.ReadBit();
                movementInfo.Transport.GUID[4] = worldPacket.ReadBit();
                movementInfo.Transport.GUID[0] = worldPacket.ReadBit();
                movementInfo.Transport.GUID[6] = worldPacket.ReadBit();
                hasTransportTime[1] = worldPacket.ReadBit();;
                movementInfo.Transport.GUID[7] = worldPacket.ReadBit();
                movementInfo.Transport.GUID[5] = worldPacket.ReadBit();
                movementInfo.Transport.GUID[3] = worldPacket.ReadBit();
                movementInfo.Transport.GUID[2] = worldPacket.ReadBit();
            }

            movementInfo.GUID[4] = worldPacket.ReadBit();

            if (hasSplineData)
            {
                hasExtendedSplineData = worldPacket.ReadBit();
                if (hasExtendedSplineData)
                {
                    movementInfo.Spline.Mode = worldPacket.ReadBits(2);
                    hasSplineStartTime = worldPacket.ReadBit();
                    movementInfo.Spline.Points.resize(worldPacket.ReadBits(22));
                    movementInfo.Spline.Type = worldPacket.ReadBits(2);

                    if (movementInfo.Spline.Type == 2)
                    {
                        movementInfo.Spline.Facing.Target[4] = worldPacket.ReadBit();
                        movementInfo.Spline.Facing.Target[3] = worldPacket.ReadBit();
                        movementInfo.Spline.Facing.Target[7] = worldPacket.ReadBit();
                        movementInfo.Spline.Facing.Target[2] = worldPacket.ReadBit();
                        movementInfo.Spline.Facing.Target[6] = worldPacket.ReadBit();
                        movementInfo.Spline.Facing.Target[1] = worldPacket.ReadBit();
                        movementInfo.Spline.Facing.Target[0] = worldPacket.ReadBit();
                        movementInfo.Spline.Facing.Target[5] = worldPacket.ReadBit();
                    }

                    hasSplineVerticalAcceleration = worldPacket.ReadBit();
                    movementInfo.Spline.Flags = worldPacket.ReadBits(25);
                }
            }

            movementInfo.GUID[6] = worldPacket.ReadBit();

            if (hasFallData)
                hasFallDirection = worldPacket.ReadBit();

            movementInfo.GUID[0] = worldPacket.ReadBit();
            movementInfo.GUID[1] = worldPacket.ReadBit();
            worldPacket.ReadBit();
            if (!worldPacket.ReadBit())
                movementInfo.FlagsExtra = worldPacket.ReadBits(12);
        }

        if (hasGameObjectPosition)
        {
            movementInfo.Transport.GameObject.GUID[5] = worldPacket.ReadBit();
            hasGameobjectTransportTime[0] = worldPacket.ReadBit();
            movementInfo.Transport.GameObject.GUID[0] = worldPacket.ReadBit();
            movementInfo.Transport.GameObject.GUID[3] = worldPacket.ReadBit();
            movementInfo.Transport.GameObject.GUID[6] = worldPacket.ReadBit();
            movementInfo.Transport.GameObject.GUID[1] = worldPacket.ReadBit();
            movementInfo.Transport.GameObject.GUID[4] = worldPacket.ReadBit();
            movementInfo.Transport.GameObject.GUID[2] = worldPacket.ReadBit();
            hasGameobjectTransportTime[1] = worldPacket.ReadBit();
            movementInfo.Transport.GameObject.GUID[7] = worldPacket.ReadBit();
        }

        if (hasAttackingTarget)
        {
            movementInfo.TargetGUID[2] = worldPacket.ReadBit();
            movementInfo.TargetGUID[7] = worldPacket.ReadBit();
            movementInfo.TargetGUID[0] = worldPacket.ReadBit();
            movementInfo.TargetGUID[4] = worldPacket.ReadBit();
            movementInfo.TargetGUID[5] = worldPacket.ReadBit();
            movementInfo.TargetGUID[6] = worldPacket.ReadBit();
            movementInfo.TargetGUID[1] = worldPacket.ReadBit();
            movementInfo.TargetGUID[3] = worldPacket.ReadBit();
        }

        if (hasAnimKits)
        {
            hasAnimKit[0] = worldPacket.ReadBit();
            hasAnimKit[1] = worldPacket.ReadBit();
            hasAnimKit[2] = worldPacket.ReadBit();
        }

        worldPacket.ResetBitPos();

        for (uint32_t& stopFrame : movementInfo.StopFrames)
            worldPacket >> stopFrame;

        if (isLiving)
        {
            worldPacket.ReadByteSeq(movementInfo.GUID[4]);
            worldPacket >> movementInfo.RunBackSpeed;
            if (hasFallData)
            {
                if (hasFallDirection)
                    worldPacket >> movementInfo.FallInfo.Jump.HorizontalSpeed >> movementInfo.FallInfo.Jump.Cosinus >> movementInfo.FallInfo.Jump.Sinus;

                worldPacket >> movementInfo.FallInfo.Time >> movementInfo.FallInfo.VerticalSpeed;
            }

            worldPacket >> movementInfo.SwimBackSpeed;
            if (hasSplineElevation)
                worldPacket >> movementInfo.Spline.Elevation;

            if (hasSplineData)
            {
                if (hasExtendedSplineData)
                {
                    if (hasSplineVerticalAcceleration)
                        worldPacket >> movementInfo.Spline.VerticalAcceleration;

                    worldPacket >> movementInfo.Spline.Time;
                    if (movementInfo.Spline.Type == 0) // Facing Angle
                        worldPacket >> movementInfo.Spline.Facing.Angle;
                    else if (movementInfo.Spline.Type == 2) // Facing Target
                    {
                        worldPacket.ReadByteSeq(movementInfo.Spline.Facing.Target[5]);
                        worldPacket.ReadByteSeq(movementInfo.Spline.Facing.Target[3]);
                        worldPacket.ReadByteSeq(movementInfo.Spline.Facing.Target[7]);
                        worldPacket.ReadByteSeq(movementInfo.Spline.Facing.Target[1]);
                        worldPacket.ReadByteSeq(movementInfo.Spline.Facing.Target[6]);
                        worldPacket.ReadByteSeq(movementInfo.Spline.Facing.Target[4]);
                        worldPacket.ReadByteSeq(movementInfo.Spline.Facing.Target[2]);
                        worldPacket.ReadByteSeq(movementInfo.Spline.Facing.Target[0]);
                    }

                    for (C3Vector& itr : movementInfo.Spline.Points)
                        worldPacket >> itr.Z >> itr.X >> itr.Y;

                    if (movementInfo.Spline.Type == 1) // Facing Spot
                        worldPacket >> movementInfo.Spline.Facing.Position.X >> movementInfo.Spline.Facing.Position.Z >> movementInfo.Spline.Facing.Position.Y;

                    worldPacket >> movementInfo.Spline.NextDurationMultiplier;
                    worldPacket >> movementInfo.Spline.Duration;
                    if (hasSplineStartTime)
                        worldPacket >> movementInfo.Spline.StartTime;
                    worldPacket >> movementInfo.Spline.DurationMultiplier;
                }

                worldPacket >> movementInfo.Spline.Endpoint.Z;
                worldPacket >> movementInfo.Spline.Endpoint.X;
                worldPacket >> movementInfo.Spline.Endpoint.Y;
                worldPacket >> movementInfo.Spline.ID;
            }

            worldPacket >> movementInfo.Position.Z;
            worldPacket.ReadByteSeq(movementInfo.GUID[5]);

            if (hasTransportData)
            {
                worldPacket.ReadByteSeq(movementInfo.Transport.GUID[5]);
                worldPacket.ReadByteSeq(movementInfo.Transport.GUID[7]);
                worldPacket >> movementInfo.Transport.Times[0];
                worldPacket >> movementInfo.Transport.Offset.O;

                if (hasTransportTime[0])
                    worldPacket >> movementInfo.Transport.Times[1];

                worldPacket >> movementInfo.Transport.Offset.Y;
                worldPacket >> movementInfo.Transport.Offset.X;
                worldPacket.ReadByteSeq(movementInfo.Transport.GUID[3]);
                worldPacket >> movementInfo.Transport.Offset.Z;
                worldPacket.ReadByteSeq(movementInfo.Transport.GUID[0]);

                if (hasTransportTime[1])
                    worldPacket >> movementInfo.Transport.Times[2];

                worldPacket >> movementInfo.Transport.Seat;
                worldPacket.ReadByteSeq(movementInfo.Transport.GUID[1]);
                worldPacket.ReadByteSeq(movementInfo.Transport.GUID[6]);
                worldPacket.ReadByteSeq(movementInfo.Transport.GUID[2]);
                worldPacket.ReadByteSeq(movementInfo.Transport.GUID[4]);
            }

            worldPacket >> movementInfo.Position.X;
            worldPacket >> movementInfo.PitchSpeed;
            worldPacket.ReadByteSeq(movementInfo.GUID[3]);
            worldPacket.ReadByteSeq(movementInfo.GUID[0]);
            worldPacket >> movementInfo.SwimSpeed;
            worldPacket >> movementInfo.Position.Y;
            worldPacket.ReadByteSeq(movementInfo.GUID[7]);
            worldPacket.ReadByteSeq(movementInfo.GUID[1]);
            worldPacket.ReadByteSeq(movementInfo.GUID[2]);
            worldPacket >> movementInfo.WalkSpeed;
            if (hasTimestamp)
                worldPacket >> movementInfo.Time;
            worldPacket >> movementInfo.FlyBackSpeed;
            worldPacket.ReadByteSeq(movementInfo.GUID[6]);
            worldPacket >> movementInfo.TurnSpeed;
            if (hasOrientation)
                worldPacket >> movementInfo.Position.O;
            worldPacket >> movementInfo.RunSpeed;
            if (hasPitch)
                worldPacket >> movementInfo.Pitch;
            worldPacket >> movementInfo.FlySpeed;
        }

        if (hasVehicleData)
        {
            worldPacket >> movementInfo.Vehicle.O;
            worldPacket >> movementInfo.Vehicle.ID;
        }

        if (hasGameObjectPosition)
        {
            worldPacket.ReadByteSeq(movementInfo.Transport.GameObject.GUID[0]);
            worldPacket.ReadByteSeq(movementInfo.Transport.GameObject.GUID[5]);
            if (hasGameobjectTransportTime[1])
                worldPacket >> movementInfo.Transport.GameObject.Times[2];
            worldPacket.ReadByteSeq(movementInfo.Transport.GameObject.GUID[3]);
            worldPacket >> movementInfo.Transport.GameObject.Offset.X;
            worldPacket.ReadByteSeq(movementInfo.Transport.GameObject.GUID[4]);
            worldPacket.ReadByteSeq(movementInfo.Transport.GameObject.GUID[6]);
            worldPacket.ReadByteSeq(movementInfo.Transport.GameObject.GUID[1]);
            worldPacket >> movementInfo.Transport.GameObject.Times[0];
            worldPacket >> movementInfo.Transport.GameObject.Offset.Y;
            worldPacket.ReadByteSeq(movementInfo.Transport.GameObject.GUID[2]);
            worldPacket.ReadByteSeq(movementInfo.Transport.GameObject.GUID[7]);
            worldPacket >> movementInfo.Transport.GameObject.Offset.Z;
            worldPacket >> movementInfo.Transport.GameObject.Seat;
            worldPacket >> movementInfo.Transport.GameObject.Offset.O;
            if (hasGameobjectTransportTime[0])
                worldPacket >> movementInfo.Transport.GameObject.Times[1];
        }

        if (hasGameObjectRotation)
        {
            movementInfo.Transport.GameObject.Rotation = worldPacket.read<uint64_t>(); // Packed quaternion
        }

        if (unkBit456)
        {
            // ...
        }

        if (hasStationaryPosition)
        {
            worldPacket >> movementInfo.Stationary.O;
            worldPacket >> movementInfo.Stationary.X;
            worldPacket >> movementInfo.Stationary.Y;
            worldPacket >> movementInfo.Stationary.Z;
        }

        if (hasAttackingTarget)
        {
            worldPacket.ReadByteSeq(movementInfo.TargetGUID[4]);
            worldPacket.ReadByteSeq(movementInfo.TargetGUID[0]);
            worldPacket.ReadByteSeq(movementInfo.TargetGUID[3]);
            worldPacket.ReadByteSeq(movementInfo.TargetGUID[5]);
            worldPacket.ReadByteSeq(movementInfo.TargetGUID[7]);
            worldPacket.ReadByteSeq(movementInfo.TargetGUID[6]);
            worldPacket.ReadByteSeq(movementInfo.TargetGUID[2]);
            worldPacket.ReadByteSeq(movementInfo.TargetGUID[1]);
        }

        if (hasAnimKits)
        {
            if (hasAnimKit[0])
                worldPacket >> movementInfo.AnimKits[0];
            if (hasAnimKit[1])
                worldPacket >> movementInfo.AnimKits[1];
            if (hasAnimKit[2])
                worldPacket >> movementInfo.AnimKits[2];
        }

        if (hasTransport)
            worldPacket >> movementInfo.Transport.PathTimer;

        return worldPacket;
    }

    inline WorldPacket& operator >> (WorldPacket& worldPacket, JamCliValuesUpdate& valuesUpdate)
    {
        uint8_t blockCount;
        worldPacket >> blockCount;

        std::vector<bool> mask(blockCount * 32);
        for (uint32_t i = 0; i < mask.size(); ++i)
            mask[i] = worldPacket.ReadBit();

        uint32_t i = 0;
        for (bool currentBit : mask)
        {
            if (currentBit)
            {
                uint32_t updateField;
                worldPacket >> updateField;

                if (updateField != 0)
                    valuesUpdate.Descriptors[i] = updateField;
            }

            ++i;
        }

        return worldPacket;
    }

    ClientUpdateObject::ClientUpdateObject(WorldPacket&& packet) : ServerPacket(std::move(packet))
    {

    }

    void ClientUpdateObject::Read()
    {
        _worldPacket >> MapID;
        Updates.resize(_worldPacket.read<uint32_t>());

        for (CClientObjCreate& objCreate : Updates)
        {
            _worldPacket >> objCreate.UpdateType;

            switch (objCreate.UpdateType)
            {
                case UpdateType::DestroyObjects:
                {
                    uint32_t objectCount;
                    _worldPacket >> objectCount;
                    for (uint32_t i = 0; i < objectCount; ++i)
                    {
                        ObjectGuid destroyGuid;
                        _worldPacket.ReadPackedGuid(destroyGuid);
                        DestroyObjects.push_back(destroyGuid);
                    }

                    break;
                }
                case UpdateType::CreateObject1:
                case UpdateType::CreateObject2:
                {
                    _worldPacket.ReadPackedGuid(objCreate.GUID);
                    _worldPacket >> objCreate.Movement;
                    _worldPacket >> objCreate.Values;
                    break;
                }
                case UpdateType::Values:
                {
                    _worldPacket.ReadPackedGuid(objCreate.GUID);
                    _worldPacket >> objCreate.Values;

                    break;
                }
                default:
                    BOOST_ASSERT_MSG_FMT(false, "Unknown update type %u", uint32_t(objCreate.UpdateType));
                    break;
            }
        }
    }


    ClientDestroyObject::ClientDestroyObject(WorldPacket&& packet) : ServerPacket(std::move(packet))
    {

    }

    void ClientDestroyObject::Read()
    {
        uint64_t guid;
        _worldPacket >> guid;

        OnDeath = _worldPacket.read<uint8_t>() != 0;
        GUID.Set(guid);
    }
}

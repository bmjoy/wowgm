#include "UpdatePackets.hpp"

#include <boost/utility/in_place_factory.hpp>

namespace wowgm::protocol::world::packets
{
    ClientUpdateObject::ClientUpdateObject(WorldPacket&& packet) : ServerPacket(std::move(packet))
    {

    }

    void ClientUpdateObject::Read()
    {
        _worldPacket >> MapID;

        std::uint32_t blockCount;
        _worldPacket >> blockCount;

        for (std::uint32_t i = 0; i < blockCount; ++i)
        {
            std::uint8_t blockType;
            _worldPacket >> blockType;

            switch (UpdateType(blockType))
            {
                case UpdateType::DestroyObjects:
                    ReadOutOfRangeObjects();
                    break;
                case UpdateType::CreateObject1:
                case UpdateType::CreateObject2:
                    ReadCreateObjectBlock();
                    break;
                case UpdateType::Values:
                    // ReadValuesUpdateBlock();
                    break;
            }
        }
    }

    void ClientUpdateObject::ReadOutOfRangeObjects()
    {
        std::uint32_t outOfRangeCount;
        _worldPacket >> outOfRangeCount;

        OutOfRange.resize(outOfRangeCount);

        for (auto&& itr : OutOfRange)
            _worldPacket.ReadPackedGuid(itr);
    }

    void ClientUpdateObject::ReadCreateObjectBlock()
    {
        CreateObjectBlock block;
        _worldPacket >> block.ObjectTypeID;
        ReadMovementUpdateBlock(block);
    }

    void ClientUpdateObject::ReadMovementUpdateBlock(CreateObjectBlock& block)
    {
        /*_worldPacket.ReadBit();
        _worldPacket.ReadBit();
        bool hasGameobjectRotation = _worldPacket.ReadBit();
        bool hasAnimKits           = _worldPacket.ReadBit();
        bool hasAttackingTarget    = _worldPacket.ReadBit();
        _worldPacket.ReadBit(); // Is self
        bool hasVehicleData        = _worldPacket.ReadBit();
        bool living                = _worldPacket.ReadBit();
        block.StopFrames.resize(_worldPacket.ReadBits(24));
        _worldPacket.ReadBit();
        bool hasGameobjectPosition = _worldPacket.ReadBit();
        bool hasStationaryPosition = _worldPacket.ReadBit();
        _worldPacket.ReadBit(); // 456
        _worldPacket.ReadBit();
        bool transport = _worldPacket.ReadBit();

        bool hasOrientation = false;
        bool hasFallData = false;
        bool hasSplineElevation = false;
        bool hasFallDirection = false;
        bool hasAiAnimKit = false;
        bool hasMeleeAnimKit = false;
        bool hasMovementAnimKit = false;
        bool hasSplineData = false;
        bool hasExtendedSplineData = false;
        bool hasSplineVerticalAcceleration = false;
        bool hasTransportData = false;
        bool hasTimestamp = false;

        if (living)
        {
            bool hasMovementFlags = !_worldPacket.ReadBit();
            hasOrientation = !_worldPacket.ReadBit();

            block.GUID[7] = _worldPacket.ReadBit();
            block.GUID[3] = _worldPacket.ReadBit();
            block.GUID[2] = _worldPacket.ReadBit();

            if (hasMovementFlags)
                block.Living->MovementFlags = _worldPacket.ReadBits(30);

            bool hasMovementInfoSpline = _worldPacket.ReadBit();
            bool hasPitch              = !_worldPacket.ReadBit();
            hasSplineData         = _worldPacket.ReadBit();
            hasFallData           = _worldPacket.ReadBit();
            hasSplineElevation    = !_worldPacket.ReadBit();
            block.GUID[5] = _worldPacket.ReadBit();
            hasTransportData      = _worldPacket.ReadBit();
            hasTimestamp          = !_worldPacket.ReadBit();

            bool hasTransportTime2 = false;
            bool hasTransportVehicleSeatId = false;
            if (hasTransportData)
            {
                block.Living->TransportGUID[1] = _worldPacket.ReadBit();
                hasTransportTime2 = _worldPacket.ReadBit();
                block.Living->TransportGUID[4] = _worldPacket.ReadBit();
                block.Living->TransportGUID[0] = _worldPacket.ReadBit();
                block.Living->TransportGUID[6] = _worldPacket.ReadBit();
                hasTransportVehicleSeatId = _worldPacket.ReadBit();
                block.Living->TransportGUID[7] = _worldPacket.ReadBit();
                block.Living->TransportGUID[5] = _worldPacket.ReadBit();
                block.Living->TransportGUID[3] = _worldPacket.ReadBit();
                block.Living->TransportGUID[2] = _worldPacket.ReadBit();
            }

            block.GUID[4] = _worldPacket.ReadBit();
            if (hasSplineData)
            {
                block.Living->Spline = boost::in_place();

                hasExtendedSplineData = _worldPacket.ReadBit();
                if (hasExtendedSplineData)
                {
                    block.Living->Spline->Mode = _worldPacket.ReadBits(2);
                    bool hasSplineStartTime = _worldPacket.ReadBit();
                    bool splinePointCount = _worldPacket.ReadBit();
                    block.Living->Spline->Type = SplineInfo::SplineType(_worldPacket.ReadBits(2));

                    if (block.Living->Spline->Type == SplineInfo::SplineType::FacingTarget)
                    {
                        block.Living->Spline->Target[4] = _worldPacket.ReadBit();
                        block.Living->Spline->Target[3] = _worldPacket.ReadBit();
                        block.Living->Spline->Target[7] = _worldPacket.ReadBit();
                        block.Living->Spline->Target[2] = _worldPacket.ReadBit();
                        block.Living->Spline->Target[6] = _worldPacket.ReadBit();
                        block.Living->Spline->Target[1] = _worldPacket.ReadBit();
                        block.Living->Spline->Target[0] = _worldPacket.ReadBit();
                        block.Living->Spline->Target[3] = _worldPacket.ReadBit();
                    }

                    hasSplineVerticalAcceleration = _worldPacket.ReadBit();
                    block.Living->Spline->Flags = _worldPacket.ReadBits(25);
                }
            }

            block.GUID[6] = _worldPacket.ReadBit();
            if (hasFallData)
                hasFallDirection = _worldPacket.ReadBit();

            block.GUID[0] = _worldPacket.ReadBit();
            block.GUID[1] = _worldPacket.ReadBit();

            _worldPacket.ReadBit();
            if (!_worldPacket.ReadBit())
                block.Living->MovementFlagsExtra = _worldPacket.ReadBits(12);
        }

        if (hasGameobjectPosition)
        {
            block.Living->TransportGUID[5] = _worldPacket.ReadBit();
            bool hasGameobjectTransportTime3 = _worldPacket.ReadBit();
            block.Living->TransportGUID[0] = _worldPacket.ReadBit();
            block.Living->TransportGUID[3] = _worldPacket.ReadBit();
            block.Living->TransportGUID[6] = _worldPacket.ReadBit();
            block.Living->TransportGUID[1] = _worldPacket.ReadBit();
            block.Living->TransportGUID[4] = _worldPacket.ReadBit();
            block.Living->TransportGUID[2] = _worldPacket.ReadBit();
            bool hasGameobjectTransportTime2 = _worldPacket.ReadBit();
            block.Living->TransportGUID[7] = _worldPacket.ReadBit();
        }

        if (hasAttackingTarget)
        {
            block.TargetGUID[2] = _worldPacket.ReadBit();
            block.TargetGUID[7] = _worldPacket.ReadBit();
            block.TargetGUID[0] = _worldPacket.ReadBit();
            block.TargetGUID[4] = _worldPacket.ReadBit();
            block.TargetGUID[5] = _worldPacket.ReadBit();
            block.TargetGUID[6] = _worldPacket.ReadBit();
            block.TargetGUID[1] = _worldPacket.ReadBit();
            block.TargetGUID[3] = _worldPacket.ReadBit();
        }

        if (hasAnimKits)
        {
            hasAiAnimKit = !_worldPacket.ReadBit();
            hasMeleeAnimKit = !_worldPacket.ReadBit();
            hasMovementAnimKit = !_worldPacket.ReadBit();
        }

        _worldPacket.ResetBitPos();

        for (auto&& itr : block.StopFrames)
            _worldPacket >> itr;

        if (living)
        {
            _worldPacket.ReadByteSeq(block.GUID[4]);
            _worldPacket >> block.RunBackSpeed;
            if (hasFallData)
            {
                if (hasFallDirection)
                {
                    _worldPacket >> block.Living->Falling.HorizontalSpeed;
                    _worldPacket >> block.Living->Falling.Cosine;
                    _worldPacket >> block.Living->Falling.Sine;
                }

                _worldPacket >> block.Living->Falling.FallTime;
                _worldPacket >> block.Living->Falling.VerticalSpeed;
            }

            _worldPacket >> block.SwimBackSpeed;

            if (hasSplineElevation)
                _worldPacket >> block.Living->Spline->Elevation;

            if (hasSplineData)
            {
                if (hasExtendedSplineData)
                {
                    if (hasSplineVerticalAcceleration)
                        _worldPacket >> block.Living->Spline->VerticalAcceleration;

                    _worldPacket >> block.Living->Spline->Time;
                    if (block.Living->Spline->Type == SplineInfo::SplineType::FacingTarget)
                    {
                        _worldPacket.ReadByteSeq(block.Living->Spline->Target[5]);
                        _worldPacket.ReadByteSeq(block.Living->Spline->Target[3]);
                        _worldPacket.ReadByteSeq(block.Living->Spline->Target[7]);
                        _worldPacket.ReadByteSeq(block.Living->Spline->Target[1]);
                        _worldPacket.ReadByteSeq(block.Living->Spline->Target[6]);
                        _worldPacket.ReadByteSeq(block.Living->Spline->Target[4]);
                        _worldPacket.ReadByteSeq(block.Living->Spline->Target[2]);
                        _worldPacket.ReadByteSeq(block.Living->Spline->Target[0]);
                    }
                    else if (block.Living->Spline->Type == SplineInfo::SplineType::FacingAngle)
                    {
                        _worldPacket >> block.Living->Spline->FacingAngle;
                    }
                }

                _worldPacket >> block.Living->Spline->ID;
            }

            _worldPacket >> block.Position.Z;
            _worldPacket.ReadByteSeq(block.GUID[5]);

            if (hasTransportData)
            {
                _worldPacket.ReadByteSeq(block.Living->TransportGUID[5]);
                _worldPacket.ReadByteSeq(block.Living->TransportGUID[7]);
                _worldPacket >> block.Living->TransportTime[0];
                _worldPacket >> block.Living->TransportOffset.O;

                if (hasTransportTime2)
                    _worldPacket >> block.Living->TransportTime[1];

                _worldPacket >> block.Living->TransportOffset.Y;
                _worldPacket >> block.Living->TransportOffset.X;
                _worldPacket.ReadByteSeq(block.Living->TransportGUID[3]);
                _worldPacket >> block.Living->TransportOffset.Z;
                _worldPacket.ReadByteSeq(block.Living->TransportGUID[0]);

                if (hasTransportTime3)
                    _worldPacket >> block.Living->TransportTime[2];

                _worldPacket >> block.Living->VehicleSeatID;
                _worldPacket.ReadByteSeq(block.Living->TransportGUID[1]);
                _worldPacket.ReadByteSeq(block.Living->TransportGUID[6]);
                _worldPacket.ReadByteSeq(block.Living->TransportGUID[2]);
                _worldPacket.ReadByteSeq(block.Living->TransportGUID[4]);
            }

            _worldPacket >> block.Position.X;
            _worldPacket >> block.PitchSpeed;
            _worldPacket.ReadByteSeq(block.GUID[3]);
            _worldPacket.ReadByteSeq(block.GUID[0]);
            _worldPacket >> block.SwimSpeed;
            _worldPacket >> block.Position.Y;
            _worldPacket.ReadByteSeq(block.GUID[7]);
            _worldPacket.ReadByteSeq(block.GUID[1]);
            _worldPacket.ReadByteSeq(block.GUID[2]);

            _worldPacket >> block.WalkSpeed;

            if (hasTimestamp)
                _worldPacket >> block.Time;

            _worldPacket >> block.FlyBackSpeed;
            _worldPacket.ReadByteSeq(block.GUID[6]);
            _worldPacket >> block.TurnSpeed;

            if (hasOrientation)
                _worldPacket >> block.Orientation;

            _worldPacket >> block.RunSpeed;

            if (hasPitch)
                _worldPacket >> block.Pitch;
        }

        if (hasVehicleData)
        {
            _worldPacket >> block.VehicleOrientation;
            _worldPacket >> block.VehicleID;
        }

        if (hasGameobjectPosition)
        {
            _worldPacket.ReadByteSeq(block.Living->TransportGUID[0]);
            _worldPacket.ReadByteSeq(block.Living->TransportGUID[5]);
            if (hasGameobjectTransportTime3)
                _worldPacket >> block.Living->TransportTime[2];

            _worldPacket.ReadByteSeq(block.Living->TransportGUID[3]);
            _worldPacket >> block.Living->TransportOffset.X;
            _worldPacket.ReadByteSeq(block.Living->TransportGUID[4]);
            _worldPacket.ReadByteSeq(block.Living->TransportGUID[6]);
            _worldPacket.ReadByteSeq(block.Living->TransportGUID[1]);
            _worldPacket >> block.Living->TransportTime[0];
            _worldPacket >> block.Living->TransportOffset.Y;
            _worldPacket.ReadByteSeq(block.Living->TransportGUID[2]);
            _worldPacket.ReadByteSeq(block.Living->TransportGUID[7]);
            _worldPacket >> block.Living->TransportOffset.Z;
            _worldPacket >> block.Living->VehicleSeatID;
            _worldPacket >> block.Living->TransportOffset.O;
            if (hasGameobjectTransportTime2)
                _worldPacket >> block.Living->TransportTime[1];
        }

        if (hasGameobjectRotation)
        {
            // read as packed quat
        }

        if (bit456)
        {
             // Some parameters
        }

        if (hasStationaryPosition)
        {
            _worldPacket >> block.Orientation;
            _worldPacket >> block.Position.X;
            _worldPacket >> block.Position.Y;
            _worldPacket >> block.Position.Z;
        }

        if (hasAttackingTarget)
        {
            _worldPacket.ReadByteSeq(block.TargetGUID[4]);
            _worldPacket.ReadByteSeq(block.TargetGUID[0]);
            _worldPacket.ReadByteSeq(block.TargetGUID[3]);
            _worldPacket.ReadByteSeq(block.TargetGUID[5]);
            _worldPacket.ReadByteSeq(block.TargetGUID[7]);
            _worldPacket.ReadByteSeq(block.TargetGUID[6]);
            _worldPacket.ReadByteSeq(block.TargetGUID[2]);
            _worldPacket.ReadByteSeq(block.TargetGUID[1]);
        }

        if (hasAnimKits)
        {
            if (hasAiAnimKit)
                _worldPacket >> block.AiAnimKit;

            if (hasMovementAnimKit)
                _worldPacket >> block.MovementAnimKit;

            if (hasMeleeAnimKit)
                _worldPacket >> block.MeleeAnimKit;
        }

        if (hasTransportData)
        {
            _worldPacket >> block.Living->TransportPathTimer;
        }*/
    }
}

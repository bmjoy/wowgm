#include "UpdatePackets.hpp"

#include <boost/utility/in_place_factory.hpp>

namespace wowgm::protocol::world::packets
{
    ClientUpdateObject::ClientUpdateObject(WorldPacket&& packet) : ServerPacket(std::move(packet))
    {

    }

    void ClientUpdateObject::Read()
    {
        _worldPacket.ReadBit();
        _worldPacket.ReadBit();
        bool hasGameObjectRotation = _worldPacket.ReadBit();
        bool hasAnimKits = _worldPacket.ReadBit();
        bool hasAttackingTarget = _worldPacket.ReadBit();
        bool isSelf = _worldPacket.ReadBit();
        bool hasVehicleData = _worldPacket.ReadBit();
        bool isLiving = _worldPacket.ReadBit();
        StopFrames.resize(_worldPacket.ReadBits(24));
        InitializeActivePlayerComponent = _worldPacket.ReadBit();
        bool hasGameObjectPosition = _worldPacket.ReadBit();
        bool hasStationaryPosition = _worldPacket.ReadBit();
        bool unkBit456 = _worldPacket.ReadBit();
        bool someBitRelatedToDoors = _worldPacket.ReadBit(); // Not shitting you. GameObject_Type_Door is the only one to use this - This is absurd.
        bool hasTransport = _worldPacket.ReadBit();

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
            bool hasMovementFlags = !_worldPacket.ReadBit();
            bool hasOrientation = !_worldPacket.ReadBit();

            GUID[7] = _worldPacket.ReadBit();
            GUID[3] = _worldPacket.ReadBit();
            GUID[2] = _worldPacket.ReadBit();

            if (hasMovementFlags)
                Movement.Flags = _worldPacket.ReadBits(30);

            bool hasMovementInfoSpline = _worldPacket.ReadBit();
            hasPitch = !_worldPacket.ReadBit();
            hasSplineData = _worldPacket.ReadBit();
            hasFallData = _worldPacket.ReadBit();
            hasSplineElevation = !_worldPacket.ReadBit();

            GUID[5] = _worldPacket.ReadBit();

            hasTransportData = _worldPacket.ReadBit();
            hasTimestamp = _worldPacket.ReadBit();

            if (hasTransportData)
            {
                Movement.Transport.GUID[1] = _worldPacket.ReadBit();
                hasTransportTime[0] = _worldPacket.ReadBit();
                Movement.Transport.GUID[4] = _worldPacket.ReadBit();
                Movement.Transport.GUID[0] = _worldPacket.ReadBit();
                Movement.Transport.GUID[6] = _worldPacket.ReadBit();
                hasTransportTime[1] = _worldPacket.ReadBit();;
                Movement.Transport.GUID[7] = _worldPacket.ReadBit();
                Movement.Transport.GUID[5] = _worldPacket.ReadBit();
                Movement.Transport.GUID[3] = _worldPacket.ReadBit();
                Movement.Transport.GUID[2] = _worldPacket.ReadBit();
            }

            GUID[4] = _worldPacket.ReadBit();

            if (hasSplineData)
            {
                hasExtendedSplineData = _worldPacket.ReadBit();
                if (hasExtendedSplineData)
                {
                    Movement.Spline.Mode = _worldPacket.ReadBits(2);
                    hasSplineStartTime = _worldPacket.ReadBit();
                    Movement.Spline.Points.resize(_worldPacket.ReadBits(22));
                    Movement.Spline.Type = _worldPacket.ReadBits(2);

                    if (Movement.Spline.Type == 2)
                    {
                        Movement.Spline.Facing.Target[4] = _worldPacket.ReadBit();
                        Movement.Spline.Facing.Target[3] = _worldPacket.ReadBit();
                        Movement.Spline.Facing.Target[7] = _worldPacket.ReadBit();
                        Movement.Spline.Facing.Target[2] = _worldPacket.ReadBit();
                        Movement.Spline.Facing.Target[6] = _worldPacket.ReadBit();
                        Movement.Spline.Facing.Target[1] = _worldPacket.ReadBit();
                        Movement.Spline.Facing.Target[0] = _worldPacket.ReadBit();
                        Movement.Spline.Facing.Target[5] = _worldPacket.ReadBit();
                    }

                    hasSplineVerticalAcceleration = _worldPacket.ReadBit();
                    Movement.Spline.Flags = _worldPacket.ReadBits(25);
                }
            }

            GUID[6] = _worldPacket.ReadBit();

            if (hasFallData)
                hasFallDirection = _worldPacket.ReadBit();

            GUID[0] = _worldPacket.ReadBit();
            GUID[1] = _worldPacket.ReadBit();
            _worldPacket.ReadBit();
            if (!_worldPacket.ReadBit())
                Movement.FlagsExtra = _worldPacket.ReadBits(12);
        }

        if (hasGameObjectPosition)
        {
            Movement.Transport.GameObject.GUID[5] = _worldPacket.ReadBit();
            hasGameobjectTransportTime[0] = _worldPacket.ReadBit();
            Movement.Transport.GameObject.GUID[0] = _worldPacket.ReadBit();
            Movement.Transport.GameObject.GUID[3] = _worldPacket.ReadBit();
            Movement.Transport.GameObject.GUID[6] = _worldPacket.ReadBit();
            Movement.Transport.GameObject.GUID[1] = _worldPacket.ReadBit();
            Movement.Transport.GameObject.GUID[4] = _worldPacket.ReadBit();
            Movement.Transport.GameObject.GUID[2] = _worldPacket.ReadBit();
            hasGameobjectTransportTime[1] = _worldPacket.ReadBit();
            Movement.Transport.GameObject.GUID[7] = _worldPacket.ReadBit();
        }

        if (hasAttackingTarget)
        {
            TargetGUID[2] = _worldPacket.ReadBit();
            TargetGUID[7] = _worldPacket.ReadBit();
            TargetGUID[0] = _worldPacket.ReadBit();
            TargetGUID[4] = _worldPacket.ReadBit();
            TargetGUID[5] = _worldPacket.ReadBit();
            TargetGUID[6] = _worldPacket.ReadBit();
            TargetGUID[1] = _worldPacket.ReadBit();
            TargetGUID[3] = _worldPacket.ReadBit();
        }

        if (hasAnimKits)
        {
            hasAnimKit[0] = _worldPacket.ReadBit();
            hasAnimKit[1] = _worldPacket.ReadBit();
            hasAnimKit[2] = _worldPacket.ReadBit();
        }

        _worldPacket.ResetBitPos();

        for (auto&& stopFrame : StopFrames)
            _worldPacket >> stopFrame;

        if (isLiving)
        {
            _worldPacket.ReadByteSeq(GUID[4]);
            _worldPacket >> Movement.RunBackSpeed;
            if (hasFallData)
            {
                if (hasFallDirection)
                {
                    _worldPacket >> Movement.FallInfo.Jump.HorizontalSpeed >> Movement.FallInfo.Jump.Cosinus >> Movement.FallInfo.Jump.Sinus;
                }

                _worldPacket >> Movement.FallInfo.Time >> Movement.FallInfo.VerticalSpeed;
            }

            _worldPacket >> Movement.SwimBackSpeed;
            if (hasSplineElevation)
                _worldPacket >> Movement.Spline.Elevation;

            if (hasSplineData)
            {
                if (hasExtendedSplineData)
                {
                    if (hasSplineVerticalAcceleration)
                        _worldPacket >> Movement.Spline.VerticalAcceleration;

                    _worldPacket >> Movement.Spline.ID;
                    if (Movement.Spline.Type == 0) // Facing Angle
                        _worldPacket >> Movement.Spline.Facing.Angle;
                    else if (Movement.Spline.Type == 2) // Facing Target
                    {
                        _worldPacket.ReadByteSeq(Movement.Spline.Facing.Target[5]);
                        _worldPacket.ReadByteSeq(Movement.Spline.Facing.Target[3]);
                        _worldPacket.ReadByteSeq(Movement.Spline.Facing.Target[7]);
                        _worldPacket.ReadByteSeq(Movement.Spline.Facing.Target[1]);
                        _worldPacket.ReadByteSeq(Movement.Spline.Facing.Target[6]);
                        _worldPacket.ReadByteSeq(Movement.Spline.Facing.Target[4]);
                        _worldPacket.ReadByteSeq(Movement.Spline.Facing.Target[2]);
                        _worldPacket.ReadByteSeq(Movement.Spline.Facing.Target[0]);
                    }

                    for (auto&& itr : Movement.Spline.Points)
                        _worldPacket >> itr.Z >> itr.X >> itr.Y;

                    if (Movement.Spline.Type == 1) // Facing Spot
                        _worldPacket >> Movement.Spline.Facing.Position.X >> Movement.Spline.Facing.Position.Z >> Movement.Spline.Facing.Position.Y;

                    _worldPacket >> Movement.Spline.NextDurationMultiplier;
                    _worldPacket >> Movement.Spline.Duration;
                    if (hasSplineStartTime)
                        _worldPacket >> Movement.Spline.StartTime;
                    _worldPacket >> Movement.Spline.DurationMultiplier;
                }

                _worldPacket >> Movement.Spline.Endpoint.Z >> Movement.Spline.Endpoint.X >> Movement.Spline.Endpoint.Y >> Movement.Spline.ID;
            }

            _worldPacket >> Movement.Position.Z;
            _worldPacket.ReadByteSeq(GUID[5]);

            if (hasTransportData)
            {
                _worldPacket.ReadByteSeq(Movement.Transport.GUID[5]);
                _worldPacket.ReadByteSeq(Movement.Transport.GUID[7]);
                _worldPacket >> Movement.Transport.Times[0];
                _worldPacket >> Movement.Transport.Offset.O;

                if (hasTransportTime[0])
                    _worldPacket >> Movement.Transport.Times[1];

                _worldPacket >> Movement.Transport.Offset.Y;
                _worldPacket >> Movement.Transport.Offset.X;
                _worldPacket.ReadByteSeq(Movement.Transport.GUID[3]);
                _worldPacket >> Movement.Transport.Offset.Z;
                _worldPacket.ReadByteSeq(Movement.Transport.GUID[0]);

                if (hasTransportTime[1])
                    _worldPacket >> Movement.Transport.Times[2];

                _worldPacket >> Movement.Transport.Seat;
                _worldPacket.ReadByteSeq(Movement.Transport.GUID[1]);
                _worldPacket.ReadByteSeq(Movement.Transport.GUID[6]);
                _worldPacket.ReadByteSeq(Movement.Transport.GUID[2]);
                _worldPacket.ReadByteSeq(Movement.Transport.GUID[4]);
            }

            _worldPacket >> Movement.Position.X;
            _worldPacket >> Movement.PitchSpeed;
            _worldPacket.ReadByteSeq(GUID[3]);
            _worldPacket.ReadByteSeq(GUID[0]);
            _worldPacket >> Movement.SwimSpeed;
            _worldPacket >> Movement.Position.Y;
            _worldPacket.ReadByteSeq(GUID[7]);
            _worldPacket.ReadByteSeq(GUID[1]);
            _worldPacket.ReadByteSeq(GUID[2]);
            _worldPacket >> Movement.WalkSpeed;
            if (hasTimestamp)
                _worldPacket >> Movement.Time;
            _worldPacket >> Movement.FlyBackSpeed;
            _worldPacket.ReadByteSeq(GUID[6]);
            _worldPacket >> Movement.TurnSpeed;
            if (hasOrientation)
                _worldPacket >> Movement.Position.O;
            _worldPacket >> Movement.RunSpeed;
            if (hasPitch)
                _worldPacket >> Movement.Pitch;
            _worldPacket >> Movement.FlySpeed;
        }

        if (hasVehicleData)
        {
            _worldPacket >> Movement.Vehicle.O;
            _worldPacket >> Movement.Vehicle.ID;
        }

        if (hasGameObjectPosition)
        {
            _worldPacket.ReadByteSeq(Movement.Transport.GameObject.GUID[0]);
            _worldPacket.ReadByteSeq(Movement.Transport.GameObject.GUID[5]);
            if (hasGameobjectTransportTime[1])
                _worldPacket >> Movement.Transport.GameObject.Times[2];
            _worldPacket.ReadByteSeq(Movement.Transport.GameObject.GUID[3]);
            _worldPacket >> Movement.Transport.GameObject.Offset.X;
            _worldPacket.ReadByteSeq(Movement.Transport.GameObject.GUID[4]);
            _worldPacket.ReadByteSeq(Movement.Transport.GameObject.GUID[6]);
            _worldPacket.ReadByteSeq(Movement.Transport.GameObject.GUID[1]);
            _worldPacket >> Movement.Transport.GameObject.Times[0];
            _worldPacket >> Movement.Transport.GameObject.Offset.Y;
            _worldPacket.ReadByteSeq(Movement.Transport.GameObject.GUID[2]);
            _worldPacket.ReadByteSeq(Movement.Transport.GameObject.GUID[7]);
            _worldPacket >> Movement.Transport.GameObject.Offset.Z;
            _worldPacket >> Movement.Transport.GameObject.Seat;
            _worldPacket >> Movement.Transport.GameObject.Offset.O;
            if (hasGameobjectTransportTime[0])
                _worldPacket >> Movement.Transport.GameObject.Times[1];
        }

        if (hasGameObjectRotation)
        {
            Movement.Transport.GameObject.Rotation = _worldPacket.read<std::uint64_t>(); // Packed quaternion
        }

        if (unkBit456)
        {
            // ...
        }

        if (hasStationaryPosition)
        {
            _worldPacket >> Movement.Stationary.O;
            _worldPacket >> Movement.Stationary.X;
            _worldPacket >> Movement.Stationary.Y;
            _worldPacket >> Movement.Stationary.Z;
        }

        if (hasAttackingTarget)
        {
            _worldPacket.ReadByteSeq(TargetGUID[4]);
            _worldPacket.ReadByteSeq(TargetGUID[0]);
            _worldPacket.ReadByteSeq(TargetGUID[3]);
            _worldPacket.ReadByteSeq(TargetGUID[5]);
            _worldPacket.ReadByteSeq(TargetGUID[7]);
            _worldPacket.ReadByteSeq(TargetGUID[6]);
            _worldPacket.ReadByteSeq(TargetGUID[2]);
            _worldPacket.ReadByteSeq(TargetGUID[1]);
        }

        if (hasAnimKits)
        {
            if (hasAnimKit[0])
                _worldPacket >> Movement.AnimKits[0];
            if (hasAnimKit[1])
                _worldPacket >> Movement.AnimKits[1];
            if (hasAnimKit[2])
                _worldPacket >> Movement.AnimKits[2];
        }

        if (hasTransport)
            _worldPacket >> Movement.Transport.PathTimer;
    }
}

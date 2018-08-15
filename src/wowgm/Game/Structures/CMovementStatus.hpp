#pragma once

#include "ObjectGuid.hpp"
#include "C4Vector.hpp"
#include "C4Quaternion.hpp"

#include <vector>
#include <cstdint>
#include <array>

namespace wowgm::game::structures
{
#pragma pack(push, 1)
    struct CMovementStatus
    {
        CMovementStatus() : FallInfo(), Vehicle(), Transport(), Spline() {
            // This is safe since this type is not supposed to be inherit(ing|ed)
            memset(this, 0, sizeof(CMovementStatus));
        }

        std::uint8_t ObjectType;

        std::uint32_t Time;
        std::uint32_t Flags;
        std::uint16_t FlagsExtra;

        C4Vector Stationary;
        C4Vector Position;

        struct {
            struct {
                float HorizontalSpeed;
                float Sinus;
                float Cosinus;
            } Jump;

            float VerticalSpeed;
            std::uint32_t Time;
        } FallInfo;

        struct _Vehicle {
            float O;
            std::uint32_t ID;
        } Vehicle;

        float RunSpeed;
        float RunBackSpeed;
        float SwimSpeed;
        float SwimBackSpeed;
        float FlySpeed;
        float FlyBackSpeed;
        float PitchSpeed;
        float WalkSpeed;
        float TurnSpeed;

        float Pitch;

        struct {
            struct {
                std::array<std::uint32_t, 3> Times;
                C4Vector Offset;
                C4Quaternion Rotation;
                ObjectGuid GUID;

                std::uint8_t Seat;
            } GameObject;

            std::uint32_t PathTimer;
            std::array<std::uint32_t, 3> Times;
            ObjectGuid GUID;
            C4Vector Offset;

            std::uint8_t Seat;
        } Transport;

        struct {
            std::uint32_t ID;
            std::uint32_t Time;
            std::uint32_t Flags;
            std::uint32_t Duration;
            std::uint32_t StartTime;

            std::uint8_t Mode;
            std::uint8_t Type;

            float Elevation;
            float VerticalAcceleration;
            float NextDurationMultiplier;
            float DurationMultiplier;

            std::vector<C3Vector> Points;

            C3Vector Endpoint;

            struct {
                ObjectGuid Target;
                C3Vector Position;
                float Angle;
            } Facing;
        } Spline;

        std::array<std::uint32_t, 3> AnimKits;

        bool PlayHoverAnim;
        bool IsSuppressingGreetings;
        bool ThisIsYou;
        bool NoBirthAnim;

        ObjectGuid GUID;
        ObjectGuid TargetGUID;

        std::vector<std::uint32_t> StopFrames;
    };
#pragma pack(pop)
}
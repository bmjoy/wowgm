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
    struct CMovementStatus final
    {
        TypeID ObjectType = TypeID::TYPEID_OBJECT;

        std::uint32_t Time = 0;
        std::uint32_t Flags = 0;
        std::uint16_t FlagsExtra = 0;

        C4Vector Stationary;
        C4Vector Position;

        struct {
            struct {
                float HorizontalSpeed = 0.0f;
                float Sinus = 0.0f;
                float Cosinus = 0.0f;
            } Jump;

            float VerticalSpeed = 0.0f;
            std::uint32_t Time = 0;
        } FallInfo;

        struct _Vehicle {
            float O = 0.0f;
            std::uint32_t ID = 0;
        } Vehicle;

        float RunSpeed = 0;
        float RunBackSpeed = 0;
        float SwimSpeed = 0;
        float SwimBackSpeed = 0;
        float FlySpeed = 0;
        float FlyBackSpeed = 0;
        float PitchSpeed = 0;
        float WalkSpeed = 0;
        float TurnSpeed = 0;

        float Pitch = 0;

        struct {
            struct {
                std::array<std::uint32_t, 3> Times;
                C4Vector Offset;
                C4Quaternion Rotation;
                ObjectGuid GUID = ObjectGuid::Empty;

                std::uint8_t Seat = 0xFF;
            } GameObject;

            std::uint32_t PathTimer = 0;
            std::array<std::uint32_t, 3> Times;
            ObjectGuid GUID = ObjectGuid::Empty;
            C4Vector Offset;

            std::uint8_t Seat = 0xFF;
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
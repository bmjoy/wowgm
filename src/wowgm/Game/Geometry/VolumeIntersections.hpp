#pragma once

#include "C3Vector.hpp"
#include "CAaBox.hpp"

#include <cmath>
#include <array>

namespace wowgm::game::geometry
{
    namespace VolumeIntersections
    {
        using namespace wowgm::game::structures;

        static std::array<C3Vector, 8> getCorners(CAaBox const& box);
        static bool boxContainsPoint(CAaBox const& box, C3Vector const& position);
        static bool sphereContainsPoint(C3Vector const& sphereCenter, double sphereRadius, C3Vector const& position);
        static bool sphereContainsBox(C3Vector const& sphereCenter, double sphereRadius, CAaBox const& boundingBox);
        static bool boxContainsSphere(CAaBox const& box, C3Vector const& sphereCenter, double sphereRadius);
        static bool boxIntersectsSphere(CAaBox const& box, C3Vector const& sphereCenter, double sphereRadius);

        static bool boxContainsPoint(CAaBox const& box, C3Vector const& position)
        {
            for (uint32_t i = 0; i < 3; ++i)
            {
                if (box.Maximum[i] < position[i])
                    return false;

                if (box.Minimum[i] > position[i])
                    return false;
            }

            return true;
        }

        static bool sphereContainsPoint(C3Vector const& sphereCenter, double sphereRadius, C3Vector const& position)
        {
            return (sphereCenter.distanceSquared(position) <= (sphereRadius * sphereRadius));
        }

        static bool sphereContainsBox(C3Vector const& sphereCenter, double sphereRadius, CAaBox const& boundingBox)
        {
            for (C3Vector const& corners : getCorners(boundingBox))
                if (!sphereContainsPoint(sphereCenter, sphereRadius, corners))
                    return false;

            return true;
        }

        static bool boxContainsBox(CAaBox const& box, CAaBox const& otherBox)
        {
            return boxContainsPoint(box, otherBox.Maximum)
                && boxContainsPoint(box, otherBox.Minimum);
        }

        static bool boxContainsSphere(CAaBox const& box, C3Vector const& sphereCenter, double sphereRadius)
        {
            C3Vector topCasingCorner;
            topCasingCorner.X = sphereCenter.X + sphereRadius;
            topCasingCorner.Y = sphereCenter.Y + sphereRadius;
            topCasingCorner.Z = sphereCenter.Z + sphereRadius;

            if (!boxContainsPoint(box, topCasingCorner))
                return false;

            C3Vector lessCasingCorner;
            lessCasingCorner.X = sphereCenter.X - sphereRadius;
            lessCasingCorner.Y = sphereCenter.Y - sphereRadius;
            lessCasingCorner.Z = sphereCenter.Z - sphereRadius;

            if (!boxContainsPoint(box, lessCasingCorner))
                return false;

            return true;
        }

        /// This only checks for intersections, not inclusions!
        static bool boxIntersectsSphere(CAaBox const& box, C3Vector const& sphereCenter, double sphereRadius)
        {
            uint32_t cornersInside = 0;
            uint32_t cornersOutside = 0;
            for (C3Vector const& corner : getCorners(box))
            {
                if (sphereContainsPoint(sphereCenter, sphereRadius, corner))
                    ++cornersInside;
                else
                    ++cornersOutside;
            }

            if (cornersInside == 0 || cornersOutside == 0)
                return false;

            return true;
        }

        static std::array<C3Vector, 8> getCorners(CAaBox const& box)
        {
            std::array<C3Vector, 8> container;

            container[0] = box.Minimum;
            container[1] = box.Maximum;

            container[2].X = box.Maximum.X;
            container[2].Y = box.Minimum.Y;
            container[2].Z = box.Minimum.Z;

            container[3].X = box.Maximum.X;
            container[3].Y = box.Maximum.Y;
            container[3].Z = box.Minimum.Z;

            container[4].X = box.Minimum.X;
            container[4].Y = box.Maximum.Y;
            container[4].Z = box.Minimum.Z;

            container[5].X = box.Minimum.X;
            container[5].Y = box.Maximum.Y;
            container[5].Z = box.Maximum.Z;

            container[6].X = box.Minimum.X;
            container[6].Y = box.Minimum.Y;
            container[6].Z = box.Maximum.Z;

            container[7].X = box.Maximum.X;
            container[7].Y = box.Minimum.Y;
            container[7].Z = box.Maximum.Z;

            return container;
        }
    }
}
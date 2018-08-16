#pragma once

#include "WorldPacket.hpp"

#include <type_traits>
#include <mutex>
#include <fstream>

namespace wowgm::utilities
{
    using namespace wowgm::protocol::world;

    namespace
    {

    }

    class PacketLogger
    {
        static void Initialize();
        static void _Initialize();

        static std::ofstream& GetFileStream();

        static void WriteBody(WorldPacket* worldPacket);

        template <typename T>
        static void Write(T data);

        static void WriteData(const char* data, size_t size);

    public:
        ~PacketLogger();

        static void WriteClientPacket(WorldPacket* worldPacket);

        static void WriteServerPacket(WorldPacket* worldPacket);

    };
}
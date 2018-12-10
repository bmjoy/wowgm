#include "PacketLogger.hpp"
#include "ClientServices.hpp"
#include "BigNumber.hpp"

#include <cstdint>
#include <memory>
#include <chrono>

namespace wowgm::utilities
{
    using namespace wowgm::protocol;
    using namespace wowgm::cryptography;
    using namespace std::chrono;
    static const steady_clock::time_point start_time = steady_clock::now();

    PacketLogger::~PacketLogger()
    {
        GetFileStream().flush();
        GetFileStream().close();
    }

    void PacketLogger::Initialize()
    {
        static std::once_flag flag;
        std::call_once(flag, PacketLogger::_Initialize);
    }

    void PacketLogger::_Initialize()
    {
        WriteData("PKT", 3);
        Write<uint16_t>(0x0301);
        Write<uint8_t>('W');
        Write<uint32_t>(15595);
        WriteData("enUS", 4);
        std::unique_ptr<uint8_t[]> sessionKey = sClientServices->GetSessionKey().AsByteArray(40);
        WriteData(reinterpret_cast<const char*>(*sessionKey.get()), 40);
        Write<uint32_t>(std::time(nullptr));
        Write<uint32_t>(duration_cast<milliseconds>(steady_clock::now() - start_time).count());
        Write<uint32_t>(0);
    }

    void PacketLogger::WriteData(const char* data, size_t size)
    {
        GetFileStream().write(data, size);
    }

    std::ofstream& PacketLogger::GetFileStream()
    {
        static std::ofstream _fileHandle("./World.pkt", std::ofstream::binary);
        return _fileHandle;
    }

    template <typename T>
    void PacketLogger::Write(T data)
    {
        static_assert(std::is_standard_layout<T>::value);
        static_assert(std::is_trivial<T>::value);

        GetFileStream().write(reinterpret_cast<char*>(&data), sizeof(T));
    }

    void PacketLogger::WriteClientPacket(WorldPacket* worldPacket)
    {
        Initialize();
        Write<uint32_t>(0x47534d43);
        WriteBody(worldPacket);
    }

    void PacketLogger::WriteServerPacket(WorldPacket* worldPacket)
    {
        Initialize();
        Write<uint32_t>(0x47534d53);
        WriteBody(worldPacket);
    }

    void PacketLogger::WriteBody(WorldPacket* worldPacket)
    {
        Write<uint32_t>(worldPacket->GetConnection());
        Write<uint32_t>(duration_cast<milliseconds>(steady_clock::now() - start_time).count());
        Write<uint32_t>(0);

        Write<uint32_t>(worldPacket->size() + 4);
        Write<uint32_t>(uint32_t(worldPacket->GetOpcode()));
        if (worldPacket->size() != 0)
            WriteData(reinterpret_cast<const char*>(worldPacket->contents()), worldPacket->size());

        GetFileStream().flush();
    }
}
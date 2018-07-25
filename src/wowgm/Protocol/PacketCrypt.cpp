#include "PacketCrypt.hpp"

namespace wowgm::protocol
{
    PacketCrypt::PacketCrypt(std::uint32_t digestSize)
        : _clientDecrypt(digestSize), _serverEncrypt(digestSize), _initialized(false)
    {
    }

    void PacketCrypt::DecryptRecv(std::uint8_t* data, size_t len)
    {
        if (!_initialized)
            return;

        _clientDecrypt.UpdateData(len, data);
    }

    void PacketCrypt::EncryptSend(std::uint8_t* data, size_t len)
    {
        if (!_initialized)
            return;

        _serverEncrypt.UpdateData(len, data);
    }

}
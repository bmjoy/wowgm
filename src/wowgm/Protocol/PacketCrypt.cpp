#include "PacketCrypt.hpp"

namespace wowgm::protocol
{
    PacketCrypt::PacketCrypt(std::uint32_t digestSize)
        : _clientEncrypt(digestSize), _serverDecrypt(digestSize), _initialized(false)
    {
    }

    void PacketCrypt::DecryptRecv(std::uint8_t* data, size_t len)
    {
        if (!_initialized)
            return;

        _serverDecrypt.UpdateData(len, data);
    }

    void PacketCrypt::EncryptSend(std::uint8_t* data, size_t len)
    {
        if (!_initialized)
            return;

        _clientEncrypt.UpdateData(len, data);
    }

}
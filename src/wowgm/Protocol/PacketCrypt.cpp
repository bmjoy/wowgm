#include "PacketCrypt.hpp"

namespace wowgm::protocol
{
    PacketCrypt::PacketCrypt(uint32_t digestSize)
        : _clientEncrypt(digestSize), _serverDecrypt(digestSize), _initialized(false)
    {
    }

    void PacketCrypt::DecryptRecv(uint8_t* data, size_t len)
    {
        if (!_initialized)
            return;

        _serverDecrypt.UpdateData(len, data);
    }

    void PacketCrypt::EncryptSend(uint8_t* data, size_t len)
    {
        if (!_initialized)
            return;

        _clientEncrypt.UpdateData(len, data);
    }

}
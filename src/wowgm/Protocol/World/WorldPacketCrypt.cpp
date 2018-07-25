#include "WorldPacketCrypt.hpp"
#include "HmacHash.hpp"
#include "BigNumber.hpp"

#include <cstring>

namespace wowgm::protocol::world
{
    WorldPacketCrypt::WorldPacketCrypt() : PacketCrypt(SHA_DIGEST_LENGTH)
    {
    }

    void WorldPacketCrypt::Init(BigNumber* K)
    {
        std::uint8_t ServerEncryptionKey[SEED_KEY_SIZE] = { 0xCC, 0x98, 0xAE, 0x04, 0xE8, 0x97, 0xEA, 0xCA, 0x12, 0xDD, 0xC0, 0x93, 0x42, 0x91, 0x53, 0x57 };
        HmacSha1 serverEncryptHmac(SEED_KEY_SIZE, (std::uint8_t*)ServerEncryptionKey);
        std::uint8_t* encryptHash = serverEncryptHmac.ComputeHash(K);

        std::uint8_t ServerDecryptionKey[SEED_KEY_SIZE] = { 0xC2, 0xB3, 0x72, 0x3C, 0xC6, 0xAE, 0xD9, 0xB5, 0x34, 0x3C, 0x53, 0xEE, 0x2F, 0x43, 0x67, 0xCE };
        HmacSha1 clientDecryptHmac(SEED_KEY_SIZE, (std::uint8_t*)ServerDecryptionKey);
        std::uint8_t* decryptHash = clientDecryptHmac.ComputeHash(K);

        _clientDecrypt.Init(decryptHash);
        _serverEncrypt.Init(encryptHash);

        // Drop first 1024 bytes, as WoW uses ARC4-drop1024.
        std::uint8_t syncBuf[1024];

        memset(syncBuf, 0, 1024);
        _serverEncrypt.UpdateData(1024, syncBuf);

        memset(syncBuf, 0, 1024);
        _clientDecrypt.UpdateData(1024, syncBuf);

        _initialized = true;
    }

}
#include "WorldPacketCrypt.hpp"

#include <shared/cryptography/HmacHash.hpp>
#include <shared/cryptography/BigNumber.hpp>

#include <cstring>

namespace wowgm::protocol::world
{
    WorldPacketCrypt::WorldPacketCrypt() : PacketCrypt(SHA_DIGEST_LENGTH)
    {
    }

    void WorldPacketCrypt::Init(const BigNumber& K)
    {
        uint8_t ServerDecryptionKey[SEED_KEY_SIZE] = { 0xC2, 0xB3, 0x72, 0x3C, 0xC6, 0xAE, 0xD9, 0xB5, 0x34, 0x3C, 0x53, 0xEE, 0x2F, 0x43, 0x67, 0xCE };
        HmacSha1 serverDecryptHmac(SEED_KEY_SIZE, (uint8_t*)ServerDecryptionKey);
        uint8_t* decryptHash = serverDecryptHmac.ComputeHash(K);

        uint8_t ClientEncryptionKey[SEED_KEY_SIZE] = { 0xCC, 0x98, 0xAE, 0x04, 0xE8, 0x97, 0xEA, 0xCA, 0x12, 0xDD, 0xC0, 0x93, 0x42, 0x91, 0x53, 0x57 };
        HmacSha1 clientEncryptHmac(SEED_KEY_SIZE, (uint8_t*)ClientEncryptionKey);
        uint8_t* encryptHash = clientEncryptHmac.ComputeHash(K);

        _clientEncrypt.Init(decryptHash);
        _serverDecrypt.Init(encryptHash);

        // Drop first 1024 bytes, as WoW uses ARC4-drop1024.
        uint8_t syncBuf[1024];

        memset(syncBuf, 0, 1024);
        _clientEncrypt.UpdateData(1024, syncBuf);

        memset(syncBuf, 0, 1024);
        _serverDecrypt.UpdateData(1024, syncBuf);

        _initialized = true;
    }

}
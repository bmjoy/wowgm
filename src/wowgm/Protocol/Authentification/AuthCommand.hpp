#pragma once

namespace wowgm::protocol::authentification
{
    enum AuthCommand
    {
        AUTH_LOGON_CHALLENGE = 0x00,
        AUTH_LOGON_PROOF = 0x01,
        AUTH_RECONNECT_CHALLENGE = 0x02,
        AUTH_RECONNECT_PROOF = 0x03,
        REALM_LIST = 0x10,
    };
} // namespace wowgm::protocol::authentification

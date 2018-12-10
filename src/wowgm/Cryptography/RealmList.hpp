#pragma once

#include <charconv>
#include <cstdint>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace wowgm::protocol::authentification
{

    #pragma pack(push, 1)
    struct RealmList
    {
        uint32_t Data;
    };
    #pragma pack(pop)

    #pragma pack(push, 1)
    struct AuthRealmList
    {
        uint8_t Command;
        uint16_t Size;
        uint32_t Unk0;
        uint16_t Count; // < WoTLK: this is 32 bits.
    };
    #pragma pack(pop)

    struct AuthRealmInfo
    {
        uint8_t Type;
        uint8_t Locked;
        uint8_t Flags;
        std::string Name;
        std::string Address;
        float Population;
        uint8_t Load;
        uint8_t Timezone;
        uint8_t ID;
        uint8_t Version[3];
        uint16_t Build;
        uint8_t ExpFlags[2]; // 2.x, 3.x
                                  // 1.12.1 1.12.2 have different values.

        uint32_t GetPort() const
        {
            auto pos = Address.find(':');
            if (pos == std::string::npos)
                return 3724u;

            uint32_t port;
            std::from_chars(Address.data() + pos + 1, Address.data() + Address.size(), port);
            return port;
        }

        boost::asio::ip::tcp::endpoint GetEndpoint() const
        {
            std::string_view sv(Address);
            return boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address(sv.substr(0, sv.find(':'))), GetPort());
        }
    };

} // wowgm::protocol::authentification

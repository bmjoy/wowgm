#pragma once

#include "Socket.hpp"
#include "BigNumber.hpp"
#include "SHA1.hpp"

#include <boost/asio/io_context.hpp>

#include <unordered_map>

namespace boost {
    namespace system {
        class error_code;
    }
}

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

namespace wowgm::networking::world
{
    using namespace wowgm::cryptography;

    class WorldSocket : public Socket<WorldSocket>
    {
        typedef Socket<WorldSocket> BaseSocket;

    public:
        WorldSocket(asio::io_context& service);

        WorldSocket(WorldSocket&&) = delete;
        WorldSocket(WorldSocket const&) = delete;


    protected:
        void ReadHandler() override;

        void OnClose() override;

    private:
        void InitializeHandlers();


        std::string _username;
        std::string _password;

        BigNumber M2;
    };

} // namespace wowgm

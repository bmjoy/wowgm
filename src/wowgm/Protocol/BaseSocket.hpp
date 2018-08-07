#pragma once

#include <boost/asio/ip/tcp.hpp>

namespace wowgm::protocol
{
    using tcp = boost::asio::ip::tcp;

    class MessageBuffer;

    class BaseSocket
    {
        public:
            virtual bool IsOpen() const = 0;
            virtual void DelayedCloseSocket() = 0;
            virtual void CloseSocket() = 0;
            virtual bool Update() = 0;

            virtual void QueuePacket(MessageBuffer&& buffer) = 0;

            virtual void Connect(tcp::endpoint const& endpoint) = 0;
            virtual void Connect(std::string hostname, std::uint32_t port) = 0;

            virtual tcp::endpoint GetLocalEndpoint() const = 0;

            // virtual boost::system::error_code const& GetErrorCode() const = 0;
    };

} // wowgm::protocol

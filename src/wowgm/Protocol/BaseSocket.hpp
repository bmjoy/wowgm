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
            virtual void AsyncCloseSocket() = 0;
            virtual void CloseSocket() = 0;
            virtual void Update() = 0;

            virtual void QueuePacket(MessageBuffer& buffer) = 0;

            virtual bool Connect(tcp::endpoint const& endpoint) = 0;

            virtual tcp::endpoint GetLocalEndpoint() = 0;
    };

} // wowgm::protocol

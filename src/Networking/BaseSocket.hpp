#pragma once

#include <boost/array.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace wowgm::networking {

    namespace asio = boost::asio;
    using tcp = asio::ip::tcp;

    class MessageBuffer;

    class BaseSocket
    {
        public:
            virtual bool IsOpen() const = 0;
            virtual void AsyncCloseSocket() = 0;
            virtual void CloseSocket() = 0;
            virtual void Update() = 0;

            virtual void QueuePacket(MessageBuffer& buffer) = 0;

            virtual void Connect(tcp::endpoint& endpoint) = 0;

            virtual tcp::endpoint GetLocalEndpoint() = 0;
    };

} // wowgm::networking

#pragma once

#include "BaseSocket.hpp"
#include "MessageBuffer.hpp"
#include "Assert.hpp"

#include <boost/array.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>

#include <memory>
#include <queue>
#include <cstdint>

namespace boost {
    namespace system {
        class error_code;
    }
}

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

namespace wowgm::protocol
{
    template <typename T>
    class Socket : public std::enable_shared_from_this<T>, public BaseSocket
    {
        Socket() = delete;
        Socket(const Socket&) = delete;
        Socket(Socket&&) = delete;

    public:
        Socket(asio::io_context& context) : _context(context), _socket(context), _closed(false), _closing(false)
        {
        }

        virtual ~Socket()
        {
            CloseSocket();
        }

        void Connect(tcp::endpoint const& endpoint) override final
        {
            auto callback = [&](const boost::system::error_code& errorCode, tcp::endpoint targetEndpoint) -> void {
                BOOST_ASSERT_MSG_FMT(errorCode == 0,
                    "Error %u while connecting to %s: %s", errorCode.value(), targetEndpoint.address().to_string().c_str(), errorCode.message().c_str());

                _OnConnect();
            };

            _socket.async_connect(endpoint, std::bind(callback, std::placeholders::_1, endpoint));
        }

        void Connect(std::string hostname, std::uint32_t port) override final
        {
            boost::asio::ip::tcp::resolver resolver(_context);

            boost::system::error_code errorCode;
            auto results = resolver.resolve(hostname, std::to_string(port), errorCode);
            BOOST_ASSERT_MSG_FMT(errorCode == 0, "Unable to resolve %s:%u", hostname.c_str(), port);

            auto callback = [&](const boost::system::error_code& error, tcp::resolver::results_type::const_iterator targetEndpointItr) -> void {
                BOOST_ASSERT_MSG_FMT(error == 0,
                    "Error %u while connecting to %s: %s", error.value(), targetEndpointItr->endpoint().address().to_string().c_str(), error.message().c_str());

                _OnConnect();
            };

            boost::asio::async_connect(_socket, results.begin(), results.end(), callback);
        }

        bool IsOpen() const override final { return _socket.is_open() && !_closed && !_closing; }

        void CloseSocket() override
        {
            if (_closed.exchange(true))
                return;

            boost::system::error_code errorCode;
            _socket.shutdown(boost::asio::socket_base::shutdown_both, errorCode);
            _socket.close();

            BOOST_ASSERT_MSG_FMT(errorCode == 0, "Error %u while closing: %s", errorCode.value(), errorCode.message().c_str());
            impl().OnClose();
        }

        void AsyncCloseSocket() override
        {
            _closing = true;
        }

        void Update()
        {
            if (!IsOpen())
                return;

            if (!_writeQueue.empty())
            {
                bool success = SendMessageInternal(_writeQueue.front());
                if (success)
                    _writeQueue.pop();
            }

            _readBuffer.Normalize();
            _readBuffer.EnsureFreeSpace();

            _socket.async_read_some(_readBuffer.AsWriteBuffer(),
                std::bind(&Socket<T>::InternalReadHandler, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2));
        }

        MessageBuffer& GetReadBuffer() { return _readBuffer; }

        boost::asio::ip::tcp::endpoint GetLocalEndpoint() const override final
        {
            return _socket.local_endpoint();
        }

        void QueuePacket(MessageBuffer& buffer) override final
        {
            _writeQueue.push(std::move(buffer));
        }

    private:

        void _OnConnect()
        {
            BOOST_ASSERT_MSG_FMT(IsOpen(), "Socket not open!");

            impl().OnConnect();
        }

        T& impl() { return static_cast<T&>(*this); }
        T const& impl() const { return static_cast<T const&>(*this); }

        bool SendMessageInternal(MessageBuffer& buffer)
        {
            if (!IsOpen())
                return false;

            boost::system::error_code errorCode;
            std::size_t bytesSent = _socket.write_some(buffer.AsReadBuffer(), errorCode);

            BOOST_ASSERT_MSG_FMT(errorCode == 0, "Error %u while sending data: %s", errorCode.value(), errorCode.message().c_str());

            buffer.ReadCompleted(bytesSent);

            if (buffer.GetActiveSize() != 0)
                return SendMessageInternal(buffer);
            else
            {
                if (_closing)
                    CloseSocket();

                return true;
            }
        }

        void InternalReadHandler(boost::system::error_code const& errorCode, size_t transferredBytes)
        {
            if (!IsOpen())
                return;

            BOOST_ASSERT_MSG_FMT(errorCode == 0, "Error %u while reading data: %s", errorCode.value(), errorCode.message().c_str());

            if (transferredBytes == 0)
                return;

            _readBuffer.WriteCompleted(transferredBytes);
            impl().OnRead();
        }


        boost::system::error_code const& GetErrorCode() const override final
        {
            return _errorCode;
        }

    private:
        boost::system::error_code _errorCode;

        boost::asio::io_context& _context;
        boost::asio::ip::tcp::socket _socket;

        MessageBuffer _readBuffer;
        std::queue<MessageBuffer> _writeQueue;

        std::atomic<bool> _closed;
        std::atomic<bool> _closing;
    };

} // wowgm::protocol

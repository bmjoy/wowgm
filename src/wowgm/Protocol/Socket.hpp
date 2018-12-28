#pragma once

#include "BaseSocket.hpp"
#include "MessageBuffer.hpp"
#include <shared/assert/assert.hpp>

// Shut up.
#define BOOST_ASIO_ENABLE_BUFFER_DEBUGGING

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
#include <iostream>

namespace boost {
    namespace system {
        class error_code;
    }
}

#ifdef BOOST_ASIO_HAS_IOCP
#define WOWGM_IOCP
#endif

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

namespace wowgm::protocol
{
    template <typename T>
    class Socket : public BaseSocket, public std::enable_shared_from_this<T>
    {
    public:
        explicit Socket(asio::io_context& context) : _context(context), _socket(context), _closed(false), _closing(false), _isWritingAsync(false), _readBuffer(4096)
        {
        }

        virtual ~Socket()
        {
            _closed = true;
            boost::system::error_code error;
            _socket.close(error);
        }

        void Connect(tcp::endpoint const& endpoint) override final
        {
            auto callback = [&](const boost::system::error_code& errorCode, tcp::endpoint targetEndpoint) -> void {
                BOOST_ASSERT_MSG_FMT(errorCode == 0,
                    "Error %u while connecting to %s:%u : %s", errorCode.value(), targetEndpoint.address().to_string().c_str(), targetEndpoint.port(), errorCode.message().c_str());

                OnConnect();
            };

            _socket.async_connect(endpoint, std::bind(callback, std::placeholders::_1, endpoint));
        }

        void Connect(std::string_view hostname, uint32_t port) override final
        {
            boost::asio::ip::tcp::resolver resolver(_context);

            boost::system::error_code errorCode;
            auto results = resolver.resolve(hostname, std::to_string(port), errorCode);
            BOOST_ASSERT_MSG_FMT(errorCode == 0, "Unable to resolve %s:%u", hostname.data(), port);

            auto callback = [&](const boost::system::error_code& error, tcp::resolver::results_type::const_iterator targetEndpointItr) -> void {
                tcp::resolver::results_type::const_iterator end;
                if (targetEndpointItr == end)
                    BOOST_ASSERT_MSG_FMT(error == 0, "Error %u while connecting to auth server: %s", error.value(), error.message().c_str());

                BOOST_ASSERT_MSG_FMT(error == 0,
                    "Error %u while connecting to %s:%u : %s", error.value(), targetEndpointItr->endpoint().address().to_string().c_str(), targetEndpointItr->endpoint().port(), error.message().c_str());

                OnConnect();
            };

            boost::asio::async_connect(_socket, results.begin(), results.end(), callback);
        }

        virtual bool Update()
        {
            if (_closed)
                return false;

#ifndef WOWGM_IOCP
            if (_isWritingAsync || (_writeQueue.empty() && !_closing))
                return true;

            for (; HandleQueue();)
                ;
#endif

            return true;
        }

        void AsyncRead()
        {
            if (!IsOpen())
                return;

            _readBuffer.Normalize();
            _readBuffer.EnsureFreeSpace();
            _socket.async_read_some(boost::asio::buffer(_readBuffer.GetWritePointer(), _readBuffer.GetRemainingSpace()),
                std::bind(&Socket<T>::ReadHandlerInternal, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2));
        }

        void AsyncReadWithCallback(void (T::*callback)(boost::system::error_code, std::size_t))
        {
            if (!IsOpen())
                return;

            _readBuffer.Normalize();
            _readBuffer.EnsureFreeSpace();
            _socket.async_read_some(boost::asio::buffer(_readBuffer.GetWritePointer(), _readBuffer.GetRemainingSpace()),
                std::bind(callback, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2));
        }

        void QueuePacket(MessageBuffer&& buffer)
        {
            _writeQueue.push(std::move(buffer));

#ifdef WOWGM_IOCP
            AsyncProcessQueue();
#endif
        }

        bool IsOpen() const { return !_closed && !_closing; }

        void CloseSocket()
        {
            if (_closed.exchange(true))
                return;

            boost::system::error_code shutdownError;
            _socket.shutdown(boost::asio::socket_base::shutdown_send, shutdownError);
            BOOST_ASSERT_MSG_FMT(shutdownError == 0, "Error %u while shutting down socket: %s", shutdownError.value(), shutdownError.message().c_str());

            _socket.close(shutdownError);
            BOOST_ASSERT_MSG_FMT(shutdownError == 0, "Error %u while closing socket: %s", shutdownError.value(), shutdownError.message().c_str());

            OnClose();
        }

        virtual void SetNoDelay(bool disableNagle)
        {
            _socket.set_option(tcp::no_delay(disableNagle));
        }

        /// Marks the socket for closing after write buffer becomes empty
        void DelayedCloseSocket() { _closing = true; }

        MessageBuffer& GetReadBuffer() { return _readBuffer; }

        tcp::endpoint GetLocalEndpoint() const override final
        {
            return _socket.local_endpoint();
        }

    protected:
        virtual void OnClose() { }
        virtual void OnConnect() { }
        virtual void ReadHandler() = 0;

        bool AsyncProcessQueue()
        {
            if (_isWritingAsync)
                return false;

            _isWritingAsync = true;

#ifdef WOWGM_IOCP
            MessageBuffer& buffer = _writeQueue.front();
            _socket.async_write_some(boost::asio::mutable_buffer(buffer.GetReadPointer(), buffer.GetActiveSize()), std::bind(&Socket<T>::WriteHandler,
                this->shared_from_this(), std::placeholders::_1, std::placeholders::_2));
#else
            _socket.async_write_some(boost::asio::null_buffers(), std::bind(&Socket<T>::WriteHandlerWrapper,
                this->shared_from_this(), std::placeholders::_1, std::placeholders::_2));
#endif

            return false;
        }

    private:
        void ReadHandlerInternal(boost::system::error_code error, size_t transferredBytes)
        {
            if (error)
            {
                CloseSocket();
                return;
            }

            _readBuffer.WriteCompleted(transferredBytes);
            ReadHandler();
        }

#ifdef WOWGM_IOCP
        void WriteHandler(boost::system::error_code error, std::size_t transferedBytes)
        {
            if (!error)
            {
                _isWritingAsync = false;
                _writeQueue.front().ReadCompleted(transferedBytes);
                if (!_writeQueue.front().GetActiveSize())
                    _writeQueue.pop();

                if (!_writeQueue.empty())
                    AsyncProcessQueue();
                else if (_closing)
                    CloseSocket();
            }
            else
                CloseSocket();
        }

#else

        void WriteHandlerWrapper(boost::system::error_code /*error*/, std::size_t /*transferedBytes*/)
        {
            _isWritingAsync = false;
            HandleQueue();
        }

        bool HandleQueue()
        {
            if (_writeQueue.empty())
                return false;

            MessageBuffer& queuedMessage = _writeQueue.front();

            std::size_t bytesToSend = queuedMessage.GetActiveSize();

            boost::system::error_code error;
            std::size_t bytesSent = _socket.write_some(boost::asio::buffer(queuedMessage.GetReadPointer(), bytesToSend), error);

            if (error)
            {
                if (error == boost::asio::error::would_block || error == boost::asio::error::try_again)
                    return AsyncProcessQueue();

                _writeQueue.pop();
                if (_closing && _writeQueue.empty())
                    CloseSocket();
                return false;
            }
            else if (bytesSent == 0)
            {
                _writeQueue.pop();
                if (_closing && _writeQueue.empty())
                    CloseSocket();
                return false;
            }
            else if (bytesSent < bytesToSend) // now n > 0
            {
                queuedMessage.ReadCompleted(bytesSent);
                return AsyncProcessQueue();
            }

            _writeQueue.pop();
            if (_closing && _writeQueue.empty())
                CloseSocket();
            return !_writeQueue.empty();
        }

#endif

        asio::io_context& _context;
        tcp::socket _socket;

        MessageBuffer _readBuffer;
        std::queue<MessageBuffer> _writeQueue;

        std::atomic<bool> _closed;
        std::atomic<bool> _closing;

        bool _isWritingAsync;
    };

} // wowgm::protocol

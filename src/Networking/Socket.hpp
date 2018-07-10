#pragma once

#include "BaseSocket.hpp"
#include "MessageBuffer.hpp"

#include <boost/array.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/placeholders.hpp>
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


template <class T>
class Socket : public BaseSocket, public std::enable_shared_from_this<T>
{
    public:
        Socket(asio::io_context& ioService) : _socket(nullptr), _closed(false), _closing(false), _ioContext(ioService)
        {
        }

        virtual ~Socket()
        {
            _closed = true;

            if (IsOpen())
            {
                boost::system::error_code error;
                _socket->shutdown(boost::asio::socket_base::shutdown_both, error);
                _socket->close(error);
            }
            delete _socket;
        }

        void Connect(tcp::endpoint&& endpoint) override
        {
            _socket = new tcp::socket(_ioContext);
            _socket->connect(endpoint);
        }

        void CloseSocket() override final
        {
            if (_closed.exchange(true))
                return;

            boost::system::error_code errorCode;
            _socket->shutdown(boost::asio::socket_base::shutdown_send, errorCode);

            OnClose();
        }

        /// The socket will be closed when the outgoing message queue becomes empty.
        void AsyncCloseSocket()
        {
            _closing = true;
        }

        bool IsOpen() const override final { return _socket != nullptr && _socket->is_open() && !_closed && !_closing; }

        void AsyncRead()
        {
            AsyncReadWithCallback(&Socket<T>::ReadHandlerInternal);
        }

        void AsyncReadWithCallback(void (T::*callback)(const boost::system::error_code& errorCode, std::size_t transferredBytes))
        {
            if (!IsOpen())
                return;

            _readBuffer.Normalize();
            _readBuffer.EnsureFreeSpace();

            _socket->async_read_some(_readBuffer.AsWriteBuffer(),
                std::bind(callback, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2));
        }

        void QueuePacket(MessageBuffer&& buffer) override final
        {
            _writeQueue.push(std::move(buffer));
        }

        void Update() override
        {
            if (!_writeQueue.empty()) {

                bool success = SendMessageInternal(_writeQueue.front());
                if (success)
                    _writeQueue.pop();
            }

            AsyncRead();
        }

        tcp::endpoint GetLocalEndpoint()
        {
            return _socket->local_endpoint();
        }

    private:

        bool SendMessageInternal(MessageBuffer& buffer)
        {
            if (!IsOpen())
                return false;

            boost::system::error_code errorCode;
            std::size_t bytesSent = _socket->write_some(buffer.AsReadBuffer(), errorCode);

            if (errorCode != 0)
                throw std::exception(errorCode.message().c_str());

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

    protected:

        virtual void OnClose() = 0;
        virtual void ReadHandler() = 0;

    private:
        void ReadHandlerInternal(const boost::system::error_code& errorCode, size_t transferredBytes)
        {
            if (errorCode != 0)
            {
                CloseSocket();
                return;
            }

            _readBuffer.WriteCompleted(transferredBytes);
            ReadHandler();
        }

    protected:
        boost::asio::ip::tcp::socket* _socket;

        MessageBuffer _readBuffer;
        std::queue<MessageBuffer> _writeQueue;

        std::atomic<bool> _closed;
        std::atomic<bool> _closing;

        boost::asio::io_context& _ioContext;
};
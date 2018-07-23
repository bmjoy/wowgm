#include "SocketManager.hpp"
#include "BaseSocket.hpp"

namespace wowgm::threading {

    boost::asio::io_context SocketManager::_context;

    void SocketManager::Update(std::uint32_t timeInterval)
    {
        _context.run();

        if (_socket)
            _socket->Update();
    }

    std::shared_ptr<BaseSocket> SocketManager::GetSocket()
    {
        return _socket;
    }

    void SocketManager::Destroy()
    {
        if (_socket)
            _socket->AsyncCloseSocket();

        _context.run();
    }

} // wowgm::threading

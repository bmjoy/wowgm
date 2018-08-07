#include "SocketManager.hpp"
#include "BaseSocket.hpp"

namespace wowgm::threading {

    SocketManager::SocketManager() : _socket(), _context(), _guard(_context.get_executor())
    {
        _contextThread = std::thread([&]() {
            _context.run();
        });
    }

    void SocketManager::Update(std::uint32_t /* timeInterval */)
    {
        if (_socket)
            _socket->Update();
    }

    std::shared_ptr<BaseSocket> SocketManager::GetSocket()
    {
        return _socket;
    }

    void SocketManager::Destroy()
    {
        _guard.reset();

        if (_socket)
            _socket->DelayedCloseSocket();
    }

} // wowgm::threading

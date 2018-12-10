#include "SocketManager.hpp"
#include "BaseSocket.hpp"

namespace wowgm::threading {

    SocketManager::SocketManager() : _socket(), _context(), _guard(_context.get_executor())
    {
        _contextThread = std::thread([&]() {
            _context.run();
        });
    }

    void SocketManager::Update(uint32_t /* timeInterval */)
    {
        if (_socket)
            _socket->Update();
    }

    std::shared_ptr<BaseSocket> SocketManager::GetSocket()
    {
        return _socket;
    }

    SocketManager::~SocketManager()
    {
    }

    void SocketManager::Destroy()
    {
        _guard.reset();

        _context.stop();
        _contextThread.join();

        if (_socket)
            _socket->DelayedCloseSocket();
    }

} // wowgm::threading

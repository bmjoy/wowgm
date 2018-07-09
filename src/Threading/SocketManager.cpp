#include "SocketManager.hpp"
#include "BaseSocket.hpp"

void SocketManager::Update(std::uint32_t timeInterval)
{
    if (_socket != nullptr)
        _socket->Update();

    _context.run();
}

std::shared_ptr<BaseSocket> SocketManager::GetSocket()
{
    return _socket;
}

#include "SocketManager.hpp"
#include "BaseSocket.hpp"

void SocketManager::Update(std::uint32_t timeInterval)
{
    _context.run_for(std::chrono::milliseconds(100));

    if (_socket)
        _socket->Update();

}

std::shared_ptr<BaseSocket> SocketManager::GetSocket()
{
    return _socket;
}

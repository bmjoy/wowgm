#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/executor_work_guard.hpp>


#include <cstdint>
#include <memory>
#include <thread>

#include "Updatable.hpp"
#include "BaseSocket.hpp"

namespace wowgm::threading
{
    using namespace wowgm::protocol;

    class SocketManager : public Updatable
    {
    public:
        SocketManager();

        template <typename T, typename... Args, typename std::enable_if<std::is_base_of<BaseSocket, T>::value, int>::type = 0>
        std::shared_ptr<T> Create(Args&&... args)
        {
            auto ptr = std::make_shared<T>(std::forward<Args>(args)...);
            _socket = ptr;
            return ptr;
        }

        void Update(std::uint32_t timeInterval) override final;
        void Destroy() override final;

        std::shared_ptr<BaseSocket> GetSocket();

        inline boost::asio::io_context& GetContext() { return _context; }

    private:
        std::shared_ptr<BaseSocket> _socket;
        std::thread _contextThread;

        boost::asio::io_context _context;
        boost::asio::executor_work_guard<boost::asio::io_context::executor_type> _guard;
    };

} // wowgm::threading

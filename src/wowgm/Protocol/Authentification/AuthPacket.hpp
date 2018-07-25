#pragma once

#include <cstdint>
#include <vector>
#include <memory>

#include "Packet.hpp"
#include "AuthCommand.hpp"

namespace wowgm::protocol { class BaseSocket; }

namespace wowgm::protocol::authentification
{
    template <typename T>
    class AuthPacket
    {
        AuthPacket(AuthPacket&&) = delete;
        AuthPacket(AuthPacket const&) = delete;

        AuthPacket& operator = (AuthPacket const&) = delete;

    public:

        AuthPacket(std::shared_ptr<BaseSocket> socket, AuthCommand command) : _socket(socket), _command(command), _wpos(0), _extraData(10)
        {

        }

        AuthPacket(MessageBuffer& buffer) : _wpos(0), _extraData(0)
        {
            auto size = std::min(sizeof(T), buffer.GetActiveSize());

            memcpy(&_data, buffer.GetReadPointer(), size);
        }

        ~AuthPacket()
        {
            if (_socket && _socket->IsOpen())
            {
                MessageBuffer buffer(1 + sizeof(T) + _extraData.size());
                buffer.GetWritePointer()[0] = std::uint8_t(_command);
                buffer.WriteCompleted(1);
                memcpy(reinterpret_cast<T*>(buffer.GetWritePointer()), &_data, sizeof(T));
                buffer.WriteCompleted(sizeof(T));
                if (!_extraData.empty())
                {
                    memcpy(buffer.GetWritePointer(), _extraData.data(), _wpos);
                    buffer.WriteCompleted(_wpos);
                }

                _socket->QueuePacket(buffer);
            }

            _extraData.resize(0);
        }

        T& operator -> ()
        {
            return &_data;
        }

        AuthPacket& operator += (std::string const& str)
        {
            if (str.length() == 0)
                return *this;

            EnsureExtraSpace(str.length());
            memcpy(_extraData.data() + _wpos, str.c_str(), str.length());
            _wpos += str.length();
            return *this;
        }

        T* GetData() { return &_data; }

    private:
        void EnsureExtraSpace(std::size_t n)
        {
            if (_wpos + n > _extraData.size())
                _extraData.resize(_wpos + n);
        }

    private:
        std::shared_ptr<BaseSocket> _socket;

        AuthCommand _command;
        T _data;

        std::size_t _wpos;
        std::vector<std::uint8_t> _extraData;
    };

} // authentification

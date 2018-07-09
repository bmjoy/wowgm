#pragma once

#include <cstdint>
#include <vector>
#include <memory>

#include "Packet.hpp"
#include "AuthCommand.hpp"

class BaseSocket;

template <typename T>
class AuthPacket
{
    AuthPacket(AuthPacket&&) = delete;
    AuthPacket(AuthPacket const&) = delete;

    AuthPacket& operator = (AuthPacket const&) = delete;

public:

    AuthPacket(std::shared_ptr<BaseSocket> socket, AuthCommand command) : _socket(socket), _command(command), _wpos(0)
    {

    }

    AuthPacket(MessageBuffer& buffer) : _wpos(0)
    {
        memcpy(&_data, buffer.GetReadPointer(), buffer.GetActiveSize());
    }

    ~AuthPacket()
    {
        if (!_socket || !_socket->IsOpen())
            return;

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

        _socket->QueuePacket(std::forward<MessageBuffer>(buffer));

        _extraData.resize(0);
    }

    T& operator -> ()
    {
        return &_data;
    }

    template <typename T, size_t N = sizeof(T), typename std::enable_if<std::is_arithmetic<T>::value, int>::type = 0>
    AuthPacket& operator += (T value)
    {
        if (!_socket)
            return *this;

        EnsureExtraSpace(N);
        memcpy(_extraData.data() + _wpos, &value, sizeof(T));
        _wpos += sizeof(T);
        return *this;
    }

    AuthPacket& operator += (const char* value)
    {
        if (!_socket || strlen(value) == 0)
            return *this;

        EnsureExtraSpace(strlen(value));
        memcpy(_extraData.data() + _wpos, value, strlen(value));
        _wpos += strlen(value);
        return *this;
    }

    template <size_t N>
    AuthPacket& operator += (char(&arr)[N])
    {
        if (!_socket || N == 0)
            return *this;

        EnsureExtraSpace(N);
        memcpy(_extraData.data() + _wpos, value, N);
        _wpos += N;
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
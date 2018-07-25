#pragma once

#include <vector>
#include <cstring>
#include <cstdint>

#include <boost/asio/buffer.hpp>

namespace wowgm::protocol
{
    class MessageBuffer
    {
        typedef std::vector<std::uint8_t>::size_type size_type;

    public:
        explicit MessageBuffer(std::size_t initialSize) : _wpos(0), _rpos(0), _storage(initialSize)
        {
            memset(_storage.data(), 0, initialSize);
        }

        MessageBuffer() : MessageBuffer(4096)
        {
        }

        MessageBuffer(MessageBuffer const& right) : _wpos(right._wpos), _rpos(right._rpos), _storage(right._storage)
        {

        }

        MessageBuffer(MessageBuffer&& right) : _wpos(right._wpos), _rpos(right._rpos), _storage(std::move(right._storage))
        {
            right._wpos = 0;
            right._rpos = 0;
        }

        void Reset()
        {
            _wpos = 0;
            _rpos = 0;
        }

        void Resize(size_type newSize) {
            _storage.resize(newSize);
        }

        std::uint8_t* GetBasePointer() { return _storage.data(); }
        std::uint8_t* GetReadPointer() { return GetBasePointer() + _rpos; }
        std::uint8_t* GetWritePointer() { return GetBasePointer() + _wpos; }

        void ReadCompleted(size_type size) {
            _rpos += size;
        }
        void WriteCompleted(size_type size) {
            _wpos += size;
        }

        size_type GetActiveSize() const { return _wpos - _rpos; }
        size_type GetRemainingSpace() const { return _storage.size() - _wpos; }
        size_type GetBufferSize() const { return _storage.size(); }

        void Normalize()
        {
            if (_rpos == 0)
                return;

            if (_rpos != _wpos)
                std::memmove(GetBasePointer(), GetReadPointer(), GetActiveSize());

            _wpos -= _rpos;
            _rpos = 0;
        }

        // Make sure to call normalize before this
        void EnsureFreeSpace()
        {
            if (GetRemainingSpace() == 0)
                _storage.resize(_storage.size() * 3 / 2);
        }

        void EnsureFreeSpace(size_type requestedSize)
        {
            if (GetRemainingSpace() >= requestedSize)
                return;

            _storage.resize(_wpos + requestedSize);
        }

        void Write(void const* data, std::size_t size)
        {
            if (size == 0 || data == nullptr)
                return;

            EnsureFreeSpace(size);
            memcpy(GetWritePointer(), data, size);
            WriteCompleted(size);
        }

        MessageBuffer& operator = (MessageBuffer const& right)
        {
            if (this != &right)
            {
                _wpos = right._wpos;
                _rpos = right._rpos;
                _storage = right._storage;
            }

            return *this;
        }

        MessageBuffer& operator = (MessageBuffer&& right)
        {
            if (this != &right)
            {
                _wpos = right._wpos;
                _rpos = right._rpos;
                _storage = std::move(right._storage);

                right._wpos = 0;
                right._rpos = 0;
            }

            return *this;
        }

        boost::asio::mutable_buffer AsWriteBuffer()
        {
            return boost::asio::buffer(GetWritePointer(), GetRemainingSpace());
        }

        boost::asio::mutable_buffer AsReadBuffer()
        {
            return boost::asio::buffer(GetReadPointer(), GetActiveSize());
        }

        MessageBuffer& operator >> (std::string& other)
        {
            std::uint8_t* data = GetReadPointer();
            while (*data != '\0')
                ++data;

            other.assign(GetReadPointer(), data);
            ReadCompleted(other.size() + 1);
            return *this;
        }

        std::vector<std::uint8_t>&& Move()
        {
            _wpos = 0;
            _rpos = 0;
            return std::move(_storage);
        }

    private:
        size_type _wpos;
        size_type _rpos;
        std::vector<std::uint8_t> _storage;
    };

} // wowgm::protocol

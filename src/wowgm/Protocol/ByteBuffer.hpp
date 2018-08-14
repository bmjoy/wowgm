#pragma once

#include <string>
#include <vector>
#include <cstring>

#include "NetworkingExceptions.hpp"

namespace wowgm::game::structures
{
    class ObjectGuid;
}

namespace wowgm::protocol
{
    using namespace exceptions;

    class MessageBuffer;

    class ByteBuffer
    {
    public:
        static size_t const DEFAULT_SIZE = 0x1000;
        static std::uint8_t const InitialBitPos = 8;

        // constructor
        ByteBuffer() : _rpos(0), _wpos(0), _bitpos(InitialBitPos), _curbitval(0)
        {
            _storage.reserve(DEFAULT_SIZE);
        }

        ByteBuffer(size_t reserve) : _rpos(0), _wpos(0), _bitpos(InitialBitPos), _curbitval(0)
        {
            _storage.reserve(reserve);
        }

        ByteBuffer(ByteBuffer&& buf) noexcept : _rpos(buf._rpos), _wpos(buf._wpos),
            _bitpos(buf._bitpos), _curbitval(buf._curbitval), _storage(buf.Move()) { }

        ByteBuffer(ByteBuffer const& right) : _rpos(right._rpos), _wpos(right._wpos),
            _bitpos(right._bitpos), _curbitval(right._curbitval), _storage(right._storage) { }

        ByteBuffer(MessageBuffer&& buffer);

        std::vector<std::uint8_t>&& Move() noexcept
        {
            _rpos = 0;
            _wpos = 0;
            _bitpos = InitialBitPos;
            _curbitval = 0;
            return std::move(_storage);
        }

        ByteBuffer& operator=(ByteBuffer const& right)
        {
            if (this !=& right)
            {
                _rpos = right._rpos;
                _wpos = right._wpos;
                _bitpos = right._bitpos;
                _curbitval = right._curbitval;
                _storage = right._storage;
            }

            return *this;
        }

        ByteBuffer& operator=(ByteBuffer&& right)
        {
            if (this !=& right)
            {
                _rpos = right._rpos;
                _wpos = right._wpos;
                _bitpos = right._bitpos;
                _curbitval = right._curbitval;
                _storage = right.Move();
            }

            return *this;
        }

        virtual ~ByteBuffer() { }

        void clear()
        {
            _rpos = 0;
            _wpos = 0;
            _bitpos = InitialBitPos;
            _curbitval = 0;
            _storage.clear();
        }

        template <typename T> void append(T value)
        {
            static_assert(std::is_fundamental<T>::value, "append(compound)");
            // EndianConvert(value);
            append((std::uint8_t *)&value, sizeof(value));
        }

        void FlushBits()
        {
            if (_bitpos == 8)
                return;

            _bitpos = 8;

            append((std::uint8_t *)&_curbitval, sizeof(std::uint8_t));
            _curbitval = 0;
        }

        void ResetBitPos()
        {
            if (_bitpos > 7)
                return;

            _bitpos = 8;
            _curbitval = 0;
        }

        bool WriteBit(bool bit)
        {
            --_bitpos;
            if (bit)
                _curbitval |= (1 << (_bitpos));

            if (_bitpos == 0)
            {
                _bitpos = 8;
                append((std::uint8_t *)&_curbitval, sizeof(_curbitval));
                _curbitval = 0;
            }

            return bit;
        }

        bool ReadBit()
        {
            ++_bitpos;
            if (_bitpos > 7)
            {
                _curbitval = read<std::uint8_t>();
                _bitpos = 0;
            }

            return ((_curbitval >> (7 - _bitpos))&  1) != 0;
        }

        void WriteBits(std::size_t value, std::int32_t bits)
        {
            for (std::int32_t i = bits - 1; i >= 0; --i)
                WriteBit((value >> i)&  1);
        }

        std::uint32_t ReadBits(std::int32_t bits)
        {
            std::uint32_t value = 0;
            for (std::int32_t i = bits - 1; i >= 0; --i)
                if (ReadBit())
                    value |= (1 << (i));

            return value;
        }

        // Reads a byte (if needed) in-place
        void ReadByteSeq(std::uint8_t& b)
        {
            if (b != 0)
                b ^= read<std::uint8_t>();
        }

        void WriteByteSeq(std::uint8_t b)
        {
            if (b != 0)
                append<std::uint8_t>(b ^ 1);
        }

        template <typename T>
        void put(std::size_t pos, T value)
        {
            static_assert(std::is_fundamental<T>::value, "append(compound)");
            // EndianConvert(value);
            put(pos, (std::uint8_t *)&value, sizeof(value));
        }

        /**
        * @name   PutBits
        * @brief  Places specified amount of bits of value at specified position in packet.
        *         To ensure all bits are correctly written, only call this method after
        *         bit flush has been performed
        * @param  pos Position to place the value at, in bits. The entire value must fit in the packet
        *             It is advised to obtain the position using bitwpos() function.
        * @param  value Data to write.
        * @param  bitCount Number of bits to store the value on.
        */
        void PutBits(std::size_t pos, std::size_t value, std::uint32_t bitCount);

        ByteBuffer& operator<<(std::uint8_t value)
        {
            append<std::uint8_t>(value);
            return *this;
        }

        ByteBuffer& operator<<(std::uint16_t value)
        {
            append<std::uint16_t>(value);
            return *this;
        }

        ByteBuffer& operator<<(std::uint32_t value)
        {
            append<std::uint32_t>(value);
            return *this;
        }

        ByteBuffer& operator<<(std::uint64_t value)
        {
            append<std::uint64_t>(value);
            return *this;
        }

        ByteBuffer& operator<<(std::int8_t value)
        {
            append<std::int8_t>(value);
            return *this;
        }

        ByteBuffer& operator<<(std::int16_t value)
        {
            append<std::int16_t>(value);
            return *this;
        }

        ByteBuffer& operator<<(std::int32_t value)
        {
            append<std::int32_t>(value);
            return *this;
        }

        ByteBuffer& operator<<(std::int64_t value)
        {
            append<std::int64_t>(value);
            return *this;
        }

        // floating points
        ByteBuffer& operator<<(float value)
        {
            append<float>(value);
            return *this;
        }

        ByteBuffer& operator<<(double value)
        {
            append<double>(value);
            return *this;
        }

        ByteBuffer& operator<<(const std::string& value)
        {
            if (size_t len = value.length())
                append((std::uint8_t const*)value.c_str(), len);
            append<std::uint8_t>(0);
            return *this;
        }

        ByteBuffer& operator<<(const char *str)
        {
            if (size_t len = (str ? strlen(str) : 0))
                append((std::uint8_t const*)str, len);
            append<std::uint8_t>(0);
            return *this;
        }

        ByteBuffer& operator>>(bool& value)
        {
            value = read<char>() > 0 ? true : false;
            return *this;
        }

        ByteBuffer& operator>>(std::uint8_t& value)
        {
            value = read<std::uint8_t>();
            return *this;
        }

        ByteBuffer& operator>>(std::uint16_t& value)
        {
            value = read<std::uint16_t>();
            return *this;
        }

        ByteBuffer& operator>>(std::uint32_t& value)
        {
            value = read<std::uint32_t>();
            return *this;
        }

        ByteBuffer& operator>>(std::uint64_t& value)
        {
            value = read<std::uint64_t>();
            return *this;
        }

        //signed as in 2e complement
        ByteBuffer& operator>>(std::int8_t& value)
        {
            value = read<std::int8_t>();
            return *this;
        }

        ByteBuffer& operator>>(std::int16_t& value)
        {
            value = read<std::int16_t>();
            return *this;
        }

        ByteBuffer& operator>>(std::int32_t& value)
        {
            value = read<std::int32_t>();
            return *this;
        }

        ByteBuffer& operator>>(std::int64_t& value)
        {
            value = read<std::int64_t>();
            return *this;
        }

        ByteBuffer& operator>>(float& value);
        ByteBuffer& operator>>(double& value);

        ByteBuffer& operator>>(std::string& value)
        {
            value.clear();
            while (rpos() < size())                         // prevent crash at wrong string format in packet
            {
                char c = read<char>();
                if (c == 0)
                    break;
                value += c;
            }
            return *this;
        }

        std::uint8_t& operator[](size_t const pos)
        {
            if (pos >= size())
                throw ByteBufferPositionException(pos, 1, size());
            return _storage[pos];
        }

        std::uint8_t const& operator[](size_t const pos) const
        {
            if (pos >= size())
                throw ByteBufferPositionException(pos, 1, size());
            return _storage[pos];
        }

        size_t rpos() const { return _rpos; }

        size_t rpos(size_t rpos_)
        {
            _rpos = rpos_;
            return _rpos;
        }

        void rfinish()
        {
            _rpos = wpos();
        }

        size_t wpos() const { return _wpos; }

        size_t wpos(size_t wpos_)
        {
            _wpos = wpos_;
            return _wpos;
        }

        /// Returns position of last written bit
        size_t bitwpos() const { return _wpos * 8 + 8 - _bitpos; }

        size_t bitwpos(size_t newPos)
        {
            _wpos = newPos / 8;
            _bitpos = 8 - (newPos % 8);
            return _wpos * 8 + 8 - _bitpos;
        }

        template<typename T>
        void read_skip() { read_skip(sizeof(T)); }

        void read_skip(size_t skip)
        {
            if (_rpos + skip > size())
                throw ByteBufferPositionException(_rpos, skip, size());

            ResetBitPos();
            _rpos += skip;
        }

        template <typename T> T read()
        {
            ResetBitPos();
            T r = read<T>(_rpos);
            _rpos += sizeof(T);
            return r;
        }

        template <typename T> T read(size_t pos) const
        {
            if (pos + sizeof(T) > size())
                throw ByteBufferPositionException(pos, sizeof(T), size());
            T val = *((T const*)&_storage[pos]);
            // EndianConvert(val);
            return val;
        }

        void read(std::uint8_t *dest, size_t len)
        {
            if (_rpos + len > size())
                throw ByteBufferPositionException(_rpos, len, size());

            ResetBitPos();
            std::memcpy(dest,& _storage[_rpos], len);
            _rpos += len;
        }

        void ReadPackedUInt64(std::uint64_t& guid)
        {
            guid = 0;
            ReadPackedUInt64(read<std::uint8_t>(), guid);
        }

        void ReadPackedGuid(wowgm::game::structures::ObjectGuid& guid);

        void ReadPackedUInt64(std::uint8_t mask, std::uint64_t& value)
        {
            for (std::uint32_t i = 0; i < 8; ++i)
                if (mask&  (std::uint8_t(1) << i))
                    value |= (std::uint64_t(read<std::uint8_t>()) << (i * 8));
        }

        std::string ReadString(std::uint32_t length)
        {
            if (_rpos + length > size())
                throw ByteBufferPositionException(_rpos, length, size());

            ResetBitPos();
            if (!length)
                return std::string();

            std::string str((char const*)&_storage[_rpos], length);
            _rpos += length;
            return str;
        }

        //! Method for writing strings that have their length sent separately in packet
        //! without null-terminating the string
        void WriteString(std::string const& str)
        {
            if (size_t len = str.length())
                append(str.c_str(), len);
        }

        void WriteString(char const* str, size_t len)
        {
            if (len)
                append(str, len);
        }

        std::uint32_t ReadPackedTime();

        std::uint8_t* contents()
        {
            if (_storage.empty())
                throw ByteBufferException();
            return _storage.data();
        }

        std::uint8_t const* contents() const
        {
            if (_storage.empty())
                throw ByteBufferException();
            return _storage.data();
        }

        size_t size() const { return _storage.size(); }
        bool empty() const { return _storage.empty(); }

        void resize(size_t newsize)
        {
            _storage.resize(newsize, 0);
            _rpos = 0;
            _wpos = size();
        }

        bool HasData() const
        {
            if (empty())
                return _storage.capacity() == 0;

            return _wpos == _storage.size();
        }

        void reserve(size_t ressize)
        {
            if (ressize > size())
                _storage.reserve(ressize);
        }

        void append(const char *src, size_t cnt)
        {
            return append((const std::uint8_t *)src, cnt);
        }

        template<class T> void append(const T *src, size_t cnt)
        {
            return append((const std::uint8_t *)src, cnt * sizeof(T));
        }

        void append(const std::uint8_t *src, size_t cnt);

        void append(const ByteBuffer& buffer)
        {
            if (!buffer.empty())
                append(buffer.contents(), buffer.size());
        }

        // can be used in SMSG_MONSTER_MOVE opcode
        void appendPackXYZ(float x, float y, float z)
        {
            std::uint32_t packed = 0;
            packed |= ((int)(x / 0.25f)&  0x7FF);
            packed |= ((int)(y / 0.25f)&  0x7FF) << 11;
            packed |= ((int)(z / 0.25f)&  0x3FF) << 22;
            *this << packed;
        }

        void appendPackGUID(std::uint64_t guid)
        {
            std::uint8_t packGUID[8 + 1];
            packGUID[0] = 0;
            size_t size = 1;
            for (std::uint8_t i = 0; guid != 0; ++i)
            {
                if (guid & 0xFF)
                {
                    packGUID[0] |= std::uint8_t(1 << i);
                    packGUID[size] = std::uint8_t(guid & 0xFF);
                    ++size;
                }

                guid >>= 8;
            }
            append(packGUID, size);
        }

        void AppendPackedUInt64(std::uint64_t guid)
        {
            std::uint8_t mask = 0;
            size_t pos = wpos();
            *this << std::uint8_t(mask);

            std::uint8_t packed[8];
            if (size_t packedSize = PackUInt64(guid,& mask, packed))
                append(packed, packedSize);

            put<std::uint8_t>(pos, mask);
        }

        static size_t PackUInt64(std::uint64_t value, std::uint8_t* mask, std::uint8_t* result)
        {
            size_t resultSize = 0;
            *mask = 0;
            memset(result, 0, 8);

            for (std::uint8_t i = 0; value != 0; ++i)
            {
                if (value&  0xFF)
                {
                    *mask |= std::uint8_t(1 << i);
                    result[resultSize++] = std::uint8_t(value&  0xFF);
                }

                value >>= 8;
            }

            return resultSize;
        }

        void AppendPackedTime(time_t time);

        void put(size_t pos, const std::uint8_t *src, size_t cnt);

    protected:
        size_t _rpos, _wpos, _bitpos;
        std::uint8_t _curbitval;
        std::vector<std::uint8_t> _storage;
    };

    /// @todo Make a ByteBuffer.cpp and move all this inlining to it.
    template<> inline std::string ByteBuffer::read<std::string>()
    {
        std::string tmp;
        *this >> tmp;
        return tmp;
    }

    template<>
    inline void ByteBuffer::read_skip<char*>()
    {
        std::string temp;
        *this >> temp;
    }

    template<>
    inline void ByteBuffer::read_skip<char const*>()
    {
        read_skip<char*>();
    }

    template<>
    inline void ByteBuffer::read_skip<std::string>()
    {
        read_skip<char*>();
    }
}
/*
* Copyright (C) 2008-2018 TrinityCore <https://www.trinitycore.org/>
* Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; either version 2 of the License, or (at your
* option) any later version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
* more details.
*
* You should have received a copy of the GNU General Public License along
* with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "ByteBuffer.hpp"
#include "MessageBuffer.hpp"
#include "Utils.hpp"
#include "Assert.hpp"
#include "ObjectGuid.hpp"
#include <sstream>
#include <ctime>

using namespace wowgm::game::structures;

namespace wowgm::protocol
{

    ByteBuffer::ByteBuffer(MessageBuffer&& buffer) : _rpos(0), _wpos(0), _bitpos(InitialBitPos), _curbitval(0), _storage(buffer.Move())
    {
    }

    ByteBuffer& ByteBuffer::operator>>(float& value)
    {
        value = read<float>();
        if (!std::isfinite(value))
            throw ByteBufferException();
        return *this;
    }

    ByteBuffer& ByteBuffer::operator>>(double& value)
    {
        value = read<double>();
        if (!std::isfinite(value))
            throw ByteBufferException();
        return *this;
    }

    uint32_t ByteBuffer::ReadPackedTime()
    {
        uint32_t packedDate = read<uint32_t>();
        tm lt = tm();

        lt.tm_min = packedDate & 0x3F;
        lt.tm_hour = (packedDate >> 6) & 0x1F;
        //lt.tm_wday = (packedDate >> 11) & 7;
        lt.tm_mday = ((packedDate >> 14) & 0x3F) + 1;
        lt.tm_mon = (packedDate >> 20) & 0xF;
        lt.tm_year = ((packedDate >> 24) & 0x1F) + 100;

        return uint32_t(mktime(&lt));
    }

    void ByteBuffer::append(const uint8_t *src, size_t cnt)
    {
        BOOST_ASSERT_MSG_FMT(src, "Attempted to put a NULL-pointer in ByteBuffer (pos: %Iu size: %Iu)", _wpos, size());
        BOOST_ASSERT_MSG_FMT(cnt, "Attempted to put a zero-sized value in ByteBuffer (pos: %Iu size: %Iu)", _wpos, size());
        BOOST_ASSERT(size() < 10000000);

        FlushBits();
        _storage.insert(_storage.begin() + _wpos, src, src + cnt);
        _wpos += cnt;
    }

    void ByteBuffer::AppendPackedTime(time_t time)
    {
        tm lt;
        localtime_r(&time, &lt);
        append<uint32_t>((lt.tm_year - 100) << 24 | lt.tm_mon << 20 | (lt.tm_mday - 1) << 14 | lt.tm_wday << 11 | lt.tm_hour << 6 | lt.tm_min);
    }

    void ByteBuffer::put(size_t pos, const uint8_t *src, size_t cnt)
    {
        BOOST_ASSERT_MSG_FMT(pos + cnt <= size(), "Attempted to put value with size: %Iu in ByteBuffer (pos: %Iu size: %Iu)", cnt, pos, size());
        BOOST_ASSERT_MSG_FMT(src, "Attempted to put a NULL-pointer in ByteBuffer (pos: %Iu size: %Iu)", pos, size());
        BOOST_ASSERT_MSG_FMT(cnt, "Attempted to put a zero-sized value in ByteBuffer (pos: %Iu size: %Iu)", pos, size());

        std::memcpy(&_storage[pos], src, cnt);
    }

    void ByteBuffer::PutBits(std::size_t pos, std::size_t value, uint32_t bitCount)
    {
        BOOST_ASSERT_MSG_FMT(pos + bitCount <= size() * 8, "Attempted to put %u bits in ByteBuffer (bitpos: %Iu size: %Iu)", bitCount, pos, size());
        BOOST_ASSERT_MSG(bitCount, "Attempted to put a zero bits in ByteBuffer");

        for (uint32_t i = 0; i < bitCount; ++i)
        {
            std::size_t wp = (pos + i) / 8;
            std::size_t bit = (pos + i) % 8;
            if ((value >> (bitCount - i - 1)) & 1)
                _storage[wp] |= 1 << (7 - bit);
            else
                _storage[wp] &= ~(1 << (7 - bit));
        }
    }

    void ByteBuffer::ReadPackedGuid(ObjectGuid& guid)
    {
        uint64_t uValue = 0;
        ReadPackedUInt64(uValue);
        guid.Set(uValue);
    }
}

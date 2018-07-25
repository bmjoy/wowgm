#include "NetworkingExceptions.hpp"

#include <sstream>

namespace wowgm::networking::exceptions
{
    ByteBufferPositionException::ByteBufferPositionException(size_t pos, size_t size, size_t valueSize)
    {
        std::ostringstream ss;

        ss << "Attempted to get value with size: "
            << valueSize << " in ByteBuffer (pos: " << pos << " size: " << size
            << ")";

        message().assign(ss.str());
    }
}
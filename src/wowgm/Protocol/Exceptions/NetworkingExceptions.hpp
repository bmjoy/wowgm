#pragma once

#include <stdexcept>

namespace wowgm::protocol::exceptions
{
    class ByteBufferException : public std::exception
    {
    public:
        ~ByteBufferException() throw() { }

        char const* what() const throw() override { return msg_.c_str(); }

    protected:
        std::string& message() throw() { return msg_; }

    private:
        std::string msg_;
    };

    class ByteBufferPositionException : public ByteBufferException
    {
    public:
        ByteBufferPositionException(size_t pos, size_t size, size_t valueSize);

        ~ByteBufferPositionException() throw() { }
    };
}
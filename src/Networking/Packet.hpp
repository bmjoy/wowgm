#pragma once

#include "MessageBuffer.hpp"

class Packet : public MessageBuffer
{
public:
    template <typename T>
    void Write(T&& value)
    {
        EnsureFreeSpace(sizeof(T));
        memcpy(GetWritePointer(), &value, sizeof(T));
        WriteCompleted(sizeof(T));
    }

    template <typename T, size_t N>
    void Write(T(&arr)[N])
    {
        EnsureFreeSpace(sizeof(T) * N);
        memcpy(GetWritePointer(), arr, N);
        WriteCompleted(sizeof(T) * N);
    }
};

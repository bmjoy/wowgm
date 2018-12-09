#pragma once

#include <atomic>
#include <thread>

// A dumbo-dumb spinlock
class Spinlock
{
public:
    void Lock() {
        while (_flag.test_and_set(std::memory_order_acquire));
    }

    void Unlock() {
        _flag.clear(std::memory_order_release);
    }

private:
    std::atomic_flag _flag = ATOMIC_FLAG_INIT;
};

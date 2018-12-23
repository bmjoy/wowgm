#pragma once

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

#include <extstd/threading/join_all.hpp>

namespace shared::threading
{
    class thread_pool
    {
    public:
        thread_pool();
        explicit thread_pool(size_t threadCount);

        ~thread_pool();

        template <typename F, typename... Args>
        auto submit(F&& function, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>;

    private:
        std::vector<std::thread> _workers;
        std::queue<std::function<void()>> _queue;

        std::mutex _queueMutex;
        std::condition_variable _condition;
        std::atomic_bool _stop;
    };

    thread_pool::~thread_pool()
    {
        while (!_queue.empty())
            _queue.pop();

        _stop.exchange(true);

        _condition.notify_all();
        extstd::threading::join_all(_workers);
    }

    thread_pool::thread_pool() : thread_pool(std::max(std::thread::hardware_concurrency() - 1, 1u))
    {

    }

    inline thread_pool::thread_pool(size_t threads) : _stop(false)
    {
        for (size_t i = 0; i < threads; ++i)
            _workers.emplace_back([this]() -> void {
                for (;;)
                {
                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lock(this->_queueMutex);

                        this->_condition.wait(lock, [this]() { return this->_stop || !this->_queue.empty(); });
                        if (this->_stop && this->_queue.empty())
                            return;

                        task = std::move(this->_queue.front());
                        this->_queue.pop();
                    }

                    task();
                }
        });
    }

    template <typename F, typename... Args>
    auto thread_pool::submit(F&& function, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>
    {
        using return_type = typename std::result_of<F(Args...)>::type;

        auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        auto future = task->get_future();
        {
            std::unique_lock<std::mutex> lock(_queueMutex);
            if (_stop)
                return;

            _queue.emplace([task]() -> void {
                (*task)();
            });
        }

        _condition.notify_one();
        return future;
    }
}

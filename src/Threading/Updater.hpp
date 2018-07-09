#pragma once

#include <memory>
#include <thread>
#include <condition_variable>
#include <future>
#include <cstdint>

class Updatable;

class Updater
{
    public:
        Updater() { }
        ~Updater()
        {
            Stop();
        }

    public:
        void Start();
        void Stop();

        template <typename T, typename... Args, typename std::enable_if<std::is_base_of<Updatable, T>::value, int>::type = 0>
        const std::shared_ptr<T>& CreateUpdatable(Args&&... args)
        {
            std::shared_ptr<T> ptr = std::make_shared<T>(std::forward<Args>(args)...);
            _updatables.push_back(ptr);
            return ptr;
        }

        template <typename T, typename std::enable_if<std::is_base_of<Updatable, T>::value, int>::type = 0>
        std::shared_ptr<T> CreateUpdatable()
        {
            std::shared_ptr<T> ptr = std::make_shared<T>();
            _updatables.push_back(ptr);
            return ptr;
        }

    private:
        void ThreadWorker(std::future<void> future);

        std::vector<std::shared_ptr<Updatable>> _updatables;

        std::promise<void> _promise;
        std::thread _worker;
};


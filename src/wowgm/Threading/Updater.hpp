#pragma once

#include <memory>
#include <thread>
#include <condition_variable>
#include <future>
#include <cstdint>

namespace wowgm::threading {

    class Updatable;

    class Updater
    {
        Updater();
    public:
        ~Updater()
        {
            Stop();
        }

        static Updater* instance();

    public:
        void Start();
        void Stop();

        template <typename T, typename... Args, typename std::enable_if<std::is_base_of<Updatable, T>::value, int>::type = 0>
        std::shared_ptr<T> CreateUpdatable(Args&&... args)
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
        void ThreadWorker(std::future<void> startFuture, std::future<void> future);

        std::vector<std::shared_ptr<Updatable>> _updatables;

        std::promise<void> _startPromise;
        std::promise<void> _promise;
        std::thread _worker;
    };

} // wowgm::threading

#define sUpdater wowgm::threading::Updater::instance()

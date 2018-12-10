#include "Updater.hpp"
#include "Updatable.hpp"

#include <iostream>
#include <chrono>

namespace wowgm::threading
{
    Updater* Updater::instance()
    {
        static Updater instance;
        return &instance;
    }

    Updater::Updater() : _worker(std::thread(&Updater::ThreadWorker, this, std::move(_startPromise.get_future()), std::move(_promise.get_future())))
    {

    }

    void Updater::Start()
    {
        _startPromise.set_value();
    }

    void Updater::Stop()
    {
        // Signal
        _promise.set_value();

        if (_worker.joinable())
            _worker.join(); // join

        for (auto&& upd : _updatables)
            upd->Destroy();

        _updatables.clear();
    }

    void Updater::ThreadWorker(std::future<void> startFuture, std::future<void> future)
    {
        while (startFuture.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout);

        namespace chrono = std::chrono;
        using hrc = chrono::high_resolution_clock;

        auto lastUpdateTick = hrc::now();

        while (future.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout)
        {
            auto milliseconds = chrono::duration_cast<chrono::microseconds>(hrc::now() - lastUpdateTick);
            for (auto&& upd : _updatables)
                upd->Update(static_cast<uint32_t>(milliseconds.count()) / 1000);

            lastUpdateTick = hrc::now();
        }
    }

} // wowgm::threading

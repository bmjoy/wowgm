#include "Updater.hpp"
#include "Updatable.hpp"

#include <iostream>
#include <chrono>

void Updater::Start()
{
    auto future = _promise.get_future();

    new (&_worker) std::thread(&Updater::ThreadWorker, this, std::move(future));
}

void Updater::Stop()
{
    _promise.set_value(); // signal
    if (_worker.joinable())
        _worker.join(); // join

    _updatables.resize(0);
}

void Updater::ThreadWorker(std::future<void> future)
{
    namespace chrono = std::chrono;
    using hrc = chrono::high_resolution_clock;

    auto lastUpdateTick = hrc::now();

    while (future.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout)
    {
        auto milliseconds = chrono::duration_cast<chrono::milliseconds>(hrc::now() - lastUpdateTick);
        for (auto&& upd : _updatables)
            upd->Update(static_cast<std::uint32_t>(milliseconds.count()));

        lastUpdateTick = hrc::now();
    }

    std::cout << "Updater: exiting." << std::endl;
}

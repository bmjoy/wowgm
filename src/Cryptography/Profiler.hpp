#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <boost/current_function.hpp>
#include <iostream>

struct profiler_manager
{
    private:

        struct data_node
        {
            std::uint32_t count;
            std::int64_t max;
            std::int64_t min;
            float avg;
        };

    public:
        static profiler_manager* instance()
        {
            static profiler_manager inst;
            return &inst;
        }

        void report(std::string&& str, std::chrono::milliseconds ns)
        {
            profiler_map::iterator data = _timers.find(str);

            auto nsc = ns.count();
            if (data == _timers.end())
            {
                _timers[str] = { 1, nsc, nsc, float(nsc) };
            }
            else
            {
                auto& ref = _timers[str];
                ref.min = std::min(ref.min, nsc);
                ref.max = std::max(ref.max, nsc);
                ref.avg = (ref.avg * ref.count + nsc) / ++ref.count;
            }

            std::cout << str << ": max " << _timers[str].max << " ms; min " << _timers[str].min << " ms; avg " << _timers[str].avg << " ms." << std::endl;
        }

    private:

        std::unordered_map<std::string, data_node> _timers;
        typedef decltype(_timers) profiler_map;
};

struct profiler
{
        profiler(std::string&& str) : _start(std::chrono::high_resolution_clock::now()), _name(std::move(str))
        {
        }

        ~profiler()
        {
            auto now = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - _start);

            profiler_manager::instance()->report(std::move(_name), elapsed);
        }

    private:
        std::string _name;
        std::chrono::time_point<std::chrono::high_resolution_clock> _start;
};

#define PROFILE profiler ___profiler(BOOST_CURRENT_FUNCTION);

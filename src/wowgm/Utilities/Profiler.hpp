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
        uint32_t count;
        int64_t max;
        int64_t min;
        float avg;
    };

public:
    static profiler_manager* instance();

    void report(std::string&& str, std::chrono::microseconds ns);

private:

    std::unordered_map<std::string, data_node> _timers;
    typedef decltype(_timers) profiler_map;
};

struct profiler
{
    profiler(std::string&& str);

    ~profiler();

private:
    std::string _name;
    std::chrono::time_point<std::chrono::high_resolution_clock> _start;
};

#if _DEBUG
# define PROFILE profiler ___profiler(BOOST_CURRENT_FUNCTION);
#else
# define PROFILE
#endif

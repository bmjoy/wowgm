#include "Profiler.hpp"

#include <shared/log/log.hpp>

profiler_manager* profiler_manager::instance()
{
    static profiler_manager inst;
    return &inst;
}

void profiler_manager::report(std::string&& str, std::chrono::microseconds ns)
{
    profiler_map::iterator data = _timers.find(str);

    auto nsc = ns.count();

    data_node* ref;

    if (data == _timers.end())
    {
        ref = &(_timers[str] = { 1, nsc, nsc, float(nsc) });
    }
    else
    {
        ref = &_timers[str];
        ref->min = std::min(ref->min, nsc);
        ref->max = std::max(ref->max, nsc);
        ref->avg = (ref->avg * ref->count + nsc) / ++ref->count;
    }

    if (ref->count == 1)
        LOG_INFO("{}: Execution time: {} ms", str, float(ref->min) / 1000.0f);
    else
        LOG_INFO("{}: Execution time: {} to {} ms", str, float(ref->min) / 1000.0f, float(ref->max) / 1000.0f);
}

profiler::profiler(std::string&& str) : _start(std::chrono::high_resolution_clock::now()), _name(std::move(str))
{
}

profiler::~profiler()
{
    auto now = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - _start);

    profiler_manager::instance()->report(std::move(_name), elapsed);
}
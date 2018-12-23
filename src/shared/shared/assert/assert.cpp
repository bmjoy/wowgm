#include <shared/assert/assert.hpp>
#include <shared/stacktrace/stacktrace.hpp>

#include <boost/config/detail/suffix.hpp>

#include <iostream>
#include <cstdarg>

namespace boost
{
    void assertion_failed_msg_fmt(char const* expr, char const* msg, char const* function, char const* file, long line, ...)
    {
        va_list args;
        va_start(args, line);

        std::cerr << file << ':' << line << " in " << function << " ASSERTION FAILED \"" << expr << "\" \n";
        vfprintf(stderr, msg, args);
        std::cerr << '\n';
        std::cerr << "Backtrace:\n" << shared::stacktrace::application_stacktrace() << std::endl;

        va_end(args);

        *((volatile int*)NULL) = 0;
        exit(1);
    }

    inline void assertion_failed_msg(char const* expr, char const* msg, char const* function, char const* file, long line)
    {
        std::cerr << file << ':' << line << " in " << function << " ASSERTION FAILED \"" << expr << "\" \n";
        if (msg != nullptr)
            std::cerr << msg << '\n';

        std::cerr << "Backtrace:\n" << shared::stacktrace::application_stacktrace() << '\n';

        *((volatile int*)NULL) = 0;
        std::exit(1);
    }

}

namespace shared::exceptions {

    template <class E, typename... Args>
    inline void throw_with_trace(Args&&... args)
    {
        E ex(std::forward<Args>(args)...);
        throw boost::enable_error_info(ex) << traced(boost::stacktrace::application_stacktrace());
    }
}
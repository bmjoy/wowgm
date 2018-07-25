#ifndef WOWGM_ASSERT_H_
#define WOWGM_ASSERT_H_

#include <iostream>

#include <boost/stacktrace.hpp>
#include <boost/exception/all.hpp>

#define BOOST_ENABLE_ASSERT_DEBUG_HANDLER
#include <boost/assert.hpp>
struct tag_stacktrace { };

typedef boost::error_info<tag_stacktrace, boost::stacktrace::stacktrace> traced;

namespace wowgm::exceptions
{
    template <class E, typename... Args>
    inline void throw_with_trace(Args&&... args)
    {
        E ex(std::forward<Args>(args)...);
        throw boost::enable_error_info(ex) << traced(boost::stacktrace::stacktrace());
    }
}

namespace boost {
    inline void assertion_failed_msg_fmt(char const* expr, char const* msg, char const* function, char const* file, long line, ...)
    {
        va_list args;
        va_start(args, line);

        std::cerr << file << ':' << line << " in " << function << " ASSERTION FAILED \"" << expr << "\" \n";
        vfprintf(stderr, msg, args);
        std::cerr << '\n';
        std::cerr << "Backtrace:\n" << boost::stacktrace::stacktrace() << std::endl;

        va_end(args);

        *((volatile int*)NULL) = 0;
        exit(1);
    }

    inline void assertion_failed_msg(char const* expr, char const* msg, char const* function, char const* file, long line) {

        std::cerr << file << ':' << line << " in " << function << " ASSERTION FAILED \"" << expr << "\" \n";
        if (msg != nullptr)
            std::cerr << msg << '\n';

        std::cerr << "Backtrace:\n" << boost::stacktrace::stacktrace() << '\n';

        *((volatile int*)NULL) = 0;
        exit(1);
    }

    inline void assertion_failed(char const* expr, char const* function, char const* file, long line) {
        ::boost::assertion_failed_msg(expr, 0 /*nullptr*/, function, file, line);
    }
} // namespace boost

#endif // WOWGM_ASSERT_H_


#define BOOST_ASSERT_MSG_FMT(expr, msg, ...) (BOOST_LIKELY(!!(expr))? ((void)0): ::boost::assertion_failed_msg(#expr, msg, BOOST_CURRENT_FUNCTION, __FILE__, __LINE__))


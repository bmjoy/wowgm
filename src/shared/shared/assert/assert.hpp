#ifndef ASSERT_GUARD_H_
#define ASSERT_GUARD_H_

#define BOOST_ENABLE_ASSERT_DEBUG_HANDLER

#include <boost/exception/all.hpp>
#include <boost/assert.hpp>
#include <iostream>

#include "stacktrace.hpp"

namespace wowgm::exceptions
{
    template <class E = std::runtime_error, typename... Args>
    inline void throw_with_trace(Args&&... args)
    {
        E ex(std::forward<Args>(args)...);
        throw boost::enable_error_info(ex) << traced(boost::stacktrace::application_stacktrace());
    }
}

namespace boost
{
    void assertion_failed_msg_fmt(char const* expr, char const* msg, char const* function, char const* file, long line, ...);

    inline void assertion_failed_msg(char const* expr, char const* msg, char const* function, char const* file, long line);

    inline void assertion_failed(char const* expr, char const* function, char const* file, long line) {
         assertion_failed_msg(expr, 0 /*nullptr*/, function, file, line);
    }
} // namespace boost

#define BOOST_ASSERT_MSG_FMT(expr, msg, ...) (BOOST_LIKELY(!!(expr))? ((void)0): ::boost::assertion_failed_msg_fmt(#expr, msg, BOOST_CURRENT_FUNCTION, __FILE__, __LINE__, ##__VA_ARGS__))

#endif // ASSERT_GUARD_H_

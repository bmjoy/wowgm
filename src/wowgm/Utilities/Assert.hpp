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
    template <class E>
    void throw_with_trace(const E& e)
    {
        throw boost::enable_error_info(e) << traced(boost::stacktrace::stacktrace());
    }
}

namespace boost {
    inline void assertion_failed_msg(char const* expr, char const* msg, char const* function, char const* /*file*/, long /*line*/) {
        std::cerr << "Expression '" << expr << "' is false in function '" << function << "': " << (msg ? msg : "<...>") << ".\n"
            << "Backtrace:\n" << boost::stacktrace::stacktrace() << '\n';
        std::abort();
    }

    inline void assertion_failed(char const* expr, char const* function, char const* file, long line) {
        ::boost::assertion_failed_msg(expr, 0 /*nullptr*/, function, file, line);
    }
} // namespace boost

#endif // WOWGM_ASSERT_H_

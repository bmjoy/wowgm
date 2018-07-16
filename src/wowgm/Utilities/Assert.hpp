#pragma once

#include <iostream>
#include <signal.h>

#include <boost/stacktrace.hpp>
#include <boost/filesystem.hpp>
#include <boost/date_time.hpp>
#include <boost/exception/all.hpp>

#define BOOST_ENABLE_ASSERT_DEBUG_HANDLER
#include <boost/assert.hpp>

#include <boost/stacktrace.hpp>
#include <boost/exception/all.hpp>

struct tag_stacktrace { };

typedef boost::error_info<tag_stacktrace, boost::stacktrace::stacktrace> traced;

template <class E>
void throw_with_trace(const E& e)
{
    throw boost::enable_error_info(e) << traced(boost::stacktrace::stacktrace());
}

void my_signal_handler(int signum)
{
    ::signal(signum, SIG_DFL);
    boost::stacktrace::safe_dump_to("./backtrace.dump");
    ::raise(SIGABRT);
}

void register_signal_handler()
{
    ::signal(SIGSEGV, &my_signal_handler);
    ::signal(SIGABRT, &my_signal_handler);
}

void check_existing_stracktraces()
{
    if (boost::filesystem::exists("./backtrace.dump"))
    {
        std::ifstream ifs("./backtrace.dump");

        boost::stacktrace::stacktrace st = boost::stacktrace::stacktrace::from_dump(ifs);
        std::cout << "Previous run crashed:\n" << st << std::endl;

        boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
        auto date_now = now.date();
        auto time_now = now.time_of_day();

        std::stringstream ss;
        ss << "backtrace-" << static_cast<int>(date_now.month()) << "-" << date_now.day() << "-" << date_now.year()
            << "_" << time_now.hours() << "-" << time_now.minutes() << "-" << time_now.seconds() << ".dump";

        auto path = boost::filesystem::current_path();
        path /= ss.str();
        std::cout << "Backtrace has been moved to " << path << ".";

        ifs.close();
        boost::filesystem::copy_file("./backtrace.dump", path);

        // cleaning up
        /*ifs.close();
        boost::filesystem::remove("./backtrace.dump");*/
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

typedef boost::error_info<struct tag_stacktrace, boost::stacktrace::stacktrace> traced;

template <class E>
void throw_with_trace(const E& e) {
    throw boost::enable_error_info(e)
        << traced(boost::stacktrace::stacktrace());
}

